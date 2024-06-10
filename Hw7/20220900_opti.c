#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <wiringPi.h>
#include <arm_neon.h>
#include <omp.h>


#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"

#define BAUDRATE B1000000
#define CLOCKS_PER_US ((double)CLOCKS_PER_SEC / 1000000)

#define CLASS 10

// Input dim
#define I1_C 1
#define I1_H 28
#define I1_W 28

// Conv1 out dim
#define I2_C 16
#define I2_H 14
#define I2_W 14

// Convw out dim
#define I3_C 1
#define I3_H 14
#define I3_W 14

#define CONV1_KERNAL 3
#define CONV1_STRIDE 2
#define CONV2_KERNAL 3
#define CONV2_STRIDE 1
#define FC_IN (I2_H * I2_W)
#define FC_OUT CLASS

int SEGMENT_PINS[8] = {0, 7, 3, 22, 23, 24, 25, 2};
int sev_seg[10][8]={
	{1,1,1,1,1,1,0,0},
	{0,1,1,0,0,0,0,0},
	{1,1,0,1,1,0,1,0},
	{1,1,1,1,0,0,1,0},
	{0,1,1,0,0,1,1,0},
	{1,0,1,1,0,1,1,0},
	{1,0,1,1,1,1,1,0},
	{1,1,1,0,0,1,0,0},
	{1,1,1,1,1,1,1,0},
	{1,1,1,1,0,1,1,0}
};

typedef struct _model {
    float conv1_weight[I2_C * I1_C * CONV1_KERNAL * CONV1_KERNAL];
    float conv1_bias[I2_C];

    float conv2_weight[I3_C * I2_C * CONV2_KERNAL * CONV2_KERNAL];
    float conv2_bias[I3_C];

    float fc_weight[FC_OUT * FC_IN];
    float fc_bias[FC_OUT];
} model;

void resize_280_to_28(unsigned char *in, unsigned char *out);
void Gray_scale(unsigned char *feature_in, unsigned char *feature_out);
void Normalized(unsigned char *feature_in, float *feature_out);

void Padding(float *feature_in, float *feature_out, int C, int H, int W);
void Conv_2d(float *feature_in, float *feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float *weight, float *bias);
void ReLU(float *feature_in, int elem_num);
void Linear(float *feature_in, float *feature_out, float *weight, float *bias);
void Log_softmax(float *activation);
int Get_pred(float *activation);
void Get_CAM(float *activation, float *cam, int pred, float *weight);
void save_image(float *feature_scaled, float *cam);
void display_sev_seg(int pred);

int main(int argc, char *argv[]) {
    clock_t start1, end1, start2, end2;

    model net;
    FILE *weights;
    weights = fopen("./weights.bin", "rb");
    fread(&net, sizeof(model), 1, weights);

    char *file;
    if (atoi(argv[1]) == 0) {
        /*          PUT YOUR CODE HERE                      */
        /*          Serial communication                    */
        int fd;
        struct termios newtio;
        char fbuf[1024];
        char buf[256];

        fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
        if (fd < 0) {
            fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
            printf("Make sure you are executing in sudo.\r\n");
            exit(1);
        }
        usleep(250000);

        memset(&newtio, 0, sizeof(newtio));
        newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = ICRNL;
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;
        newtio.c_cc[VTIME] = 0;
        newtio.c_cc[VMIN] = 1;

        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);

        while(1){
        // Read from serial until 'c' or 'C' is received
        int cnt = read(fd, buf, sizeof(buf));
        buf[cnt] = '\0';

            if (buf[0] == 'c' || buf[0] == 'C') {
                system("libcamera-still -e bmp --width 280 --height 280 -t 20000 -o image.bmp");
                file = "image.bmp";
                
                // Send the captured image via serial
                FILE *fp = fopen("image.bmp", "rb");
                if (fp == NULL) {
                    fprintf(stderr, "failed to open image file: %s.\r\n", strerror(errno));
                    exit(1);
                }

                while (!feof(fp)) {
                    size_t bytesRead = fread(fbuf, sizeof(char), sizeof(fbuf), fp);
                    write(fd, fbuf, bytesRead);
                }
                fclose(fp);
                
                break;
            } 
        }
        close(fd);
    }
    else if (atoi(argv[1]) == 1) {
        file = "example_1.bmp";
    }
    else if (atoi(argv[1]) == 2) {
        file = "example_2.bmp";
    }
    else {
        printf("Wrong Input!\n");
        exit(1);
    }

    unsigned char *feature_in;
    unsigned char *feature_resize;
    unsigned char feature_gray[I1_C * I1_H * I1_W];
    float feature_scaled[I1_C * I1_H * I1_W];
    float feature_padding1[I1_C * (I1_H + 2) * (I1_W + 2)];
    float feature_conv1_out[I2_C * I2_H * I2_W];
    float feature_padding2[I2_C * (I2_H + 2) * (I2_W + 2)];
    float feature_conv2_out[I3_C * I3_H * I3_W];
    float fc_out[1 * CLASS];
    float cam[1 * I3_H * I3_W];
    int channels, height, width;

    if (atoi(argv[1]) == 0) {
        feature_resize = stbi_load(file, &width, &height, &channels, 3);
        feature_in = (unsigned char *)malloc(sizeof(unsigned char) * 3 * I1_H * I1_W);
        resize_280_to_28(feature_resize, feature_in);
    }
    else {
        feature_in = stbi_load(file, &width, &height, &channels, 3);
    }

    int pred = 0;
    Gray_scale(feature_in, feature_gray);
    Normalized(feature_gray, feature_scaled);
    /***************      Implement these functions      ********************/
    start1 = clock();
    Padding(feature_scaled, feature_padding1, I1_C, I1_H, I1_W);
    Conv_2d(feature_padding1, feature_conv1_out, I1_C, I1_H + 2, I1_W + 2, I2_C, I2_H, I2_W, CONV1_KERNAL, CONV1_STRIDE, net.conv1_weight, net.conv1_bias);
    ReLU(feature_conv1_out, I2_C * I2_H * I2_W);

    Padding(feature_conv1_out, feature_padding2, I2_C, I2_H, I2_W);
    Conv_2d(feature_padding2, feature_conv2_out, I2_C, I2_H + 2, I2_W + 2, I3_C, I3_H, I3_W, CONV2_KERNAL, CONV2_STRIDE, net.conv2_weight, net.conv2_bias);
    ReLU(feature_conv2_out, I3_C * I3_H * I3_W);

    Linear(feature_conv2_out, fc_out, net.fc_weight, net.fc_bias);
    end1 = clock() - start1;

    Log_softmax(fc_out);

    start2 = clock();
    pred = Get_pred(fc_out);
    Get_CAM(feature_conv2_out, cam, pred, net.fc_weight);
    end2 = clock() - start2;
    /************************************************************************/
    save_image(feature_scaled, cam);

    /*          PUT YOUR CODE HERE                      */
    /*          7-segment                               */
    
    if (wiringPiSetup() == -1){
		return 1;
    }
    for(int i = 0; i < 8; i++){
	pinMode(SEGMENT_PINS[i], OUTPUT);
    }

    display_sev_seg(pred);
    
    printf("Log softmax value\n");
    for (int i = 0; i < CLASS; i++) {
        printf("%2d: %6.3f\n", i, fc_out[i]);
    }
    printf("Prediction: %d\n", pred);
    printf("Execution time: %9.3lf[us]\n", (double)(end1 + end2) / CLOCKS_PER_US);

    fclose(weights);
    if (atoi(argv[1]) == 0) {
        free(feature_in);
        stbi_image_free(feature_resize);
    }
    else {
        stbi_image_free(feature_in);
    }
    return 0;
}

void resize_280_to_28(unsigned char *in, unsigned char *out) {
    /*            DO NOT MODIFIY            */
    int x, y, c;
    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            for (c = 0; c < 3; c++) {
                out[y * 28 * 3 + x * 3 + c] = in[y * 10 * 280 * 3 + x * 10 * 3 + c];
            }
        }
    }
    return;
}

void Gray_scale(unsigned char *feature_in, unsigned char *feature_out) {
    /*            DO NOT MODIFIY            */
    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            int sum = 0;
            for (int c = 0; c < 3; c++) {
                sum += feature_in[I1_H * 3 * h + 3 * w + c];
            }
            feature_out[I1_W * h + w] = sum / 3;
        }
    }

    return;
}

void Normalized(unsigned char *feature_in, float *feature_out) {
    /*            DO NOT MODIFIY            */
    for (int i = 0; i < I1_H * I1_W; i++) {
        feature_out[i] = ((float)feature_in[i]) / 255.0;
    }

    return;
}
/*
void Padding(float *feature_in, float *feature_out, int C, int H, int W) {
	//r0: c, r1: h, r2: w
	//r3: padded term, r4: feature_out term, r5: featuer_in term
    asm (
        // Outer loop for channels (c)
        "mov r0, #0\n\t"  // r11 = c = 0
        "c_loop:\n\t"
        "cmp r0, %[C]\n\t"  // if c >= C, break
        "bge c_done\n\t"

        // Inner loop for rows (h)
        "mov r1, #0\n\t"  // r12 = h = 0
        "h_loop:\n\t"
	"add r3, %[H], #2\n\t"
        "cmp r1, r3\n\t"  // if h >= padded_H, break
        "bge h_done\n\t"

        // Inner loop for columns (w)
        "mov r2, #0\n\t"  // r2 = w = 0
        "w_loop:\n\t"
	"add r3, %[W], #2\n\t"
        "cmp r2, r3\n\t"  // if w >= padded_W, break
        "bge w_done\n\t"

        // Calculate index for feature_out	
	"add r3, %[H], #2\n\t" // padded_H
        "mul r4, r0, r3\n\t"  // r4 = c * padded_H
	"add r3, %[W], #2\n\t" // padded_W
        "mul r4, r4, r3\n\t"  // r4 = c * padded_H * padded_W
	"add r3, %[W], #2\n\t" // padded_W
	"mul r3, r3, r1\n\t" //h * padded_W
	"add r4, r4, r3\n\t"
	"add r4, r4, r2\n\t"   // r4 += w
        "ldr r4, [%[feature_out], r4, lsl #2]\n\t"   // r4 += feature_out

        // Check if we are on the border
        "cmp r1, #0\n\t"
        "beq zero_pad\n\t"
	"add r3, %[H], #1\n\t" // padded_H -1
        "cmp r1, r3\n\t"
        "beq zero_pad\n\t"
        "cmp r2, #0\n\t"
        "beq zero_pad\n\t"
	"add r3, %[W], #1\n\t" // padded_W - 1
        "cmp r2, r3\n\t"
        "beq zero_pad\n\t"

        // Not on the border, copy from feature_in
        // Calculate index for feature_in
	"mul r5, r0, %[H]\n\t"
	"mul r5, r5, %[W]\n\t"
        "sub r3, r1, #1\n\t"  // r3 = h - 1
        "mul r3, r3, %[W]\n\t"   // r3 *= W
	"add r5, r5, r3\n\t"
        "sub r3, r2, #1\n\t"   // r3 = w - 1
        "add r5, r5, r3\n\t"   // r5 += w - 1
        "ldr r5, [ %[feature_in], r5, lsl #2]\n\t"     // r0 = feature_in[c * H * W + (h - 1) * W + (w - 1)]
        "b store\n\t"

        "zero_pad:\n\t"
        "mov r5, #0\n\t"  // r5 = 0

        "store:\n\t"
        "str r5, [r4]\n\t"    // feature_out[...] = r0

        "add r2, r2, #1\n\t"  // w++
        "b w_loop\n\t"

        "w_done:\n\t"
        "add r1, r1, #1\n\t"  // h++
        "b h_loop\n\t"

        "h_done:\n\t"
        "add r0, r0, #1\n\t"  // c++
        "b c_loop\n\t"

        "c_done:\n\t"
        // Restore callee-saved registers and return
        "pop {r4-r11, lr}\n\t"
        "bx lr\n\t"
        :
        : [feature_in] "r"(feature_in), [feature_out] "r"(feature_out), [C] "r"(C), [H] "r"(H), [W] "r"(W)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
}
*/
void Padding(float *feature_in, float *feature_out, int C, int H, int W) {
    asm volatile(
        // Outer loop for channels (c)
        "mov r0, #0\n\t"  // r0 = c = 0
        "c_loop:\n\t"
        "cmp r0, %[C]\n\t"  // if c >= C, break
        "bge c_done\n\t"

        // Inner loop for rows (h)
        "mov r1, #0\n\t"  // r1 = h = 0
        "h_loop:\n\t"
        "add r3, %[H], #2\n\t"
        "cmp r1, r3\n\t"  // if h >= padded_H, break
        "bge h_done\n\t"

        // Inner loop for columns (w)
        "mov r2, #0\n\t"  // r2 = w = 0
        "w_loop:\n\t"
        "add r3, %[W], #2\n\t"
        "cmp r2, r3\n\t"  // if w >= padded_W, break
        "bge w_done\n\t"

        // Calculate index for feature_out
        "add r3, %[H], #2\n\t" // padded_H
        "mul r4, r0, r3\n\t"  // r4 = c * padded_H
        "add r3, %[W], #2\n\t" // padded_W
        "mul r4, r4, r3\n\t"  // r4 = c * padded_H * padded_W
        "mul r3, r1, %[W]\n\t" // h * padded_W
        "add r4, r4, r3\n\t"
        "add r4, r4, r2\n\t"   // r4 += w
        "lsl r4, r4, #2\n\t"  // r4 *= 4 (sizeof(float))

        // Check if we are on the border
        "cmp r1, #0\n\t"
        "beq zero_pad\n\t"
        "add r3, %[H], #1\n\t" // padded_H - 1
        "cmp r1, r3\n\t"
        "beq zero_pad\n\t"
        "cmp r2, #0\n\t"
        "beq zero_pad\n\t"
        "add r3, %[W], #1\n\t" // padded_W - 1
        "cmp r2, r3\n\t"
        "beq zero_pad\n\t"

        // Not on the border, copy from feature_in
        // Calculate index for feature_in
        "mul r5, r0, %[H]\n\t"
        "mul r5, r5, %[W]\n\t"
        "sub r3, r1, #1\n\t"  // r3 = h - 1
        "mul r3, r3, %[W]\n\t" // r3 *= W
        "add r5, r5, r3\n\t"
        "sub r3, r2, #1\n\t"  // r3 = w - 1
        "add r5, r5, r3\n\t"  // r5 += w - 1
        "lsl r5, r5, #2\n\t"  // r5 *= 4 (sizeof(float))
        "add r5, %[feature_in], r5\n\t"
        "ldr r5, [r5]\n\t"  // r5 = feature_in[c * H * W + (h - 1) * W + (w - 1)]
        "b store\n\t"

        "zero_pad:\n\t"
        "mov r5, #0\n\t"  // r5 = 0

        "store:\n\t"
        "add r4, %[feature_out], r4\n\t"
        "str r5, [r4]\n\t"  // feature_out[...] = r5

        "add r2, r2, #1\n\t"  // w++
        "b w_loop\n\t"

        "w_done:\n\t"
        "add r1, r1, #1\n\t"  // h++
        "b h_loop\n\t"

        "h_done:\n\t"
        "add r0, r0, #1\n\t"  // c++
        "b c_loop\n\t"

        "c_done:\n\t"
        :
        : [feature_in] "r"(feature_in), [feature_out] "r"(feature_out), [C] "r"(C), [H] "r"(H), [W] "r"(W)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
}

#include <arm_neon.h>

void Conv_2d(float *feature_in, float *feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float *weight, float *bias) {
    asm volatile (
        // Clear all temporary registers
        "mov r0, #0\n\t"  // oc loop index
        "mov r1, #0\n\t"  // oh loop index
        "mov r2, #0\n\t"  // ow loop index
        "mov r3, #0\n\t"  // ic loop index
        "mov r4, #0\n\t"  // kh loop index
        "mov r5, #0\n\t"  // kw loop index

        // Outer loop for output channels (oc)
        "oc_loop:\n\t"
        "cmp r0, %[out_C]\n\t"
        "bge oc_done\n\t"

        // Inner loop for output height (oh)
        "oh_loop:\n\t"
        "cmp r1, %[out_H]\n\t"
        "bge oh_done\n\t"

        // Inner loop for output width (ow)
        "ow_loop:\n\t"
        "cmp r2, %[out_W]\n\t"
        "bge ow_done\n\t"

        // Initialize sum with bias
        "ldr q0, [%[bias], r0, lsl #2]\n\t"  // q0 = sum = bias[oc]

        // Loop over input channels (ic)
        "ic_loop:\n\t"
        "cmp r3, %[in_C]\n\t"
        "bge ic_done\n\t"

        // Loop over kernel height (kh)
        "kh_loop:\n\t"
        "cmp r4, %[K]\n\t"
        "bge kh_done\n\t"

        // Loop over kernel width (kw)
        "kw_loop:\n\t"
        "cmp r5, %[K]\n\t"
        "bge kw_done\n\t"

        // Calculate input height and width indices
        "mul r6, r1, %[S]\n\t"  // r6 = oh * S
        "add r6, r6, r4\n\t"  // r6 += kh
        "mul r10, r2, %[S]\n\t"  // r7 = ow * S
        "add r10, r10, r5\n\t"  // r7 += kw

        // Load input feature value
        "mul r8, r3, %[in_H]\n\t"
        "mul r8, r8, %[in_W]\n\t"  // r8 = ic * in_H * in_W
        "add r8, r8, r6\n\t"  // r8 += ih
        "mul r8, r8, %[in_W]\n\t"
        "add r8, r8, r10\n\t"  // r8 += iw
        "lsl r8, r8, #2\n\t"  // r8 *= 4 (sizeof(float))
        "vld1.32 {d0-d1}, [%[feature_in], r8]\n\t"  // d0 = feature_in[ic * in_H * in_W + ih * in_W + iw]

        // Load weight value
        "mul r9, r0, %[in_C]\n\t"
        "mul r9, r9, %[K]\n\t"
        "mul r9, r9, %[K]\n\t"  // r9 = oc * in_C * K * K
        "add r9, r9, r3\n\t"
        "mul r9, r9, %[K]\n\t"
        "add r9, r9, r4\n\t"
        "mul r9, r9, %[K]\n\t"
        "add r9, r9, r5\n\t"  // r9 += ic * K * K + kh * K + kw
        "lsl r9, r9, #2\n\t"  // r9 *= 4 (sizeof(float))
        "vld1.32 {d2-d3}, [%[weight], r9]\n\t"  // d2 = weight[oc * in_C * K * K + ic * K * K + kh * K + kw]

        // Multiply and accumulate
        "vmla.f32 q0, q1, q2\n\t"

        "add r5, r5, #1\n\t"  // kw++
        "b kw_loop\n\t"

        "kw_done:\n\t"
        "add r4, r4, #1\n\t"  // kh++
        "b kh_loop\n\t"

        "kh_done:\n\t"
        "add r3, r3, #1\n\t"  // ic++
        "b ic_loop\n\t"

        "ic_done:\n\t"
        // Store the result to output feature map
        "mul r6, r0, %[out_H]\n\t"
        "mul r6, r6, %[out_W]\n\t"  // r6 = oc * out_H * out_W
        "add r6, r6, r1\n\t"
        "mul r6, r6, %[out_W]\n\t"
        "add r6, r6, r2\n\t"  // r6 += oh * out_W + ow
        "lsl r6, r6, #2\n\t"  // r6 *= 4 (sizeof(float))
        "vst1.32 {q0}, [%[feature_out], r6]\n\t"

        "add r2, r2, #1\n\t"  // ow++
        "b ow_loop\n\t"

        "ow_done:\n\t"
        "add r1, r1, #1\n\t"  // oh++
        "b oh_loop\n\t"

        "oh_done:\n\t"
        "add r0, r0, #1\n\t"  // oc++
        "b oc_loop\n\t"

        "oc_done:\n\t"
        :
        : [feature_in] "r"(feature_in), [feature_out] "r"(feature_out), [in_C] "r"(in_C), [in_H] "r"(in_H), [in_W] "r"(in_W), [out_C] "r"(out_C), [out_H] "r"(out_H), [out_W] "r"(out_W), [K] "r"(K), [S] "r"(S), [weight] "r"(weight), [bias] "r"(bias)
        : "q0", "q1", "q2", "q3", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "memory"
    );
}



void ReLU(float *feature_in, int elem_num){
    /*          PUT YOUR CODE HERE          */
    // ReLU input : float *feature_in
    // ReLU output: float *feature_in
    float32x4_t zero = vdupq_n_f32(0.0f);
    #pragma omp parallel for
    for (int i = 0; i < elem_num; i += 4) {
        float32x4_t in_vec = vld1q_f32(&feature_in[i]);
        float32x4_t result = vmaxq_f32(in_vec, zero);
        vst1q_f32(&feature_in[i], result);
    }
}

void Linear(float *feature_in, float *feature_out, float *weight, float *bias) {
    /*          PUT YOUR CODE HERE          */
    // Linear input : float *feature_in
    // Linear output: float *feature_out
    #pragma omp parallel for
    for (int out = 0; out < FC_OUT; out++) {
        float32x4_t sum_vec = vdupq_n_f32(0.0f);
        float sum = bias[out];
        for (int in = 0; in < FC_IN; in += 4) {
            float32x4_t in_vec = vld1q_f32(&feature_in[in]);
            float32x4_t w_vec = vld1q_f32(&weight[out * FC_IN + in]);
            sum_vec = vmlaq_f32(sum_vec, in_vec, w_vec);
        }
        sum += vgetq_lane_f32(sum_vec, 0) + vgetq_lane_f32(sum_vec, 1) + vgetq_lane_f32(sum_vec, 2) + vgetq_lane_f32(sum_vec, 3);
        feature_out[out] = sum;
    }
}

void Log_softmax(float *activation) {
    /*            DO NOT MODIFIY            */
    double max = activation[0];
    double sum = 0.0;

    for (int i = 1; i < CLASS; i++) {
        if (activation[i] > max) {
            max = activation[i];
        }
    }

    for (int i = 0; i < CLASS; i++) {
        activation[i] = exp(activation[i] - max);
        sum += activation[i];
    }

    for (int i = 0; i < CLASS; i++) {
        activation[i] = log(activation[i] / sum);
    }

    return;
}

int Get_pred(float *activation) {
    /*          PUT YOUR CODE HERE          */
    // Get_pred input : float *activation
    // Get_pred output: int pred
    float max_val = activation[0];
    int pred = 0;

    for (int i = 1; i < CLASS; i++) {
        if (activation[i] > max_val) {
            max_val = activation[i];
            pred = i;
        }
    }

    return pred;
}

void Get_CAM(float *activation, float *cam, int pred, float *weight) {
    /*          PUT YOUR CODE HERE          */
    // Get_CAM input : float *activation
    // Get_CAM output: float *cam
    #pragma omp parallel for
    for (int h = 0; h < I3_H; h++) {
        for (int w = 0; w < I3_W; w += 4) {
            float32x4_t act_vec = vld1q_f32(&activation[h * I3_W + w]);
            float32x4_t w_vec = vld1q_f32(&weight[pred * FC_IN + h * I3_W + w]);
            float32x4_t cam_vec = vmulq_f32(act_vec, w_vec);
            vst1q_f32(&cam[h * I3_W + w], cam_vec);
        }
    }
}

void save_image(float *feature_scaled, float *cam) {
    /*            DO NOT MODIFIY            */
    float *output = (float *)malloc(sizeof(float) * 3 * I1_H * I1_W);
    unsigned char *output_bmp = (unsigned char *)malloc(sizeof(unsigned char) * 3 * I1_H * I1_W);
    unsigned char *output_bmp_resized = (unsigned char *)malloc(sizeof(unsigned char) * 3 * I1_H * 14 * I1_W * 14);

    float min = cam[0];
    float max = cam[0];
    for (int i = 1; i < I3_H * I3_W; i++) {
        if (cam[i] < min) {
            min = cam[i];
        }
        if (cam[i] > max) {
            max = cam[i];
        }
    }

    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            for (int c = 0; c < 3; c++) {
                output[I1_H * I1_W * c + I1_W * h + w] = (cam[I3_W * (h >> 1) + (w >> 1)] - min) / (max - min);
            }
        }
    }

    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            for (int c = 0; c < 3; c++) {
                output_bmp[I1_H * 3 * h + 3 * w + c] = (output[I1_H * I1_W * c + I1_W * h + w]) * 255;
            }
        }
    }

    stbir_resize_uint8_linear(output_bmp, I1_H, I1_W, 0, output_bmp_resized, I1_H * 14, I1_W * 14, 0, 3);
    stbi_write_bmp("Activation_map.bmp", I1_W * 14, I1_H * 14, 3, output_bmp_resized);

    free(output);
    free(output_bmp);
    return;
}
void display_sev_seg(int pred){
	for(int i = 0; i < 8; i++)	
	digitalWrite(SEGMENT_PINS[i], sev_seg[pred][i]);
}
