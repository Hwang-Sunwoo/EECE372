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

#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#include <arm_neon.h>

#include <wiringPi.h>

#define MNIST_IMAGE_WIDTH 28
#define MNIST_IMAGE_HEIGHT 28
#define MNIST_IMAGE_SIZE MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT
#define MNIST_LABELS 10
#define CHANNEL_NUMBER 32
#define CHANNEL_WIDTH 3
#define CHANNEL_HEIGHT 3
#define PIXEL_SCALE(x) (((float) (x)) / 255.0f)

#define BAUDRATE B1000000
#define SW 5
#define segA 27
#define segB 26
#define segC 10
#define segD 11
#define segE 31
#define segF 28
#define segG 29

typedef struct neural_network_t_ {
    float conv_bias[CHANNEL_NUMBER];
    float conv_weight[CHANNEL_NUMBER][CHANNEL_HEIGHT][CHANNEL_WIDTH];
    float fc_bias[MNIST_LABELS];
    float fc_weight[MNIST_LABELS][MNIST_IMAGE_WIDTH*MNIST_IMAGE_HEIGHT*CHANNEL_NUMBER];
} neural_network_t;

void SegCount(int n);

void resize_280_to_28(unsigned char* out, unsigned char* in) {
    int x, y, c;
    
    for(y = 0; y < 28; y++) {
        for(x = 0; x < 28; x++) {
            for(c = 0; c < 3; c++) {
                out[y*28*3 + x*3 + c] = in[y*10*280*3 + x*10*3 + c];
            }
        }
    }
}


void RGB_to_Grayscale(uint8_t out[][MNIST_IMAGE_WIDTH], unsigned char* in) {
    int x, y, c;
    int sum = 0;
    
    for(y = 0; y < 28; y++) {
        for(x = 0; x < 28; x++) {
            sum = 0;
            for(c = 0; c < 3; c++) {
                sum += in[y*28*3 + x*3 + c];
            }
            for(c = 0; c < 3; c++) {
                in[y*28*3 + x*3 + c] = 255 - sum/3;
            }
            
        }   
    }
    
    for(y = 0; y < 28; y++) {
        for(x = 0; x < 28; x++) {
            sum = 0;
            for(c = 0; c < 3; c++) {
                sum += in[y*28*3 + x*3 + c];
            }
            if(sum/3 < 150)
                out[y][x];
            else
                out[y][x] = sum/3;
        }   
    }
}


void pixel_scale(float out[][MNIST_IMAGE_WIDTH], uint8_t in[][MNIST_IMAGE_WIDTH]) {
    int i;
    
    for (i=0;i<28*28;i++){
        out[i/28][i%28]=PIXEL_SCALE(in[i/28][i%28]);
    }
}

void zero_padding(float out[][30], float in[][28]) {
    int i, j;
    /*
    for (i = 0; i < 30; i++) {
        for (j = 0; j < 30; j++) {
            if (i == 0 || i == 29 || j == 0 || j == 29)
                out[i][j] = 0;
        }
    }
    */
    for(i = 1; i < 29; i++){
        out[0][i] = 0;
    } // (0,1) ~ (0,28)
    
    for(i = 0; i < 29; i++){
        out[i][0] = 0;
    } // (0,0) ~ (28,0)
    for(i = 0; i < 29; i++){
        out[i][29] = 0;
    } // (0,29) ~ (28,29)
    for(i = 0; i < 30; i++){
        out[29][i] = 0;
    } // (29,0) ~ (29,29)
    
    for (i = 0; i < 28; i++) {        
        for (j = 0; j < 28; j++) {
            out[i + 1][j + 1] = in[i][j];
        }
        
    }
}


void convolution(float out[][28][28], float in[][30], neural_network_t network) {
    int i, j, ch;
    
                
    for (ch = 0; ch < CHANNEL_NUMBER; ch++) {
        for (i = 0; i < 28; i++) {
            for (j = 0; j < 28; j++) {
                /*
                out[ch][i][j] = (network.conv_weight[ch][0][0] * in[i][j] + network.conv_weight[ch][0][1] * in[i][j + 1] + network.conv_weight[ch][0][2] * in[i][j + 2]
                    + network.conv_weight[ch][1][0] * in[i + 1][j] + network.conv_weight[ch][1][1] * in[i + 1][j + 1] + network.conv_weight[ch][1][2] * in[i + 1][j + 2]
                    + network.conv_weight[ch][2][0] * in[i + 2][j] + network.conv_weight[ch][2][1] * in[i + 2][j + 1] + network.conv_weight[ch][2][2] * in[i + 2][j + 2]
                    + network.conv_bias[ch]);
                */
                asm(
                    "mov r0, %[out]\n\t"
                    "mov r1, %[in]\n\t"
                    "mov r2, %[bias]\n\t"
                    "mov r3, %[weight]\n\t"
                
                    "vldr s2, [r2]\n\t" // network.conv_bias[ch]
    
                    // (network.conv_weight[ch][0][0] * in[i][j]
                    "vldr s3, [r3]\n\t" // 0,0
                    "vldr s0, [r1]\n\t"  // 0,0
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // network.conv_weight[ch][0][1] * in[i][j + 1]
                    "vldr s3, [r3, #4]\n\t" // 0,1
                    "vldr s0, [r1, #4]\n\t" // 0,1
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // network.conv_weight[ch][0][2] * in[i][j + 2]
                    "vldr s3, [r3, #8]\n\t" // 0,2
                    "vldr s0, [r1, #8]\n\t" // 0,2
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // network.conv_weight[ch][1][0] * in[i + 1][j]
                    "vldr s3, [r3, #12]\n\t" // 1,0
                    "vldr s0, [r1, #120]\n\t" // 1,0
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // network.conv_weight[ch][1][1] * in[i + 1][j + 1]
                    "vldr s3, [r3, #16]\n\t" // 1,1
                    "vldr s0, [r1, #124]\n\t" // 1,1
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t" 
                    
                    // network.conv_weight[ch][1][2] * in[i + 1][j + 2]
                    "vldr s3, [r3, #20]\n\t" // 1,2
                    "vldr s0, [r1, #128]\n\t" // 1,2
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // network.conv_weight[ch][2][0] * in[i + 2][j]
                    "vldr s3, [r3, #24]\n\t" // 2,0
                    "vldr s0, [r1, #240]\n\t" // 2,0
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // network.conv_weight[ch][2][1] * in[i + 2][j + 1]
                    "vldr s3, [r3, #28]\n\t" // 2,1
                    "vldr s0, [r1, #244]\n\t" // 2,1
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // network.conv_weight[ch][2][2] * in[i + 2][j + 2]
                    "vldr s3, [r3, #32]\n\t" // 2,2
                    "vldr s0, [r1, #248]\n\t" // 2,2
                    "vmul.f32 s3, s3, s0\n\t"
                    "vadd.f32 s2, s2, s3\n\t"
                    
                    // out[ch][i][j]
                    "vstr s2, [r0]\n\t"
                :
                :
                [in] "r"(&in[i][j]), [out] "r"(&out[ch][i][j]), [bias] "r"(&network.conv_bias[ch]), [weight] "r"(&network.conv_weight[ch][0][0])
                :  "r0",                  "r1",          "r2",           "r3",      "s0", "s1", "s2", "s3"
            //   output_address     input_address  bias_address   weight_address   in    out  bias  weight         
                );
            }
        }
    }
}

void relu(float out[], float in[][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]) {
/*
    int ch, i, j;
    
    for (ch = 0; ch < CHANNEL_NUMBER; ch++) {
        for (i = 0; i < 28; i++) {
            for (j = 0; j < 28; j++) {
                if (in[ch][i][j] < 0)
                    out[MNIST_IMAGE_SIZE * ch + MNIST_IMAGE_HEIGHT * i + j] = 0;
                else
                    out[MNIST_IMAGE_SIZE * ch + MNIST_IMAGE_HEIGHT * i + j] = in[ch][i][j];
            }
        }
    }
*/
    asm(
    
        "mov r6, #0\n\t" // Reg for address offset
        
        "mov r3, #0\n\t" // ch = 0
        "mov r8, #32\n\t" // reg_N1 = 32
        
        "Loop1:\n\t"
            "mov r4, #0\n\t" // i = 0
            "mov r9, #28\n\t" // reg_N2 = 28
                
            "Loop2:\n\t"
                "mov r5, #0\n\t" // j = 0
                "mov r10, #28\n\t" // reg_N3 = 28
                
                "Loop3:\n\t"
                    "ldr r12, [%[in],r6]\n\t" // reg_temp <= in[ch][i][j]
                    "cmp r12, #0\n\t" // temp < 0 --> ifin
                    "blt inif\n\t"
                    "b elseif\n\t"
                    
                    "inif:\n\t"
                    "mov r12, #0\n\t" // reg_temp <= in[ch][i][j]
                    "str r12, [%[out], r6]\n\t" // 0 => out[ch][i][j]	
                    "b ifend\n\t"
                    
                    "elseif:\n\t"
                    "ldr r12, [%[in],r6]\n\t" // reg_temp <= in[ch][i][j]
                    "str r12, [%[out], r6]\n\t" // reg_temp => out[ch][i][j]
                    
                    "ifend:\n\t"
                    "add r6, r6, #4\n\t" // adress += 4
                
                "add r5, r5, #1\n\t" // j++
                "cmp r5, r10\n\t" // j < N3
                "blt Loop3\n\t"
                
            "add r4, r4, #1\n\t" // i++
            "cmp r4, r9\n\t" // i < N2
            "blt Loop2\n\t"
            
        "add r3, r3, #1\n\t" // ch++
        "cmp r3, r8\n\t" // ch < N1
        "blt Loop1\n\t"
                
        :
        :
        [out] "r"(out), [in] "r"(in)
        :  "r0",  "r1", "r3", "r4",   "r5",  "r6",   "r8",  "r9", "r10", "r12"
    //   output  input   ch     i      j   address1   N1     N2     N3     temp     
    );
}

void fc(float out[], float in[], neural_network_t network) {
    /*
    int i, j;
    float sum = 0;
    
    for (i = 0; i < MNIST_LABELS; i++) {
        sum = 0.0;
        for (j = 0; j < MNIST_IMAGE_SIZE * CHANNEL_NUMBER; j++) {
            sum += network.fc_weight[i][j] * in[j];
        }
        out[i] = sum + network.fc_bias[i];
    }*/
    
    int iLEN = MNIST_LABELS;
    int jLEN = MNIST_IMAGE_SIZE * CHANNEL_NUMBER;
    asm(    
    "mov r0, %[out]\n\t" //output_address
    "mov r1, %[in]\n\t" // input_address
    "mov r2, %[bias]\n\t" // bias_address
    "mov r3, %[weight]\n\t" // weight_address
    
    "mov r4, #0\n\t" // i = 0
    "ldr r8, %[iLEN]\n\t" // N1 = 10
    
    "mov r5, #0\n\t" // j = 0
    
    "Loop4:\n\t"
    
        "vldr s2, [r2]\n\t" // sum = bias[i]
        
        "mov r5, #0\n\t" // j = 0
        "ldr r9, %[jLEN]\n\t" // N2 = 28 * 28 * 32
        "mov r1, %[in]\n\t" // input_address

        "Loop5:\n\t"    
            "vldr s0, [r1]\n\t"  // s0 <= in[j]
            "vldr s3, [r3]\n\t"  // s3 <= weight[i][j]
            "vmul.f32 s3, s3, s0\n\t" // weight * in
            "vadd.f32 s2, s2, s3\n\t" // sum += weight * in
            
            "add r1, r1, #4\n\t" // in[j++]
            "add r3, r3, #4\n\t" // weight[i][j++]
            
            "add r5, r5, #1\n\t" // j++
            "cmp r5, r9\n\t" // j < N2
            "blt Loop5\n\t"
        
        "vstr s2, [r0]\n\t"
    
        "add r2, r2, #4\n\t" // bias[i++]
        "add r0, r0, #4\n\t" // out[i++]
        
        "add r4, r4, #1\n\t"
        "cmp r4, r8\n\t"
        "blt Loop4\n\t"
        

    :
    :
     [iLEN] "m"(iLEN), [jLEN] "m"(jLEN), [in] "r"(in), [out] "r"(out), [bias] "r"(network.fc_bias), [weight] "r"(network.fc_weight)
    :
     "r0",                "r1",          "r2",           "r3",       "r4", "r5", "r8", "r9", "s0", "s2", "s3"
//   output_address     input_address  bias_address   weight_address   i    j    N1     N2    in  out  bias  weight
    );    
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

int main()
{
 
    int height;
    int width;
    int channels;
    int x,y,c,i,j;
    int sum;
    clock_t start, end;

    neural_network_t network;



    FILE *weight;
    weight=fopen("./weight.bin","rb");
    fread(&network,sizeof(neural_network_t),1,weight);


    /*          PUT YOUR CODE HERE                      */
    int fd;
    struct termios newtio;
    struct pollfd poll_handler;
    char fbuf[1024];
    char buf[256];

    fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
        printf("Make sure you are executing in sudo.\r\n");
    }
    usleep(250);

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
    while(1) {
        int img;
        int cnt = read(fd, buf, 256);
        buf[cnt] = '\0';
        if (buf[1] == '\0') {
            printf("Cheeze!\r\n");
            system("raspistill -w 280 -h 280 -t 5000 -o project.bmp");
            break;
        }
    }
    /*          Capture image(project.bmp)              */

    unsigned char* feature_in = stbi_load("example_1.bmp", &width, &height, &channels, 3);
    unsigned char* feature_resized = (unsigned char*) malloc (sizeof(unsigned char)*28*28*3);
    unsigned char feature_gray[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_scaled[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_zeroPadding[30][30];
    float feature_conv[CHANNEL_NUMBER][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_relu[MNIST_IMAGE_SIZE*CHANNEL_NUMBER];
    float activations[MNIST_LABELS];


    resize_280_to_28(feature_resized, feature_in);
    RGB_to_Grayscale(feature_gray, feature_resized);
    pixel_scale(feature_scaled, feature_gray);
    /***************    Implement these functions       ********************/
    start = clock();
    zero_padding(feature_zeroPadding, feature_scaled);
    convolution(feature_conv, feature_zeroPadding, network);
    relu(feature_relu, feature_conv);
    fc(activations, feature_relu, network);
    printf("Excution time_simple : %.3lf ms\n", (double)clock()-start);
    /***********************************************************************/
    softmax(activations, MNIST_LABELS);


    printf("\n");
    printf("softmax value\n");
    for (i=0;i<10;i++){
    printf("%d : %f\n",i,activations[i]);
    }


    /*          PUT YOUR CODE HERE                      */
    if (wiringPiSetup() == -1)
		return 1;
	pinMode(segA, OUTPUT);
	pinMode(segB, OUTPUT);
	pinMode(segC, OUTPUT);
	pinMode(segD, OUTPUT);
	pinMode(segE, OUTPUT);
	pinMode(segF, OUTPUT);
	pinMode(segG, OUTPUT);
    
    float seg;
    int seg_no;
    seg = activations[0];
    for (i = 0; i < 10; i++) {
        if (seg < activations[i]) {
            seg = activations[i];
            seg_no = i;
        }
    }
    SegCount(seg_no);
    /*          7-segment                               */
    

    fclose(weight);
    stbi_image_free(feature_in);
    free(feature_resized);
    return 0;
}

void SegCount(int n){
	switch(n){
		case 0:
			digitalWrite(segA,1);
			digitalWrite(segB,1);
			digitalWrite(segC,1);
			digitalWrite(segD,1);
			digitalWrite(segE,1);
			digitalWrite(segF,1);
			digitalWrite(segG,0);
			break;
		case 1:
			digitalWrite(segA,0);
			digitalWrite(segB,1);
			digitalWrite(segC,1);
			digitalWrite(segD,0);
			digitalWrite(segE,0);
			digitalWrite(segF,0);
			digitalWrite(segG,0);		
			break;
		case 2:
			digitalWrite(segA,1);
			digitalWrite(segB,1);
			digitalWrite(segC,0);
			digitalWrite(segD,1);
			digitalWrite(segE,1);
			digitalWrite(segF,0);
			digitalWrite(segG,1);		
			break;
		case 3:
			digitalWrite(segA,1);
			digitalWrite(segB,1);
			digitalWrite(segC,1);
			digitalWrite(segD,1);
			digitalWrite(segE,0);
			digitalWrite(segF,0);
			digitalWrite(segG,1);		
			break;
		case 4:
			digitalWrite(segA,0);
			digitalWrite(segB,1);
			digitalWrite(segC,1);
			digitalWrite(segD,0);
			digitalWrite(segE,0);
			digitalWrite(segF,1);
			digitalWrite(segG,1);		
			break;		
		case 5:
			digitalWrite(segA,1);
			digitalWrite(segB,0);
			digitalWrite(segC,1);
			digitalWrite(segD,1);
			digitalWrite(segE,0);
			digitalWrite(segF,1);
			digitalWrite(segG,1);		
			break;		
		case 6:
			digitalWrite(segA,1);
			digitalWrite(segB,0);
			digitalWrite(segC,1);
			digitalWrite(segD,1);
			digitalWrite(segE,1);
			digitalWrite(segF,1);
			digitalWrite(segG,1);		
			break;		
		case 7:
			digitalWrite(segA,1);
			digitalWrite(segB,1);
			digitalWrite(segC,1);
			digitalWrite(segD,0);
			digitalWrite(segE,0);
			digitalWrite(segF,1);
			digitalWrite(segG,0);		
			break;		
		case 8:
			digitalWrite(segA,1);
			digitalWrite(segB,1);
			digitalWrite(segC,1);
			digitalWrite(segD,1);
			digitalWrite(segE,1);
			digitalWrite(segF,1);
			digitalWrite(segG,1);		
			break;		
		case 9:
			digitalWrite(segA,1);
			digitalWrite(segB,1);
			digitalWrite(segC,1);
			digitalWrite(segD,1);
			digitalWrite(segE,0);
			digitalWrite(segF,1);
			digitalWrite(segG,1);		
			break;		
	}	
}

