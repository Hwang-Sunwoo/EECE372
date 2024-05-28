/* student ID : 20210404 */

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
    int16x4_t arr1_0;	// 1st row of arr1
    int16x4_t arr1_1;	// 2nd row of arr1
    int16x4_t arr1_2;	// 3rd row of arr1
    int16x4_t arr1_3;	// 4th row of arr1

    int16x4_t arr2_0;	// 1st row of arr2
    int16x4_t arr2_1;	// 2nd row of arr2
    int16x4_t arr2_2;	// 3rd row of arr2
    int16x4_t arr2_3;	// 4th row of arr2

	// Initialize to zero
    int16x4_t ans_0 = vdup_n_s16(0); // 1st row of (arr1 * arr2)
	int16x4_t ans_1 = vdup_n_s16(0); // 2nd row of (arr1 * arr2)
    int16x4_t ans_2 = vdup_n_s16(0); // 3rd row of (arr1 * arr2)
    int16x4_t ans_3 = vdup_n_s16(0); // 4th row of (arr1 * arr2)

	arr1_0 = vld1_s16(arr1);		// load 1st row of arr1 values to arr1_0 register
	arr1_1 = vld1_s16(arr1 + 4);	// load 2st row of arr1 values to arr1_1 register
	arr1_2 = vld1_s16(arr1 + 8);	// load 3st row of arr1 values to arr1_2 register
	arr1_3 = vld1_s16(arr1 + 12);	// load 4st row of arr1 values to arr1_3 register

	arr2_0 = vld1_s16(arr2);		// load 1st row of arr2 values to arr2_0 register
	arr2_1 = vld1_s16(arr2 + 4);	// load 2st row of arr2 values to arr2_1 register
	arr2_2 = vld1_s16(arr2 + 8);	// load 3st row of arr2 values to arr2_2 register
	arr2_3 = vld1_s16(arr2 + 12);	// load 4st row of arr2 values to arr2_3 register
	
	int16x4_t temp;

	////////  Matrix multiplication with NEON (start)  /////////
	p0 = clock(); // Operation starts after this line
	
	// ans_0 = arr1_0[0] * arr2_0 + arr1_0[1] * arr2_1 + arr1_0[2] * arr2_2 + arr1_0[3] * arr2_3
	temp = vmul_n_s16(arr2_0, vget_lane_s16(arr1_0, 0));	// arr1_0[0] * arr2_0
	ans_0 = vadd_s16(ans_0, temp);							// ans_0 = arr1_0[0] * arr2_0
	temp = vmul_n_s16(arr2_1, vget_lane_s16(arr1_0, 1));	// arr1_0[1] * arr2_1 + arr1_0[2]
	ans_0 = vadd_s16(ans_0, temp);							// ans_0 = arr1_0[0] * arr2_0 + arr1_0[1] * arr2_1
	temp = vmul_n_s16(arr2_2, vget_lane_s16(arr1_0, 2));	// arr1_0[2] * arr2_2
	ans_0 = vadd_s16(ans_0, temp);							// ans_0 = arr1_0[0] * arr2_0 + arr1_0[1] * arr2_1 + arr1_0[2] * arr2_2
	temp = vmul_n_s16(arr2_3, vget_lane_s16(arr1_0, 3));	// arr1_0[3] * arr2_3
	ans_0 = vadd_s16(ans_0, temp);							// ans_0 = arr1_0[0] * arr2_0 + arr1_0[1] * arr2_1 + arr1_0[2] * arr2_2 + arr1_0[3] * arr2_3
	
	// ans_1 = arr1_1[0] * arr2_0 + arr1_1[1] * arr2_1 + arr1_1[2] * arr2_2 + arr1_1[3] * arr2_3
	temp = vmul_n_s16(arr2_0, vget_lane_s16(arr1_1, 0));	// arr1_1[0] * arr2_0
	ans_1 = vadd_s16(ans_1, temp);							// ans_1 = arr1_1[0] * arr2_0
	temp = vmul_n_s16(arr2_1, vget_lane_s16(arr1_1, 1));	// arr1_1[1] * arr2_1
	ans_1 = vadd_s16(ans_1, temp);							// ans_1 = arr1_1[0] * arr2_0 + arr1_1[1] * arr2_1 
	temp = vmul_n_s16(arr2_2, vget_lane_s16(arr1_1, 2));	// arr1_1[2] * arr2_2
	ans_1 = vadd_s16(ans_1, temp);							// ans_1 = arr1_1[0] * arr2_0 + arr1_1[1] * arr2_1 + arr1_1[2] * arr2_2
	temp = vmul_n_s16(arr2_3, vget_lane_s16(arr1_1, 3));	// arr1_1[3] * arr2_3
	ans_1 = vadd_s16(ans_1, temp);							// ans_1 = arr1_1[0] * arr2_0 + arr1_1[1] * arr2_1 + arr1_1[2] * arr2_2 + arr1_1[3] * arr2_3
	
	// ans_2 = arr1_2[0] * arr2_0 + arr1_2[1] * arr2_1 + arr1_2[2] * arr2_2 + arr1_2[3] * arr2_3
	temp = vmul_n_s16(arr2_0, vget_lane_s16(arr1_2, 0));
	ans_2 = vadd_s16(ans_2, temp);
	temp = vmul_n_s16(arr2_1, vget_lane_s16(arr1_2, 1));
	ans_2 = vadd_s16(ans_2, temp);
	temp = vmul_n_s16(arr2_2, vget_lane_s16(arr1_2, 2));
	ans_2 = vadd_s16(ans_2, temp);
	temp = vmul_n_s16(arr2_3, vget_lane_s16(arr1_2, 3));
	ans_2 = vadd_s16(ans_2, temp);

	// ans_3 = arr1_3[0] * arr2_0 + arr1_3[1] * arr2_1 + arr1_3[2] * arr2_2 + arr1_3[3] * arr2_3
	temp = vmul_n_s16(arr2_0, vget_lane_s16(arr1_3, 0));
	ans_3 = vadd_s16(ans_3, temp);
	temp = vmul_n_s16(arr2_1, vget_lane_s16(arr1_3, 1));
	ans_3 = vadd_s16(ans_3, temp);
	temp = vmul_n_s16(arr2_2, vget_lane_s16(arr1_3, 2));
	ans_3 = vadd_s16(ans_3, temp);
	temp = vmul_n_s16(arr2_3, vget_lane_s16(arr1_3, 3));
	ans_3 = vadd_s16(ans_3, temp);
	
	// Store values from ans_0,1,2,3 to ans_neon
	vst1_s16(ans_neon, ans_0);
	vst1_s16(ans_neon + 4, ans_1);
	vst1_s16(ans_neon + 8, ans_2);
	vst1_s16(ans_neon + 12, ans_3);

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

