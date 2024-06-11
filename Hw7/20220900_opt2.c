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
                /*
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
                */
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
    int padded_H = H + 2;
    int padded_W = W + 2;

    #pragma omp parallel for collapse(2)
    for (int c = 0; c < C; c++) {
        for (int h = 0; h < padded_H; h++) {
            for (int w = 0; w < padded_W; w += 4) { // NEON: process 4 elements at a time
                if (h == 0 || h == padded_H - 1 || w == 0 || w == padded_W - 1) {
                    // Set 4 elements to 0 using NEON
                    asm(
                        // Calculate address: feature_out + (c * padded_H * padded_W + h * padded_W + w)
                        "mul r0, %[c], %[padded_H]\n\t"
                        "mul r0, r0, %[padded_W]\n\t"
                        "mul r1, %[h], %[padded_W]\n\t"
                        "add r0, r0, r1\n\t"
                        "add r0, r0, %[w]\n\t"
                        "lsl r0, r0, #2\n\t"
                        "add r0, %[feature_out], r0\n\t"

                        // Set 4 elements to 0
                        "vmov.f32 q0, #0.0\n\t"
                        "vst1.32 {q0}, [r0]\n\t"
                        :
                        : [feature_out] "r" (feature_out), [c] "r" (c), [h] "r" (h), [w] "r" (w), [padded_H] "r" (padded_H), [padded_W] "r" (padded_W)
                        : "r0", "r1", "q0", "memory"
                    );
                } else {
                    // Load input values from feature_in and store in feature_out
                    asm volatile (
                        "mul r0, %[c], %[H]\n\t"
                        "mul r0, r0, %[W]\n\t"
                        "sub r1, %[h], #1\n\t"
                        "mul r1, r1, %[W]\n\t"
                        "add r0, r0, r1\n\t"
                        "sub r1, %[w], #1\n\t"
                        "add r1, r1, r0\n\t"
                        "lsl r1, r1, #2\n\t"
                        "add r1, %[feature_in], r1\n\t"
                        "ldr r1, [r1]\n\t" // Load input values

                        // Calculate address: feature_out + (c * padded_H * padded_W + h * padded_W + w)
                        "mul r0, %[c], %[padded_H]\n\t"
                        "mul r0, r0, %[padded_W]\n\t"
                        "mul r1, %[h], %[padded_W]\n\t"
                        "add r0, r0, r1\n\t"
                        "add r0, r0, %[w]\n\t"
                        "lsl r0, r0, #2\n\t"
                        "add r0, %[feature_out], r0\n\t"

                        "vst1.32 {q0}, [r0]\n\t" // Store 4 values to feature_out
                        :
                        : [feature_in] "r" (feature_in), [feature_out] "r" (feature_out), [c] "r" (c), [h] "r" (h), [w] "r" (w), [H] "r" (H), [W] "r" (W), [padded_H] "r" (padded_H), [padded_W] "r" (padded_W)
                        : "r0", "r1", "q0", "memory"
                    );
                }
            }
        }
    }
}
*/
/*
void Padding(float *feature_in, float *feature_out, int C, int H, int W) {
    int padded_H = H + 2;
    int padded_W = W + 2;

    #pragma omp parallel for collapse(3)
    for (int c = 0; c < C; c++) {
        for (int h = 0; h < padded_H; h++) {
            for (int w = 0; w < padded_W; w += 4) {
                if (h == 0 || h == padded_H - 1 || w == 0 || w >= padded_W - 4) {
                    // 네온을 사용하여 4개의 요소를 0으로 설정
                    float32x4_t zero_vec = vdupq_n_f32(0.0);
                    vst1q_f32(&feature_out[c * padded_H * padded_W + h * padded_W + w], zero_vec);
                } else {
                    // feature_in에서 값을 로드하고 feature_out에 저장
                    float32x4_t input_vec = vld1q_f32(&feature_in[c * H * W + (h - 1) * W + (w - 1)]);
                    vst1q_f32(&feature_out[c * padded_H * padded_W + h * padded_W + w], input_vec);
                }
            }
        }
    }
}
*/
void Padding(float *feature_in, float *feature_out, int C, int H, int W) {
    int padded_H = H + 2;
    int padded_W = W + 2;

    #pragma omp parallel for
    for (int c = 0; c < C; c++) {
        for (int h = 0; h < padded_H; h++) {
            for (int w = 0; w < padded_W; w++) {
                int out_index = c * padded_H * padded_W + h * padded_W + w;
                if (h == 0 || h == padded_H - 1 || w == 0 || w == padded_W - 1) {
                    __asm__ volatile (
                        "mov r0, #0\n\t"
                        "vdup.f32 d0, r0\n\t"
                        "vstr d0, [%[out]]\n\t"
                        :
                        : [out] "r" (&feature_out[out_index])
                        : "r0", "d0", "memory"
                    );
                } else {
                    int in_index = c * H * W + (h - 1) * W + (w - 1);
                    __asm__ volatile (
                        "vldr d0, [%[in]]\n\t"
                        "vstr d0, [%[out]]\n\t"
                        :
                        : [in] "r" (&feature_in[in_index]), [out] "r" (&feature_out[out_index])
                        : "d0", "memory"
                    );
                }
            }
        }
    }
}

void Conv_2d(float *feature_in, float *feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float *weight, float *bias) {
    #pragma omp parallel for collapse(3)
    for (int oc = 0; oc < out_C; oc++) {
        for (int oh = 0; oh < out_H; oh++) {
            for (int ow = 0; ow < out_W; ow++) {
                float32x4_t sum_vec = vdupq_n_f32(0.0f);
                float sum_scalar = bias[oc];
                int ih_base = oh * S;
                int iw_base = ow * S;

                for (int ic = 0; ic < in_C; ic++) {
                    for (int kh = 0; kh < K; kh++) {
                        int kw;
                        for (kw = 0; kw <= K - 4; kw += 4) {
                            int ih = ih_base + kh;
                            int iw = iw_base + kw;

                            float32x4_t feature_vec = vld1q_f32(&feature_in[ic * in_H * in_W + ih * in_W + iw]);
                            float32x4_t weight_vec = vld1q_f32(&weight[oc * in_C * K * K + ic * K * K + kh * K + kw]);
                            
                            sum_vec = vmlaq_f32(sum_vec, feature_vec, weight_vec);
                        }
                        // 처리되지 않은 나머지 요소를 수동으로 처리
                        for (; kw < K; kw++) {
                            int ih = ih_base + kh;
                            int iw = iw_base + kw;
                            sum_scalar += feature_in[ic * in_H * in_W + ih * in_W + iw] * weight[oc * in_C * K * K + ic * K * K + kh * K + kw];
                        }
                    }
                }

                // Sum the elements of the NEON vector
                sum_scalar += vgetq_lane_f32(sum_vec, 0) + vgetq_lane_f32(sum_vec, 1) + vgetq_lane_f32(sum_vec, 2) + vgetq_lane_f32(sum_vec, 3);
                feature_out[oc * out_H * out_W + oh * out_W + ow] = sum_scalar;
            }
        }
    }
}

void ReLU(float *feature_in, int elem_num) {
    asm(
        "mov r2, #0\n\t"  // r2를 0으로 초기화 i
        "relu_loop_start:\n\t"
        "cmp r2, %[elem_num]\n\t"  // r2와 elem_num을 비교 )
        "beq relu_loop_end\n\t"    // r2가 elem_num과 같으면 루프를 종료
        "ldr r3, [%[feature_in], r2, LSL #2]\n\t"  // feature_in + r2 * 4에서 값을 로드하여 r3에 저장 
        "cmp r3, #0\n\t"    // r3와 0을 비교
        "bge relu_no_change\n\t"  // r3가 0보다 크거나 같으면 값을 변경하지 않음
        "mov r3, #0\n\t"    // r3를 0으로 설정
        "str r3, [%[feature_in], r2, LSL #2]\n\t"  // feature_in + r2 * 4에 r3를 저장 
        "relu_no_change:\n\t"
        "add r2, r2, #1\n\t"  // r2를 1 증가 i++
        "b relu_loop_start\n\t"  // 루프 시작으로 돌아감
        "relu_loop_end:\n\t"
        :
        : [feature_in] "r"(feature_in), [elem_num] "r"(elem_num)
        : "r2", "r3", "memory"
    );
}

void Linear(float *feature_in, float *feature_out, float *weight, float *bias) {
    for (int out = 0; out < FC_OUT; out++) {
        float sum = bias[out];
        
        asm volatile(
            "mov r0, %[feature_in]\n\t"
            "mov r1, %[weight]\n\t"
            "mov r2, %[sum]\n\t"
            "mov r3, %[in_size]\n\t"
            "mov r4, #0\n\t"

        "1:\n\t"
            "vld1.32 {q0}, [r0]!\n\t"   // Load 4 elements from feature_in
            "vld1.32 {q1}, [r1]!\n\t"   // Load 4 elements from weight
            "vmla.f32 q2, q0, q1\n\t"   // Multiply and accumulate
            "add r4, r4, #4\n\t"        // Increment loop counter by 4
            "cmp r4, r3\n\t"
            "blt 1b\n\t"

            "vpadd.f32 d0, d4, d5\n\t"  // Pairwise add
            "vpadd.f32 d0, d0, d0\n\t"  // Pairwise add
            "vadd.f32 %[sum], %[sum], s0\n\t"  // Add the accumulated sum

            : [sum] "+w" (sum)
            : [feature_in] "r" (feature_in), [weight] "r" (weight + out * FC_IN), [in_size] "r" (FC_IN)
            : "r0", "r1", "r2", "r3", "r4", "q0", "q1", "q2", "memory"
        );

        feature_out[out] = sum;
    }
    return;
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
    int pred;
    asm volatile (
        "mov r1, #0\n\t"  // pred = 0
        "vldr s0, [%[activation]]\n\t"  // max_val = activation[0]
        "mov r2, #1\n\t"  // i = 1
        
    "get_pred_loop_start:\n\t"
        "cmp r2, %[class]\n\t"  // if (i >= CLASS) break
        "bge get_pred_loop_end\n\t"
        "add r4, %[activation], r2, LSL #2\n\t"  // address of activation[i]
        "vldr s1, [r4]\n\t"  // load activation[i]
        "vcmp.f32 s1, s0\n\t"  // if (activation[i] > max_val)
        "vmrs APSR_nzcv, fpscr\n\t"  // Move FP status to APSR
        "ble get_pred_continue_loop\n\t"  // if not greater, continue
        "vmov.f32 s0, s1\n\t"  // max_val = activation[i]
        "mov r1, r2\n\t"  // pred = i
        
    "get_pred_continue_loop:\n\t"
        "add r2, r2, #1\n\t"  // i++
        "b get_pred_loop_start\n\t"
        
    "get_pred_loop_end:\n\t"
        "mov %[pred], r1\n\t"  // pred = r1
        : [pred] "=r" (pred)
        : [activation] "r" (activation), [class] "r" (CLASS)
        : "r1", "r2", "r3", "r4", "s0", "s1", "memory"
    );
    return pred;
}

void Get_CAM(float *activation, float *cam, int pred, float *weight) {
    /*          PUT YOUR CODE HERE          */
    // Get_CAM input : float *activation
    // Get_CAM output: float *cam
    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);

    #pragma omp parallel for collapse(2)
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
