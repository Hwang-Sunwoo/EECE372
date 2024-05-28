#include <stdio.h> 
#include <stdlib.h> 
#include <arm_neon.h>
#include <time.h>

#include <string.h>           // needed for memset()
#include <math.h>             // needed for floating point routines
#include <sys/time.h>         // needed for gettimeofday()
#include <unistd.h>           // needed for pid_t type
#include <sys/ioctl.h>        // needed for ioctl()
#include <asm/unistd.h>       // needed for perf_event syscall
#include <linux/perf_event.h> // needed for perf_event
#include <omp.h>

#include <sys/time.h>
#include "arm_perf.h"

void func() {

	clock_t np0, np1, p0, p1;

	int16_t *arr1 = malloc(sizeof(int16_t) * 16);
	int16_t *arr2 = malloc(sizeof(int16_t) * 16);
	int16_t *ans_neon = malloc(sizeof(int16_t) * 16);
  	int16_t *ans_for = malloc(sizeof(int16_t) * 16);

    time_t t;

    srand((unsigned) time(&t));

	for (int i = 0; i < 16; i++){
		arr1[i] = rand()%15;
		arr2[i] = rand()%15;

	}
	///////////////////////  Matrix multiplication with for loop (start)  /////////////////
	np0 = clock();

	for (int i = 0; i < 4; i ++) {
		for (int j = 0; j < 4; j++) {
			ans_for[i*4 + j] += arr1[4*i]*arr2[j] + arr1[4*i+1]*arr2[j+4] + arr1[4*i+2]*arr2[j+8] + arr1[4*i+3]*arr2[j+12];
		}
	}

	np1 = clock();
	///////////////////////  Matrix multiplication with for loop (end)  /////////////////
   

	//////////////////  Change to your own code (start)  //////////////////////
   	int16x4_t arr1_0=vld1_s16(arr1);	// 1st row of arr1
   	int16x4_t arr1_1=vld1_s16(arr1+4);	// 2nd row of arr1
   	int16x4_t arr1_2=vld1_s16(arr1+8);	// 3rd row of arr1
   	int16x4_t arr1_3=vld1_s16(arr1+12);	// 4th row of arr1

	int16x4x4_t mt;
	mt=vld4_s16(arr2);//Set arr2 to fit the format

   	int16x4_t arr2_0=vmov_n_s16(0);	// 1st col of arr2 reset
   	int16x4_t arr2_1=vmov_n_s16(0);	// 2nd col of arr2 reset
   	int16x4_t arr2_2=vmov_n_s16(0);	// 3rd col of arr2 reset
   	int16x4_t arr2_3=vmov_n_s16(0);	// 4th col of arr2 reset

	arr2_0=mt.val[0];//1st col of arr2
	arr2_1=mt.val[1];//2nd col of arr2
	arr2_2=mt.val[2];// 3rd col of arr2
	arr2_3=mt.val[3];// 4th col of arr2

    	int16x4_t ans_0 =vmov_n_s16(0); //Randomly used variable for row 1 operation: reset to 0
    	int16x4_t ans_1 =vmov_n_s16(0); //Randomly used variable for row 2 operation: reset to 0
    	int16x4_t ans_2 =vmov_n_s16(0); //Randomly used variable for row 3 operation: reset to 0
    	int16x4_t ans_3 =vmov_n_s16(0); //Randomly used variable for row 4 operation: reset to 0
	int sum=0;
	////////  Matrix multiplication with NEON (start)  /////////
	p0 = clock(); // Operation starts after this line

	ans_0=vmul_s16(arr1_0,arr2_0); //Dot product of 1th row of arr1, 1th column of arr2
	ans_neon[0]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3];
	ans_0=vmul_s16(arr1_0,arr2_1); //Dot product of 1th row of arr1, 2th column of arr2
	ans_neon[1]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3];
	ans_0=vmul_s16(arr1_0,arr2_2); //Dot product of 1th row of arr1, 3th column of arr2
	ans_neon[2]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3];
	ans_0=vmul_s16(arr1_0,arr2_3); //Dot product of 1th row of arr1, 4th column of arr2
	ans_neon[3]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3];

	
	ans_1=vmul_s16(arr1_1,arr2_0); //Dot product of 2th row of arr1, 1th column of arr2
	ans_neon[4]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3];
	ans_1=vmul_s16(arr1_1,arr2_1); //Dot product of 2th row of arr1, 2th column of arr2
	ans_neon[5]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3];
	ans_1=vmul_s16(arr1_1,arr2_2); //Dot product of 2th row of arr1, 3th column of arr2
	ans_neon[6]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3];
	ans_1=vmul_s16(arr1_1,arr2_3); //Dot product of 2th row of arr1, 4th column of arr2
	ans_neon[7]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3];
	
	ans_2=vmul_s16(arr1_2,arr2_0); //Dot product of 3th row of arr1, 1th column of arr2
	ans_neon[8]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3];
	ans_2=vmul_s16(arr1_2,arr2_1); //Dot product of 3th row of arr1, 2th column of arr2
	ans_neon[9]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3];
	ans_2=vmul_s16(arr1_2,arr2_2); //Dot product of 3th row of arr1, 3th column of arr2
	ans_neon[10]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3];
	ans_2=vmul_s16(arr1_2,arr2_3); //Dot product of 3th row of arr1, 4th column of arr2
	ans_neon[11]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3];

	ans_3=vmul_s16(arr1_3,arr2_0); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[12]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3];
	ans_3=vmul_s16(arr1_3,arr2_1); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[13]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3];
	ans_3=vmul_s16(arr1_3,arr2_2); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[14]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3];
	ans_3=vmul_s16(arr1_3,arr2_3); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[15]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3];
	p1 = clock();
	/////////  Matrix multiplication with NEON (end)  /////////
	
	int check = 0;

	for (int i = 0; i < 16;i++) {
		if (ans_neon[i] != ans_for[i]) {
			check += 1;
		}
	}

	if (check == 0) {
		printf("PASS\n");
    }
	else {
		printf("FAIL\n");
	}
 
	printf("Execution time (for loop): %lf ms\n", (double)np1-np0);
	printf("Execution time (neon)    : %lf ms\n", (double)p1-p0);

	free(arr1);
	free(arr2);
  	free(ans_for);
	free(ans_neon);
}

int main(int argc, char* argv[]) {

	func();

	return 0;
}

