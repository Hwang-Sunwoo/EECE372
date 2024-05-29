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
    p0 = clock();

    // Load rows of arr1
    int16x8_t A_row0 = vld1q_s16(&arr1[0]);
    int16x8_t A_row1 = vld1q_s16(&arr1[8]);
    int16x8_t A_row2 = vld1q_s16(&arr1[16]);
    int16x8_t A_row3 = vld1q_s16(&arr1[24]);
    int16x8_t A_row4 = vld1q_s16(&arr1[32]);
    int16x8_t A_row5 = vld1q_s16(&arr1[40]);
    int16x8_t A_row6 = vld1q_s16(&arr1[48]);
    int16x8_t A_row7 = vld1q_s16(&arr1[56]);

    // Initialize result vectors
    int32x4_t C_row0 = vmovq_n_s32(0);
    int32x4_t C_row1 = vmovq_n_s32(0);
    int32x4_t C_row2 = vmovq_n_s32(0);
    int32x4_t C_row3 = vmovq_n_s32(0);
    int32x4_t C_row4 = vmovq_n_s32(0);
    int32x4_t C_row5 = vmovq_n_s32(0);
    int32x4_t C_row6 = vmovq_n_s32(0);
    int32x4_t C_row7 = vmovq_n_s32(0);

    for (int i = 0; i < 8; i++) {
        int16x8_t B_col = vld1q_s16(&arr2[i * 8]);

        // Multiply and accumulate
        C_row0 = vmlal_s16(C_row0, vget_low_s16(A_row0), B_col, i);
        C_row0 = vmlal_s16(C_row0, vget_high_s16(A_row0), B_col, i);
        
        C_row1 = vmlal_s16(C_row1, vget_low_s16(A_row1), B_col, i);
        C_row1 = vmlal_s16(C_row1, vget_high_s16(A_row1), B_col, i);
        
        C_row2 = vmlal_s16(C_row2, vget_low_s16(A_row2), B_col, i);
        C_row2 = vmlal_s16(C_row2, vget_high_s16(A_row2), B_col, i);
        
        C_row3 = vmlal_s16(C_row3, vget_low_s16(A_row3), B_col, i);
        C_row3 = vmlal_s16(C_row3, vget_high_s16(A_row3), B_col, i);
        
        C_row4 = vmlal_s16(C_row4, vget_low_s16(A_row4), B_col, i);
        C_row4 = vmlal_s16(C_row4, vget_high_s16(A_row4), B_col, i);
        
        C_row5 = vmlal_s16(C_row5, vget_low_s16(A_row5), B_col, i);
        C_row5 = vmlal_s16(C_row5, vget_high_s16(A_row5), B_col, i);
        
        C_row6 = vmlal_s16(C_row6, vget_low_s16(A_row6), B_col, i);
        C_row6 = vmlal_s16(C_row6, vget_high_s16(A_row6), B_col, i);
        
        C_row7 = vmlal_s16(C_row7, vget_low_s16(A_row7), B_col, i);
        C_row7 = vmlal_s16(C_row7, vget_high_s16(A_row7), B_col, i);
    }

    // Store results
    vst1q_s32((int32_t*)&ans_neon[0], C_row0);
    vst1q_s32((int32_t*)&ans_neon[8], C_row1);
    vst1q_s32((int32_t*)&ans_neon[16], C_row2);
    vst1q_s32((int32_t*)&ans_neon[24], C_row3);
    vst1q_s32((int32_t*)&ans_neon[32], C_row4);
    vst1q_s32((int32_t*)&ans_neon[40], C_row5);
    vst1q_s32((int32_t*)&ans_neon[48], C_row6);
    vst1q_s32((int32_t*)&ans_neon[56], C_row7);

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
