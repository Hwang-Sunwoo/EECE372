#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "omp.h"

#define ARRAY_SIZE 1000000

void vec_simple(double *x, double *y, double *z);
void vec_slicing(double *x, double *y, double *z);
void vec_chunking(double *x, double *y, double *z);

int main() {
    double error1 = 0;
    double error2 = 0;
    double *x = malloc(sizeof(double) * ARRAY_SIZE);
    double *y = malloc(sizeof(double) * ARRAY_SIZE);
    double *z = malloc(sizeof(double) * ARRAY_SIZE);

    clock_t start, end;

    srand((unsigned)time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        z[i] = 0;
        x[i] = rand() % 10 + 1;
        y[i] = rand() % 10 + 1;
    }

    start = clock();
    vec_simple(x, y, z);
    printf("Execution time (simple)  : %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    start = clock();
    vec_slicing(x, y, z);
    printf("Execution time (slicing) : %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        error1 = error1 + (z[i] - (x[i] + y[i]));
    }

    start = clock();
    vec_chunking(x, y, z);
    printf("Execution time (chunking): %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        error2 = error2 + (z[i] - (x[i] + y[i]));
    }

    if (error1 == 0 && error2 == 0) {
        printf("PASS\n");
    }
    else {
        printf("FAIL\n");
    }

    free(x);
    free(y);
    free(z);
    return 0;
}

void vec_simple(double *x, double *y, double *z) {
    omp_set_num_threads(6);
#pragma omp parallel
    {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            z[i] = x[i] + y[i];
        }
    }
}

void vec_slicing(double *x, double *y, double *z) {

	omp_set_num_threads(6); // Set the number of threads to 6

#pragma omp parallel
	{	
		int thread_ID = omp_get_thread_num();

		switch (thread_ID) { // Perform different actions depending on the value of the thread ID
		case 0:
			// thread 0 is i % 6 == 0 (i = 0, 6, 12,...)
			for (int i = 0; i < ARRAY_SIZE; i += 6)
				z[i] = x[i] + y[i];
			break;
		case 1:
			// thread 1 is i % 6 == 1 (i = 1, 7, 13,...)
			for (int i = 1; i < ARRAY_SIZE; i += 6)
				z[i] = x[i] + y[i];
			break;
		case 2:
			// thread 2 is i % 6 == 2 (i = 2,8,14,...)
			for (int i = 2; i < ARRAY_SIZE; i += 6)
				z[i] = x[i] + y[i];
			break;
		case 3:
			// thread 3 is i % 6 == 3 (i = 3,9,15,...)
			for (int i = 3; i < ARRAY_SIZE; i += 6)
				z[i] = x[i] + y[i];
			break;
		case 4:
			// thread 4 is i % 6 == 4 (i = 4,10,16,...)
			for (int i = 4; i < ARRAY_SIZE; i += 6)
				z[i] = x[i] + y[i];
			break;
		case 5:
			// thread 5 is i % 6 == 5 (i = 5,11,17,...)
			for (int i = 5; i < ARRAY_SIZE; i += 6)
				z[i] = x[i] + y[i];
			break;
		}
	
    }
}

void vec_chunking(double *x, double *y, double *z) {
    omp_set_num_threads(6);
    // Write Your Code
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        int chunk_size = (ARRAY_SIZE + num_threads - 1) / num_threads;
        int start = thread_id * chunk_size;
        int end = (start + chunk_size > ARRAY_SIZE) ? ARRAY_SIZE : start + chunk_size;

        for (int i = start; i < end; i++) {
            z[i] = x[i] + y[i];
        }
    }
}
