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

    int16x8_t A_row0 = vld1q_s16(&arr1[0]);
    int16x8_t A_row1 = vld1q_s16(&arr1[8]);
    int16x8_t A_row2 = vld1q_s16(&arr1[16]);
    int16x8_t A_row3 = vld1q_s16(&arr1[24]);
    int16x8_t A_row4 = vld1q_s16(&arr1[32]);
    int16x8_t A_row5 = vld1q_s16(&arr1[40]);
    int16x8_t A_row6 = vld1q_s16(&arr1[48]);
    int16x8_t A_row7 = vld1q_s16(&arr1[56]);

    int16x8_t B_col0 = vld1q_s16(&arr2[0]);
    int16x8_t B_col1 = vld1q_s16(&arr2[8]);
    int16x8_t B_col2 = vld1q_s16(&arr2[16]);
    int16x8_t B_col3 = vld1q_s16(&arr2[24]);
    int16x8_t B_col4 = vld1q_s16(&arr2[32]);
    int16x8_t B_col5 = vld1q_s16(&arr2[40]);
    int16x8_t B_col6 = vld1q_s16(&arr2[48]);
    int16x8_t B_col7 = vld1q_s16(&arr2[56]);

    int32x4_t C_row0_lo = vmovq_n_s32(0);
    int32x4_t C_row0_hi = vmovq_n_s32(0);
    int32x4_t C_row1_lo = vmovq_n_s32(0);
    int32x4_t C_row1_hi = vmovq_n_s32(0);
    int32x4_t C_row2_lo = vmovq_n_s32(0);
    int32x4_t C_row2_hi = vmovq_n_s32(0);
    int32x4_t C_row3_lo = vmovq_n_s32(0);
    int32x4_t C_row3_hi = vmovq_n_s32(0);
    int32x4_t C_row4_lo = vmovq_n_s32(0);
    int32x4_t C_row4_hi = vmovq_n_s32(0);
    int32x4_t C_row5_lo = vmovq_n_s32(0);
    int32x4_t C_row5_hi = vmovq_n_s32(0);
    int32x4_t C_row6_lo = vmovq_n_s32(0);
    int32x4_t C_row6_hi = vmovq_n_s32(0);
    int32x4_t C_row7_lo = vmovq_n_s32(0);
    int32x4_t C_row7_hi = vmovq_n_s32(0);

    // Multiply-accumulate operations
    C_row0_lo = vmlal_s16(C_row0_lo, vget_low_s16(A_row0), vget_low_s16(B_col0));
    C_row0_hi = vmlal_s16(C_row0_hi, vget_high_s16(A_row0), vget_high_s16(B_col0));
    C_row1_lo = vmlal_s16(C_row1_lo, vget_low_s16(A_row1), vget_low_s16(B_col1));
    C_row1_hi = vmlal_s16(C_row1_hi, vget_high_s16(A_row1), vget_high_s16(B_col1));
    C_row2_lo = vmlal_s16(C_row2_lo, vget_low_s16(A_row2), vget_low_s16(B_col2));
    C_row2_hi = vmlal_s16(C_row2_hi, vget_high_s16(A_row2), vget_high_s16(B_col2));
    C_row3_lo = vmlal_s16(C_row3_lo, vget_low_s16(A_row3), vget_low_s16(B_col3));
    C_row3_hi = vmlal_s16(C_row3_hi, vget_high_s16(A_row3), vget_high_s16(B_col3));
    C_row4_lo = vmlal_s16(C_row4_lo, vget_low_s16(A_row4), vget_low_s16(B_col4));
    C_row4_hi = vmlal_s16(C_row4_hi, vget_high_s16(A_row4), vget_high_s16(B_col4));
    C_row5_lo = vmlal_s16(C_row5_lo, vget_low_s16(A_row5), vget_low_s16(B_col5));
    C_row5_hi = vmlal_s16(C_row5_hi, vget_high_s16(A_row5), vget_high_s16(B_col5));
    C_row6_lo = vmlal_s16(C_row6_lo, vget_low_s16(A_row6), vget_low_s16(B_col6));
    C_row6_hi = vmlal_s16(C_row6_hi, vget_high_s16(A_row6), vget_high_s16(B_col6));
    C_row7_lo = vmlal_s16(C_row7_lo, vget_low_s16(A_row7), vget_low_s16(B_col7));
    C_row7_hi = vmlal_s16(C_row7_hi, vget_high_s16(A_row7), vget_high_s16(B_col7));

    vst1q_s32((int32_t*)&ans_neon[0], C_row0_lo);
    vst1q_s32((int32_t*)&ans_neon[4], C_row0_hi);
    vst1q_s32((int32_t*)&ans_neon[8], C_row1_lo);
    vst1q_s32((int32_t*)&ans_neon[12], C_row1_hi);
    vst1q_s32((int32_t*)&ans_neon[16], C_row2_lo);
    vst1q_s32((int32_t*)&ans_neon[20], C_row2_hi);
    vst1q_s32((int32_t*)&ans_neon[24], C_row3_lo);
    vst1q_s32((int32_t*)&ans_neon[28], C_row3_hi);
    vst1q_s32((int32_t*)&ans_neon[32], C_row4_lo);
    vst1q_s32((int32_t*)&ans_neon[36], C_row4_hi);
    vst1q_s32((int32_t*)&ans_neon[40], C_row5_lo);
    vst1q_s32((int32_t*)&ans_neon[44], C_row5_hi);
    vst1q_s32((int32_t*)&ans_neon[48], C_row6_lo);
    vst1q_s32((int32_t*)&ans_neon[52], C_row6_hi);
    vst1q_s32((int32_t*)&ans_neon[56], C_row7_lo);
    vst1q_s32((int32_t*)&ans_neon[60], C_row7_hi);

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
