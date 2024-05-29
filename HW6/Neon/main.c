#include <arm_neon.h>
#include <asm/unistd.h>        // needed for perf_event syscall
#include <linux/perf_event.h>  // needed for perf_event
#include <math.h>              // needed for floating point routines
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // needed for memset()
#include <sys/ioctl.h>  // needed for ioctl()
#include <sys/time.h>   // needed for gettimeofday()
#include <sys/time.h>
#include <time.h>
#include <unistd.h>  // needed for pid_t type

#include "arm_perf.h"

void func() {
    clock_t np0, np1, p0, p1;

    int16_t *arr1 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *arr2 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *ans_neon = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *ans_for = malloc(sizeof(int16_t) * 8 * 8);

    srand((unsigned)time(NULL));
    for (int i = 0; i < 8 * 8; i++) {
        arr1[i] = rand() % 15;
        arr2[i] = rand() % 15;
    }

    ///////////////////////  Matrix multiplication with for loop start /////////////////
    np0 = clock();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                ans_for[8 * i + j] += arr1[8 * i + k] * arr2[8 * k + j];
            }
        }
    }

    np1 = clock();
    ///////////////////////  Matrix multiplication with for loop end  /////////////////

    ///////// Matrix multiplication with NEON start/////////
    int16x4_t arr1_0=vld1_s16(arr1);	// 1st row of arr1
   	int16x4_t arr1_1=vld1_s16(arr1+8);	// 2nd row of arr1
   	int16x4_t arr1_2=vld1_s16(arr1+16);	// 3rd row of arr1
   	int16x4_t arr1_3=vld1_s16(arr1+24);	// 4th row of arr1
    int16x4_t arr1_4=vld1_s16(arr1+32);	// 5th row of arr1
    int16x4_t arr1_5=vld1_s16(arr1+40);	// 6th row of arr1
    int16x4_t arr1_6=vld1_s16(arr1+48);	// 7th row of arr1
    int16x4_t arr1_7=vld1_s16(arr1+56);	// 8th row of arr1

	int16x4x4_t mt;
	mt=vld4_s16(arr2);//Set arr2 to fit the format

   	int16x4_t arr2_0=vmov_n_s16(0);	// 1st col of arr2 reset
   	int16x4_t arr2_1=vmov_n_s16(0);	// 2nd col of arr2 reset
   	int16x4_t arr2_2=vmov_n_s16(0);	// 3rd col of arr2 reset
   	int16x4_t arr2_3=vmov_n_s16(0);	// 4th col of arr2 reset
    int16x4_t arr2_4=vmov_n_s16(0);	// 5th col of arr2 reset
   	int16x4_t arr2_5=vmov_n_s16(0);	// 6th col of arr2 reset
   	int16x4_t arr2_6=vmov_n_s16(0);	// 7th col of arr2 reset
   	int16x4_t arr2_7=vmov_n_s16(0);	// 8th col of arr2 reset

	arr2_0=mt.val[0];//1st col of arr2
	arr2_1=mt.val[1];//2nd col of arr2
	arr2_2=mt.val[2];// 3rd col of arr2
	arr2_3=mt.val[3];// 4th col of arr2
    arr2_4=mt.val[4];// 5th col of arr2
	arr2_5=mt.val[5];// 6th col of arr2
	arr2_6=mt.val[6];// 7th col of arr2
	arr2_7=mt.val[7];// 8th col of arr2

    int16x4_t ans_0 =vmov_n_s16(0); //Randomly used variable for row 1 operation: reset to 0
	int16x4_t ans_1 =vmov_n_s16(0); //Randomly used variable for row 2 operation: reset to 0
   	int16x4_t ans_2 =vmov_n_s16(0); //Randomly used variable for row 3 operation: reset to 0
    int16x4_t ans_3 =vmov_n_s16(0); //Randomly used variable for row 4 operation: reset to 0
    int16x4_t ans_4 =vmov_n_s16(0); //Randomly used variable for row 5 operation: reset to 0
	int16x4_t ans_5 =vmov_n_s16(0); //Randomly used variable for row 6 operation: reset to 0
   	int16x4_t ans_6 =vmov_n_s16(0); //Randomly used variable for row 7 operation: reset to 0
    int16x4_t ans_7 =vmov_n_s16(0); //Randomly used variable for row 8 operation: reset to 0
	int sum=0;

    
	p0 = clock(); // Operation starts after this line

	ans_0=vmul_s16(arr1_0,arr2_0); //Dot product of 1th row of arr1, 1th column of arr2
	ans_neon[0]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
	ans_0=vmul_s16(arr1_0,arr2_1); //Dot product of 1th row of arr1, 2th column of arr2
	ans_neon[1]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
	ans_0=vmul_s16(arr1_0,arr2_2); //Dot product of 1th row of arr1, 3th column of arr2
	ans_neon[2]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
	ans_0=vmul_s16(arr1_0,arr2_3); //Dot product of 1th row of arr1, 4th column of arr2
	ans_neon[3]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
    ans_0=vmul_s16(arr1_0,arr2_4); //Dot product of 1th row of arr1, 1th column of arr2
	ans_neon[4]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
	ans_0=vmul_s16(arr1_0,arr2_5); //Dot product of 1th row of arr1, 2th column of arr2
	ans_neon[5]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
	ans_0=vmul_s16(arr1_0,arr2_6); //Dot product of 1th row of arr1, 3th column of arr2
	ans_neon[6]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
	ans_0=vmul_s16(arr1_0,arr2_7); //Dot product of 1th row of arr1, 4th column of arr2
	ans_neon[7]=ans_0[0]+ans_0[1]+ans_0[2]+ans_0[3]+ans_0[4]+ans_0[5]+ans_0[6]+ans_0[7];
    

	
	ans_1=vmul_s16(arr1_1,arr2_0); //Dot product of 2th row of arr1, 1th column of arr2
	ans_neon[8]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
	ans_1=vmul_s16(arr1_1,arr2_1); //Dot product of 2th row of arr1, 2th column of arr2
	ans_neon[9]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
	ans_1=vmul_s16(arr1_1,arr2_2); //Dot product of 2th row of arr1, 3th column of arr2
	ans_neon[10]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
	ans_1=vmul_s16(arr1_1,arr2_3); //Dot product of 2th row of arr1, 4th column of arr2
	ans_neon[11]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
    ans_1=vmul_s16(arr1_1,arr2_4); //Dot product of 2th row of arr1, 1th column of arr2
	ans_neon[12]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
	ans_1=vmul_s16(arr1_1,arr2_5); //Dot product of 2th row of arr1, 2th column of arr2
	ans_neon[13]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
	ans_1=vmul_s16(arr1_1,arr2_6); //Dot product of 2th row of arr1, 3th column of arr2
	ans_neon[14]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
	ans_1=vmul_s16(arr1_1,arr2_7); //Dot product of 2th row of arr1, 4th column of arr2
	ans_neon[15]=ans_1[0]+ans_1[1]+ans_1[2]+ans_1[3]+ans_1[4]+ans_1[5]+ans_1[6]+ans_1[7];
	
	ans_2=vmul_s16(arr1_2,arr2_0); //Dot product of 3th row of arr1, 1th column of arr2
	ans_neon[16]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];
	ans_2=vmul_s16(arr1_2,arr2_1); //Dot product of 3th row of arr1, 2th column of arr2
	ans_neon[17]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];
	ans_2=vmul_s16(arr1_2,arr2_2); //Dot product of 3th row of arr1, 3th column of arr2
	ans_neon[18]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];
	ans_2=vmul_s16(arr1_2,arr2_3); //Dot product of 3th row of arr1, 4th column of arr2
	ans_neon[19]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];
    ans_2=vmul_s16(arr1_2,arr2_4); //Dot product of 3th row of arr1, 1th column of arr2
	ans_neon[20]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];
	ans_2=vmul_s16(arr1_2,arr2_5); //Dot product of 3th row of arr1, 2th column of arr2
	ans_neon[21]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];
	ans_2=vmul_s16(arr1_2,arr2_6); //Dot product of 3th row of arr1, 3th column of arr2
	ans_neon[22]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];
	ans_2=vmul_s16(arr1_2,arr2_7); //Dot product of 3th row of arr1, 4th column of arr2
	ans_neon[23]=ans_2[0]+ans_2[1]+ans_2[2]+ans_2[3]+ans_2[4]+ans_2[5]+ans_2[6]+ans_2[7];

	ans_3=vmul_s16(arr1_3,arr2_0); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[24]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];
	ans_3=vmul_s16(arr1_3,arr2_1); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[25]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];
	ans_3=vmul_s16(arr1_3,arr2_2); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[26]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];
	ans_3=vmul_s16(arr1_3,arr2_3); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[27]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];
    ans_3=vmul_s16(arr1_3,arr2_4); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[28]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];
	ans_3=vmul_s16(arr1_3,arr2_5); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[29]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];
	ans_3=vmul_s16(arr1_3,arr2_6); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[30]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];
	ans_3=vmul_s16(arr1_3,arr2_7); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[31]=ans_3[0]+ans_3[1]+ans_3[2]+ans_3[3]+ans_3[4]+ans_3[5]+ans_3[6]+ans_3[7];

    ans_4=vmul_s16(arr1_4,arr2_0); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[32]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];
	ans_4=vmul_s16(arr1_4,arr2_1); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[33]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];
	ans_4=vmul_s16(arr1_4,arr2_2); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[34]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];
	ans_4=vmul_s16(arr1_4,arr2_3); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[35]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];
    ans_4=vmul_s16(arr1_4,arr2_4); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[36]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];
	ans_4=vmul_s16(arr1_4,arr2_5); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[37]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];
	ans_4=vmul_s16(arr1_4,arr2_6); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[38]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];
	ans_4=vmul_s16(arr1_4,arr2_7); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[39]=ans_4[0]+ans_4[1]+ans_4[2]+ans_4[3]+ans_4[4]+ans_4[5]+ans_4[6]+ans_4[7];

    ans_5=vmul_s16(arr1_5,arr2_0); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[40]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];
	ans_5=vmul_s16(arr1_5,arr2_1); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[41]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];
	ans_5=vmul_s16(arr1_5,arr2_2); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[42]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];
	ans_5=vmul_s16(arr1_5,arr2_3); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[43]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];
    ans_5=vmul_s16(arr1_5,arr2_4); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[44]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];
	ans_5=vmul_s16(arr1_5,arr2_5); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[45]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];
	ans_5=vmul_s16(arr1_5,arr2_6); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[46]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];
	ans_5=vmul_s16(arr1_5,arr2_7); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[47]=ans_5[0]+ans_5[1]+ans_5[2]+ans_5[3]+ans_5[4]+ans_5[5]+ans_5[6]+ans_5[7];

    ans_6=vmul_s16(arr1_6,arr2_0); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[48]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];
	ans_6=vmul_s16(arr1_6,arr2_1); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[49]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];
	ans_6=vmul_s16(arr1_6,arr2_2); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[50]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];
	ans_6=vmul_s16(arr1_6,arr2_3); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[51]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];
    ans_6=vmul_s16(arr1_6,arr2_4); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[52]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];
	ans_6=vmul_s16(arr1_6,arr2_5); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[53]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];
	ans_6=vmul_s16(arr1_6,arr2_6); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[54]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];
	ans_6=vmul_s16(arr1_6,arr2_7); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[55]=ans_6[0]+ans_6[1]+ans_6[2]+ans_6[3]+ans_6[4]+ans_6[5]+ans_6[6]+ans_6[7];

    ans_7=vmul_s16(arr1_7,arr2_0); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[56]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
	ans_7=vmul_s16(arr1_7,arr2_1); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[57]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
	ans_7=vmul_s16(arr1_7,arr2_2); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[58]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
	ans_7=vmul_s16(arr1_7,arr2_3); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[59]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
    ans_7=vmul_s16(arr1_7,arr2_4); //Dot product of 4th row of arr1, 1th column of arr2
	ans_neon[60]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
	ans_7=vmul_s16(arr1_7,arr2_5); //Dot product of 4th row of arr1, 2th column of arr2
	ans_neon[61]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
	ans_7=vmul_s16(arr1_7,arr2_6); //Dot product of 4th row of arr1, 3th column of arr2
	ans_neon[62]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
	ans_7=vmul_s16(arr1_7,arr2_7); //Dot product of 4th row of arr1, 4th column of arr2
	ans_neon[63]=ans_7[0]+ans_7[1]+ans_7[2]+ans_7[3]+ans_7[4]+ans_7[5]+ans_7[6]+ans_7[7];
    
	p1 = clock();
    ///////// Matrix multiplication with NEON end///////////


    int check = 0;
    for (int i = 0; i < 8 * 8; i++) {
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

    printf("Execution time (for) : %7.3lf[us]\n", ((double)np1 - np0) / ((double)CLOCKS_PER_SEC / 1000000));
    printf("Execution time (NEON): %7.3lf[us]\n", ((double)p1 - p0) / ((double)CLOCKS_PER_SEC / 1000000));

    free(arr1);
    free(arr2);
    free(ans_for);
    free(ans_neon);
    return;
}
int main(int argc, char *argv[]) {
    func();

    return 0;
}
