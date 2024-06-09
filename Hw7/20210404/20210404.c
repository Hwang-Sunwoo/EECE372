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


void zero_padding(float out[][30], float in[][28]) {
    /*          PUT YOUR CODE HERE          */
    int i, j;

    for (i = 0; i < 30; i++) {
        for (j = 0; j < 30; j++) {
            if ((i == 0) || (i == 29) || (j == 0) || (j == 29))
                out[i][j] = 0.0;
            else
                out[i][j] = in[i - 1][j - 1];
        }
    }
}


void convolution(float out[][28][28], float in[][30], neural_network_t network) {
    /*          PUT YOUR CODE HERE          */
    int i, j, k, m, channel;
    float sum = 0.0;

    for (channel = 0; channel < 32; channel++) {
        for (k = 0; k < 28; k++) {
            for (m = 0; m < 28; m++) {
                for (i = k; i - k < 3; i++) {
                    for (j = m; j - m < 3; j++) {
                        sum = sum + in[i][j] * network.conv_weight[channel][i - k][j - m];
                    }
                }
                out[channel][k][m] = sum + network.conv_bias[channel];
                sum = 0.0;
            }
        }
    }
}


void relu(float out[], float in[][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]) {
    /*          PUT YOUR CODE HERE          */
    int i, j, channel;

    for (channel = 0; channel < 32; channel++) {
        for (i = 0; i < 28; i++) {
            for (j = 0; j < 28; j++) {
                if (in[channel][i][j] > 0)
                    out[channel * MNIST_IMAGE_SIZE + i * 28 + j] = in[channel][i][j];
                else
                    out[channel * MNIST_IMAGE_SIZE + j * 28 + j] = 0;
            }
        }
    }
}


void fc(float out[], float in[], neural_network_t network) {
    /*          PUT YOUR CODE HERE          */
    int i, j;
    float sum = 0.0;

    for (i = 0; i < MNIST_LABELS; i++) {
        for (j = 0; j < MNIST_IMAGE_SIZE * CHANNEL_NUMBER; j++) {
            sum = sum + (in[j] * network.fc_weight[i][j]);
        }
        out[i] = sum + network.fc_bias[i];
        sum = 0.0;
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



int main(int argc, char* argv[])
{
    int height;
    int width;
    int channels;
    int x, y, c, i, j;
    int sum;
    clock_t start, end;

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
    printf("Execution time   (basic): %.3lf ms\n", (double)clock()-start);
    /***********************************************************************/

    softmax(activations, MNIST_LABELS);

    printf("\n");
    printf("softmax value (basic)\n");
    for (i = 0; i < 10; i++) {
        printf("%d : %f\n", i, activations[i]);
    }

    int result = 0;
    for (i = 0; i < 9; i++)
        if (activations[result] < activations[i + 1])
            result = i + 1;
    printf("result (basic): %d\n", result);


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


    display_Digit(result);
    /*          7-segment                               */
    

    fclose(weight);
    stbi_image_free(feature_in);
    free(feature_resized);

    return 0;
}

