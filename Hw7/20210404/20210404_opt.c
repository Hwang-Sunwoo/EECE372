#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <wiringPi.h>

#include <omp.h>
#include <arm_neon.h>


#define BAUDRATE B1000000

#define MNIST_IMAGE_WIDTH 28
#define MNIST_IMAGE_HEIGHT 28
#define MNIST_IMAGE_SIZE MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT
#define MNIST_LABELS 10
#define CHANNEL_NUMBER 32
#define CHANNEL_WIDTH 3
#define CHANNEL_HEIGHT 3
#define PIXEL_SCALE(x) (((float) (x)) / 255.0f)

#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define F 5
#define G 6
#define DP 7


typedef struct neural_network_t_ {
    float conv_bias[CHANNEL_NUMBER];
    float conv_weight[CHANNEL_NUMBER][CHANNEL_HEIGHT][CHANNEL_WIDTH];
    float fc_bias[MNIST_LABELS];
    float fc_weight[MNIST_LABELS][MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT * CHANNEL_NUMBER];
} neural_network_t;


void resize_280_to_28(unsigned char* out, unsigned char* in) {
    int x, y, c;

    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            for (c = 0; c < 3; c++) {
                out[y * 28 * 3 + x * 3 + c] = in[y * 10 * 280 * 3 + x * 10 * 3 + c];
            }
        }
    }
}


void RGB_to_Grayscale(uint8_t out[][MNIST_IMAGE_WIDTH], unsigned char* in) {
    int x, y, c;
    int sum = 0;

    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            sum = 0;
            for (c = 0; c < 3; c++) {
                sum += in[y * 28 * 3 + x * 3 + c];
            }
            for (c = 0; c < 3; c++) {
                in[y * 28 * 3 + x * 3 + c] = 255 - sum / 3;
            }

        }
    }

    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            sum = 0;
            for (c = 0; c < 3; c++) {
                sum += in[y * 28 * 3 + x * 3 + c];
            }
            if (sum / 3 < 150)
                out[y][x];
            else
                out[y][x] = sum / 3;
        }
    }
}


void pixel_scale(float out[][MNIST_IMAGE_WIDTH], uint8_t in[][MNIST_IMAGE_WIDTH]) {
    int i;

    for (i = 0; i < 28 * 28; i++) {
        out[i / 28][i % 28] = PIXEL_SCALE(in[i / 28][i % 28]);
    }
}


void zero_padding_opt(float out[][30], float in[][28]) {
    /*          PUT YOUR CODE HERE          */
    int i, j;

    for (i = 0; i < 30; i++) {
        out[i][0] = 0.0;
        out[0][i] = 0.0;
        out[i][29] = 0.0;
        out[29][i] = 0.0;
    }
    
    for (i = 1; i < 29; i++) {
        for (j = 1; j < 29; j++) {
            out[i][j] = in[i - 1][j - 1];
        }
    }
}


void convolution_opt(float out[][28][28], float in[][30], neural_network_t network) {
    /*          PUT YOUR CODE HERE          */
    int k, m, channel, i;
    
    float32x4_t filter_1, filter_2, filter_3;
    
    float32x4_t arr_1, arr_2, arr_3, ans;
        
    for (channel = 0; channel < 32; channel++) {
        
        filter_1 = vld1q_f32(network.conv_weight[channel][0]); // network.conv_weight[channel][0][0], network.conv_weight[channel][0][1], network.conv_weight[channel][0][2]
        filter_2 = vld1q_f32(network.conv_weight[channel][1]); // network.conv_weight[channel][1][0], network.conv_weight[channel][1][1], network.conv_weight[channel][1][2]
        filter_3 = vld1q_f32(network.conv_weight[channel][2]); // network.conv_weight[channel][2][0], network.conv_weight[channel][2][1], network.conv_weight[channel][2][2]
        
        for (k = 0; k < 28; k++) {
            for (m = 0; m < 28; m++) {
                arr_1 = vld1q_f32(in[k] + m); // in[k][m], in[k][m+1], in[k][m+1]
                arr_2 = vld1q_f32(in[k+1] + m); // in[k+1][m], in[k+1][m+1], in[k+1][m+1]
                arr_3 = vld1q_f32(in[k+2] + m); // in[k+2][m], in[k+2][m+1], in[k+2][m+1]
                
                arr_1 = vmulq_f32(arr_1, filter_1);
                arr_2 = vmulq_f32(arr_2, filter_2);
                arr_3 = vmulq_f32(arr_3, filter_3);
                
                ans = vaddq_f32(arr_1, arr_2);
                ans = vaddq_f32(ans, arr_3);                
                                
                out[channel][k][m] = ans[0] + ans[1] + ans[2] + network.conv_bias[channel];                
            }
        }
    }
}


void relu_opt(float out[], float in[][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]) {
    /*          PUT YOUR CODE HERE          */
    asm(
        // r0 : out / r1 : in
        // r4 : i / r5 : j / r6 : channel / r7 : in[channel][i][j] / r8 : address offset
        
        "MOV r8, #0\n\t" // address offset
        
        // for (channel = 0; channel < 32; channel++) {
        "MOV r6, #0\n\t" // channel = 0
        "B FOR1\n\t"
        
    "FOR2:\n\t"   
            // for (i = 0; i < 28; i++) {
            "MOV r4, #0\n\t" // i = 0
            "B FOR3\n\t"
            
        "FOR4:\n\t"
                // for (j = 0; j < 28; j++) {
                "MOV r5, #0\n\t" // j = 0
                "B FOR5\n\t"
                
            "FOR6:\n\t"
                    //if (in[channel][i][j] > 0)
                    "LDR r7, [%[in], r8]\n\t" // r7 = in[channel][i][j]
                    "CMP r7, #0\n\t" // if (in[channel][i][j] > 0)
                    "BLT IF1\n\t"
                        // out[channel * MNIST_IMAGE_SIZE + i * 28 + j] = in[channel][i][j];
                        "STR r7, [%[out], r8]\n\t" // out[channel * MNIST_IMAGE_SIZE + i * 28 + j] = in[channel][i][j] = r7 
                        "B IF2\n\t"
                "IF1:\n\t"    
                    //else
                        // out[channel * MNIST_IMAGE_SIZE + j * 28 + j] = 0;
                        "MOV r7, #0\n\t" // r7 = 0
                        "STR r7, [%[out], r8]\n\t" // out[channel * MNIST_IMAGE_SIZE + i * 28 + j] = r7
                "IF2:\n\t"        
                        
                "ADD r5, r5, #1\n\t" // j++
                "ADD r8, r8, #4\n\t" // set address offset
                
            "FOR5:\n\t"
                "CMP r5, #28\n\t" // j < 28
                "BLT FOR6\n\t"
                //}
                
            "ADD r4, r4, #1\n\t" // i++
            
        "FOR3:\n\t"
            "CMP r4, #28\n\t" // i < 28
            "BLT FOR4\n\t"
            //}
            
        "ADD r6, r6, #1\n\t" // channel++
            
    "FOR1:\n\t"
        "CMP r6, #32\n\t" // channel < 32
        "BLT FOR2\n\t"
        //}
        
        :
        // input operands
        :
        [out] "r"(out), [in] "r"(in)
        // output operands
        :
        "r0", "r1", "r4", "r5", "r6", "r7", "r8"
        // clobbers
    );
}


void fc_opt(float out[], float in[], neural_network_t network) {
    /*          PUT YOUR CODE HERE          */
    int i, j;
    
    float32x4_t vec_in, vec_weight;
    float32x4_t ans;
    float32x4_t sum = vdupq_n_f32(0);

    for (i = 0; i < MNIST_LABELS; i++) {
        for (j = 0; j < MNIST_IMAGE_SIZE * CHANNEL_NUMBER; j = j + 4) { // 28 * 28 * 32
            
            //sum = sum + (in[j] * network.fc_weight[i][j]);

            vec_in = vld1q_f32(in + j); // put in[j] address -> in[j], in[j+1], in[j+2], in[j+3]
            vec_weight = vld1q_f32(network.fc_weight[i] + j); // put network.fc_weight[i][j] address -> network.fc_weight[i][j], network.fc_weight[i][j+1], network.fc_weight[i][j+2], network.fc_weight[i][j+3]
            
            ans = vmulq_f32(vec_in, vec_weight);
            
            sum = vaddq_f32(sum, ans);
        }
        //out[i] = sum + network.fc_bias[i];   
        out[i] = sum[0] + sum[1] + sum[2] + sum[3] + network.fc_bias[i];
        
        //sum = 0.0;
        sum = vdupq_n_f32(0);
    }
}


void softmax(float activations[], int length) {
    int i;
    float sum, max;

    for (i = 1, max = activations[0]; i < length; i++) {
        if (activations[i] > max) {
            max = activations[i];
        }
    }

    for (i = 0, sum = 0; i < length; i++) {
        activations[i] = exp(activations[i] - max);
        sum += activations[i];
    }

    for (i = 0; i < length; i++) {
        activations[i] /= sum;
    }
}


int camera_Mode() {
    int fd;
    struct termios newtio;
    char buf[256];

    fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
        printf("Make sure you are executing in sudo.\r\n");
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

    while (1)
    {
        int cnt = read(fd, buf, sizeof(buf));
        buf[cnt] = '\0';

        if (strcmp(buf, "c") == 0 || strcmp(buf, "C") == 0)
        {
            system("raspistill -w 280 -h 280 -t 10 -o project.bmp");
            break;
        }
    }
    return 0;
}


int display_Digit(int digit) {
    switch (digit) {
    case 0:
        digitalWrite(A, 1);
        digitalWrite(B, 1);
        digitalWrite(C, 1);
        digitalWrite(D, 1);
        digitalWrite(E, 1);
        digitalWrite(F, 1);
        digitalWrite(G, 0);
        digitalWrite(DP, 0);
        break;
    case 1:
        digitalWrite(A, 0);
        digitalWrite(B, 1);
        digitalWrite(C, 1);
        digitalWrite(D, 0);
        digitalWrite(E, 0);
        digitalWrite(F, 0);
        digitalWrite(G, 0);
        digitalWrite(DP, 0);
        break;
    case 2:
        digitalWrite(A, 1);
        digitalWrite(B, 1);
        digitalWrite(C, 0);
        digitalWrite(D, 1);
        digitalWrite(E, 1);
        digitalWrite(F, 0);
        digitalWrite(G, 1);
        digitalWrite(DP, 0);
        break;
    case 3:
        digitalWrite(A, 1);
        digitalWrite(B, 1);
        digitalWrite(C, 1);
        digitalWrite(D, 1);
        digitalWrite(E, 0);
        digitalWrite(F, 0);
        digitalWrite(G, 1);
        digitalWrite(DP, 0);
        break;
    case 4:
        digitalWrite(A, 0);
        digitalWrite(B, 1);
        digitalWrite(C, 1);
        digitalWrite(D, 0);
        digitalWrite(E, 0);
        digitalWrite(F, 1);
        digitalWrite(G, 1);
        digitalWrite(DP, 0);
        break;
    case 5:
        digitalWrite(A, 1);
        digitalWrite(B, 0);
        digitalWrite(C, 1);
        digitalWrite(D, 1);
        digitalWrite(E, 0);
        digitalWrite(F, 1);
        digitalWrite(G, 1);
        digitalWrite(DP, 0);
        break;
    case 6:
        digitalWrite(A, 1);
        digitalWrite(B, 0);
        digitalWrite(C, 1);
        digitalWrite(D, 1);
        digitalWrite(E, 1);
        digitalWrite(F, 1);
        digitalWrite(G, 1);
        digitalWrite(DP, 0);
        break;
    case 7:
        digitalWrite(A, 1);
        digitalWrite(B, 1);
        digitalWrite(C, 1);
        digitalWrite(D, 0);
        digitalWrite(E, 0);
        digitalWrite(F, 1);
        digitalWrite(G, 0);
        digitalWrite(DP, 0);
        break;
    case 8:
        digitalWrite(A, 1);
        digitalWrite(B, 1);
        digitalWrite(C, 1);
        digitalWrite(D, 1);
        digitalWrite(E, 1);
        digitalWrite(F, 1);
        digitalWrite(G, 1);
        digitalWrite(DP, 0);
        break;
    case 9:
        digitalWrite(A, 1);
        digitalWrite(B, 1);
        digitalWrite(C, 1);
        digitalWrite(D, 1);
        digitalWrite(E, 0);
        digitalWrite(F, 1);
        digitalWrite(G, 1);
        digitalWrite(DP, 0);
        break;
    }
}



int main(int argc, char* argv[]) {
    int height;
    int width;
    int channels;
    int x, y, c, i, j;
    int sum;
    clock_t start, end, start_opt, end_opt;

    neural_network_t network;

    FILE* weight;
    weight = fopen("./weight.bin", "rb");
    fread(&network, sizeof(neural_network_t), 1, weight);


    /*          PUT YOUR CODE HERE                      */
    if (argc < 2) {
        printf("Error1: Input the mode(0:camera mode, 1,2:example mode\n");
        return -1;
    }
    int mode = atoi(argv[1]);
    char bmpFileName[20] = { 0 };
    switch (mode) {
    case 0:
        camera_Mode();
        break;
    case 1:
        system("cp example_1.bmp project.bmp");
        break;
    case 2:
        system("cp example_2.bmp project.bmp");
        break;
    default:
        printf("Error2: Unavailable mode\n");
        return -1;
    }
    /*          Capture image(project.bmp)              */


    unsigned char* feature_in = stbi_load("project.bmp", &width, &height, &channels, 3);
    unsigned char* feature_resized = (unsigned char*) malloc (sizeof(unsigned char)*28*28*3);
    unsigned char feature_gray[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_scaled[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]; 
    float feature_zeroPadding_opt[30][30];
    float feature_conv_opt[CHANNEL_NUMBER][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_relu_opt[MNIST_IMAGE_SIZE*CHANNEL_NUMBER];
    float activations_opt[MNIST_LABELS];


    resize_280_to_28(feature_resized, feature_in);
    RGB_to_Grayscale(feature_gray, feature_resized);
    pixel_scale(feature_scaled, feature_gray);

    /***************    Implement these functions       ********************/
    start = clock();
    zero_padding_opt(feature_zeroPadding_opt, feature_scaled);
    convolution_opt(feature_conv_opt, feature_zeroPadding_opt, network);
    relu_opt(feature_relu_opt, feature_conv_opt);
    fc_opt(activations_opt, feature_relu_opt, network);
    printf("Execution time   (opt): %.3lf ms\n", (double)clock()-start);
    /***********************************************************************/

    softmax(activations_opt, MNIST_LABELS);
    
    printf("\n");
    printf("softmax value (opt)\n");
    for (i = 0; i < 10; i++) {
        printf("%d : %f\n", i, activations_opt[i]);
    }
    
    int result_opt = 0;
    for (i = 0; i < 9; i++)
        if (activations_opt[result_opt] < activations_opt[i + 1])
            result_opt = i + 1;
    printf("result   (opt): %d\n", result_opt);


    /*          PUT YOUR CODE HERE                      */
    if (wiringPiSetup() == -1) {
        return 1;
    }
    pinMode(A, OUTPUT);
    pinMode(B, OUTPUT);
    pinMode(C, OUTPUT);
    pinMode(D, OUTPUT);
    pinMode(E, OUTPUT);
    pinMode(F, OUTPUT);
    pinMode(G, OUTPUT);
    pinMode(DP, OUTPUT);


    display_Digit(result_opt);
    /*          7-segment                               */
    

    fclose(weight);
    stbi_image_free(feature_in);
    free(feature_resized);

    return 0;
}

