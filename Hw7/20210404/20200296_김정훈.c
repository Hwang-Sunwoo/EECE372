#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <omp.h>
#include <arm_neon.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <wiringPi.h>

#define pin_a 1
#define MNIST_IMAGE_WIDTH 28
#define MNIST_IMAGE_HEIGHT 28
#define MNIST_IMAGE_SIZE MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT
#define MNIST_LABELS 10
#define CHANNEL_NUMBER 32
#define CHANNEL_WIDTH 3
#define CHANNEL_HEIGHT 3
#define PIXEL_SCALE(x) (((float) (x)) / 255.0f)
#define BAUDRATE B1000000

typedef struct neural_network_t_ {
    float conv_bias[CHANNEL_NUMBER];
    float conv_weight[CHANNEL_NUMBER][CHANNEL_HEIGHT][CHANNEL_WIDTH];
    float fc_bias[MNIST_LABELS];
    float fc_weight[MNIST_LABELS][MNIST_IMAGE_WIDTH*MNIST_IMAGE_HEIGHT*CHANNEL_NUMBER];
} neural_network_t;


void set_seg(int pin, int a, int b, int c, int d, int e, int f, int g){
	digitalWrite(pin,a); //make pin to a
	digitalWrite(pin+1,b); //pin+1 is pin b
	digitalWrite(pin+2,c);
	digitalWrite(pin+3,d);
	digitalWrite(pin+4,e);
	digitalWrite(pin+5,f);
	digitalWrite(pin+6,g);
}
void num_seg(int num, int pin){
	if(num==0) set_seg(pin,1,1,1,1,1,1,0);
	else if(num==1) set_seg(pin,0,1,1,0,0,0,0);
	else if(num==2) set_seg(pin,1,1,0,1,1,0,1);
	else if(num==3) set_seg(pin,1,1,1,1,0,0,1);
	else if(num==4) set_seg(pin,0,1,1,0,0,1,1);
	else if(num==5) set_seg(pin,1,0,1,1,0,1,1);
	else if(num==6) set_seg(pin,1,0,1,1,1,1,1);
	else if(num==7) set_seg(pin,1,1,1,0,0,1,0);
	else if(num==8) set_seg(pin,1,1,1,1,1,1,1);
	else if(num==9) set_seg(pin,1,1,1,1,0,1,1);
}

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
	asm(
		"mov r4, #0\n\t" // adrress of out
		"mov r5, #0\n\t" // address of in
		"mov r10,#0\n\t" //value
		
		"FOR1:\n\t" //1st line loop
		"mov r10,#0\n\t"
		"str r10,[%[out],r4]\n\t"
		"add r4,r4,#4\n\t"
		"cmp r4,#0x78\n\t" //r4=30 
		"bne FOR1\n\t"
		
		"FOR2:\n\t" //2~29 line loop
		"str r10,[%[out],r4]\n\t"
		"add r4,r4,#4\n\t"
		"mov r8,#0\n\t" //check for 28
		
		"FOR3:\n\t"
		"ldr r10,[%[in],r5]\n\t"
		"str r10,[%[out],r4]\n\t" //out[i,j]=in[i-1,j-1]
		
		"add r5,r5,#4\n\t" //in address ++
		"add r8,r8,#1\n\t" //j++
		"add r4,r4,#4\n\t" //out address ++

		"cmp r8,#0x1c\n\t" //28?
		"bne FOR3\n\t"
		
		
		"mov r10,#0\n\t"
		"str r10,[%[out],r4]\n\t" // j=29 out=0
		"add r4,r4,#4\n\t"
		//"mov r10,#0xc40\n\t" //28*28*4
		//"cmp r5,r10\n\t" // when in address full used
		"cmp r5,#0xc40\n\t"
		"bne FOR2\n\t"

		
		"FOR4:\n\t" //last line loop
		"mov r10,#0\n\t"
		"str r10,[%[out],r4]\n\t"
		"add r4,r4,#4\n\t"
		//"mov r10,#0xe10\n\t"
		//"cmp r4,r10\n\t" //r4=30*30*4
		"cmp r4,#0xe10\n\t"
		"bne FOR4\n\t"
		
		
		:
		:
			[in] "r"(in[0]), [out] "r"(out[0])
		:
			"r4","r5","r8","r10"
	);
    //       PUT YOUR CODE HERE          
}

void convolution(float out[][28][28], float in[][30], neural_network_t network) {	
	float32x4_t filter_0,filter_1,filter_2,in_0, in_1, in_2,out_0,out_1,out_2;//multi data set
	int row=0;
	for(int z=0;z<CHANNEL_NUMBER;z++){
		filter_0=vld1q_f32(**(network.conv_weight+z)); //1st line filter packing
		filter_1=vld1q_f32(**(network.conv_weight+z)+3);//2nd line filter packing
		filter_2=vld1q_f32(**(network.conv_weight+z)+6); // last line filter packing
		#pragma omp parallel
		{
			#pragma omp for private(in_0, in_1, in_2,out_0,out_1,out_2,row)
			for(int i=0;i<900;i++) //in size
			{	
				row=i/30; // 2* row num, 2 index isn't need in each row
				if(i%30<28) // last 2 index does not muliply
				{
					in_0=vld1q_f32(*(in)+i); // 1st line of in[i]~in[i+2]
					in_1=vld1q_f32(*(in+1)+i); // 2nd line of in
					in_2=vld1q_f32(*(in+2)+i); // 3rd line of in
	
					out_0=vmulq_f32(filter_0,in_0);//filter*in = out--- 1st line
					out_1=vmulq_f32(filter_1,in_1);
					out_2=vmulq_f32(filter_2,in_2);

					*(**(out+z)+i-2*row)=out_0[0]+out_0[1]+out_0[2]+out_1[0]+out_1[1]+out_1[2]+out_2[0]+out_2[1]+out_2[2]+network.conv_bias[z];
				}
			}
		}
		
	}
    //          PUT YOUR CODE HERE          
}

void relu(float out[], float in[][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]) {
	asm(
		"mov r4, #0\n\t" // address
		"mov r10, #0\n\t" //temp
		
		"FORz:\n\t" //z loop
		"ldr r10,[%[in],r4]\n\t" //in[z,y,x] = r10
		"cmp r10,#0\n\t" // in >0 ?
		"ble xIF\n\t"
		
		"str r10,[%[out],r4]\n\t" //out[z,y,x] = in [z,y,x]
		"b finFORz\n\t"
				
		"xIF:\n\t" 
		"mov r10,#0\n\t"
		"str r10,[%[out],r4]\n\t" //out[z,y,x]=0
	
		"finFORz:\n\t"
		"add r4,r4,#4\n\t" //address ++
		"cmp r4,#0x18800\n\t" // 32*28*28*4
		"bne FORz\n\t"
		

		:
		:
			[in] "r"(in[0][0]), [out] "r"(out)
		:
			"r4","r10"
	);
    //          PUT YOUR CODE HERE          
}

void fc(float out[], float in[], neural_network_t network) {

	float32x4_t filter_arr,in_arr,out_arr;
	for(int L;L<MNIST_LABELS;L++){
		#pragma omp parallel
		{
			#pragma omp for reduction(+:out[L]) private(filter_arr,in_arr,out_arr)
			for(int i=0;i<MNIST_IMAGE_WIDTH*MNIST_IMAGE_HEIGHT*CHANNEL_NUMBER;i+=4){
				in_arr=vld1q_f32(in+i);
				filter_arr=vld1q_f32( *(network.fc_weight+L)+i);
				out_arr=vmulq_f32(in_arr,filter_arr);
				out[L]+=out_arr[0]+out_arr[1]+out_arr[2];
			}
		
			out[L]+=network.fc_bias[L];
		}
	}
    //         PUT YOUR CODE HERE          
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
void azero_padding(float out[][30], float in[][28]) {
	for(int i=0;i<30;i++){
		for(int j=0;j<30;j++){
			if(i==0||i==29||j==0||j==29){
				out[i][j]=0;
			}
			else{
				out[i][j]=in[i-1][j-1];
			}
		}
	}
    /*          PUT YOUR CODE HERE          */
}


void aconvolution(float out[][28][28], float in[][30], neural_network_t network) {
	float temp=0;
	for(int z=0;z<CHANNEL_NUMBER;z++){
		for(int y=0;y<MNIST_IMAGE_HEIGHT;y++){
			for(int x=0;x<MNIST_IMAGE_WIDTH;x++){
				
				for(int i=0;i<CHANNEL_HEIGHT;i++){
					for(int j=0;j<CHANNEL_WIDTH;j++){
						temp+=in[y+i][x+j]*network.conv_weight[z][i][j];
					}
				}
				out[z][y][x]=temp+network.conv_bias[z];
				temp=0;
			}
			
		}
	}
    /*          PUT YOUR CODE HERE          */
}

void arelu(float out[], float in[][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]) {
	for(int z=0;z<CHANNEL_NUMBER;z++){
		for(int y=0;y<MNIST_IMAGE_HEIGHT;y++){
			for(int x=0;x<MNIST_IMAGE_WIDTH;x++){
				if(in[z][y][x]>0) out[28*28*z+28*y+x]=in[z][y][x];
				else out[28*28*z+28*y+x]=0;
			}
		}
	}
    /*          PUT YOUR CODE HERE          */
}

void afc(float out[], float in[], neural_network_t network) {
	float temp=0;
	for(int L;L<MNIST_LABELS;L++){
		for(int i=0;i<MNIST_IMAGE_WIDTH*MNIST_IMAGE_HEIGHT*CHANNEL_NUMBER;i++){
			temp+=in[i]*network.fc_weight[L][i];
		}
		out[L]=temp+network.fc_bias[L];
		temp=0;
	}
    /*          PUT YOUR CODE HERE          */
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

	char buf[256];

	fd = open("/dev/ttyAMA0", O_RDWR|O_NOCTTY);
	if(fd<0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;
	
//	speed_t baudRate = B1000000;
//	cfsetispeed(&newtio, baudRate);
//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);


    int cnt = read(fd, buf, sizeof(buf));
    buf[cnt] = '\0';
    printf("Received: %s\r\n", buf);
    
	if(cnt>0){
	    printf("take a photo\n");
	    system("raspistill -w 280 -h 280 -o project.bmp");
	}

		// Insert your code
    /*          Capture image(project.bmp)              */

    unsigned char* feature_in = stbi_load("project.bmp", &width, &height, &channels, 3);
    unsigned char* feature_resized = (unsigned char*) malloc (sizeof(unsigned char)*28*28*3);
    unsigned char feature_gray[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_scaled[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_zeroPadding[30][30];
    float feature_conv[CHANNEL_NUMBER][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_relu[MNIST_IMAGE_SIZE*CHANNEL_NUMBER];
    float activations[MNIST_LABELS];

    unsigned char* afeature_in = stbi_load("example_1.bmp", &width, &height, &channels, 3);
    unsigned char* afeature_resized = (unsigned char*) malloc (sizeof(unsigned char)*28*28*3);
    unsigned char afeature_gray[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float afeature_scaled[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float afeature_zeroPadding[30][30];
    float afeature_conv[CHANNEL_NUMBER][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float afeature_relu[MNIST_IMAGE_SIZE*CHANNEL_NUMBER];
    float aactivations[MNIST_LABELS];


    resize_280_to_28(afeature_resized, afeature_in);
    RGB_to_Grayscale(afeature_gray, afeature_resized);
    pixel_scale(afeature_scaled, afeature_gray);
    /***************    Implement these functions       ********************/
    start = clock();
    azero_padding(afeature_zeroPadding, afeature_scaled);
    aconvolution(afeature_conv, afeature_zeroPadding, network);
    arelu(afeature_relu, afeature_conv);
    afc(aactivations, afeature_relu, network);
    printf("Excution time_simple : %.3lf ms\n", (double)clock()-start);

    /***********************************************************************/
    softmax(aactivations, MNIST_LABELS);
    
	printf("softmax value_simple\n");
    for (i=0;i<10;i++){
    printf("%d : %f\n",i,aactivations[i]);
    }

    resize_280_to_28(feature_resized, feature_in);
    RGB_to_Grayscale(feature_gray, feature_resized);
    pixel_scale(feature_scaled, feature_gray);
    /***************    Implement these functions       ********************/
    start = clock();
    zero_padding(feature_zeroPadding, feature_scaled);
    convolution(feature_conv, feature_zeroPadding, network);
    relu(feature_relu, feature_conv);
    fc(activations, feature_relu, network);
    printf("Excution time_simple_acc : %.3lf ms\n", (double)clock()-start);
    /***********************************************************************/
    softmax(activations, MNIST_LABELS);


    printf("\n");
    printf("softmax value_acc\n");
    for (i=0;i<10;i++){
    printf("%d : %f\n",i,activations[i]);
    }

    /*          PUT YOUR CODE HERE                      */
    int sys=0; //switch push -> sys=0, non push -> 1
	int num=0; //number that switch has pushed.
	if(wiringPiSetup()==-1) //library include failed
		return 1;
	pinMode(pin_a,OUTPUT); //pin a to g is output
	pinMode(pin_a+1,OUTPUT);
	pinMode(pin_a+2,OUTPUT);
	pinMode(pin_a+3,OUTPUT);
	pinMode(pin_a+4,OUTPUT);
	pinMode(pin_a+5,OUTPUT);
	pinMode(pin_a+6,OUTPUT);
	
	num_seg(0,pin_a); //initialize 7 segment to 0
	int ans=0;
	for(i=1;i<10;i++){
		if(activations[i]>activations[ans]) ans=i;
	}
	num_seg(ans,pin_a);
    /*          7-segment                               */
    

    fclose(weight);
    stbi_image_free(feature_in);
    free(feature_resized);
    return 0;
}

