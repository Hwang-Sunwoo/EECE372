/* student ID : 20210404 */

#include "omp.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ARRAY_SIZE 100000

void v_add(double* x, double* y, double* z);
void vec_slicing(double* x, double* y, double* z);
void vec_chunking(double* x, double* y, double* z);

int main() {

	double z[ARRAY_SIZE];
	double x[ARRAY_SIZE];
	double y[ARRAY_SIZE];

	double error = 0;
	double error_slicing = 0;
	double error_chunking = 0;

	clock_t start, end;

	for (int i = 0; i < ARRAY_SIZE; i++) {
		z[i] = 0;
		x[i] = rand() % 10 + 1;
		y[i] = rand() % 10 + 1;
	}


	///////////////////  vector addition original (start)  ///////////////////
	start = clock();

	v_add(x, y, z);

	printf("Execution time (original) : %lf ms\n", (double)clock()-start);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		error = error + (z[i] - (x[i] + y[i]));
	}

	if (error == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");
	///////////////////  vector addition original (end)  ///////////////////



	///////////////////  vector addition slicing (start)  ///////////////////
	start = clock();

	vec_slicing(x, y, z);

	printf("Execution time (slicing) : %lf ms\n", (double)clock() - start);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		error_slicing = error_slicing + (z[i] - (x[i] + y[i]));
	}

	if (error_slicing == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");
	///////////////////  vector addition slicing (end)  ///////////////////



	///////////////////  vector addition chunking (start)  ///////////////////
	start = clock();

	vec_chunking(x, y, z);

	printf("Execution time (chunking) : %lf ms\n", (double)clock() - start);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		error_chunking = error_chunking + (z[i] - (x[i] + y[i]));
	}

	if (error_chunking == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");
	///////////////////  vector addition chunking (end)  ///////////////////

	return 0;
}

void v_add(double* x, double* y, double* z) {

	omp_set_num_threads(6);

	#pragma omp parallel
	{
		for(int i = 0; i< ARRAY_SIZE; i++)
			z[i] = x[i] + y[i];
	}
}

void vec_slicing(double* x, double* y, double* z) {

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

void vec_chunking(double* x, double* y, double* z) {

	omp_set_num_threads(6); // Set the number of threads to 6

#pragma omp parallel
	{
		int num_thread = omp_get_num_threads();
		int thread_ID = omp_get_thread_num();
		
		int chunk_size = ARRAY_SIZE / 6;

		switch (thread_ID) { // Perform different actions depending on the value of the thread ID
		case 0:
			// thread 0 is 0 to (chunk_size - 1)
			for (int i = 0; i < chunk_size; i++)
				z[i] = x[i] + y[i];
			break;
		case 1:
			// thread 1 is chunk_size to (2 * chunk_size - 1)
			for (int i = chunk_size; i < 2 * chunk_size; i++)
				z[i] = x[i] + y[i];
			break;
		case 2:
			// thread 2 is 2 * chunk_size to (3 * chunk_size - 1)
			for (int i = 2 * chunk_size; i < 3 * chunk_size; i++)
				z[i] = x[i] + y[i];
			break;
		case 3:
			// thread 3 is 3 * chunk_size to (4 * chunk_size - 1)
			for (int i = 3 * chunk_size; i < 4 * chunk_size; i++)
				z[i] = x[i] + y[i];
			break;
		case 4:
			// thread 4 is 4 * chunk_size to (5 * chunk_size - 1)
			for (int i = 4 * chunk_size; i < 5 * chunk_size; i++)
				z[i] = x[i] + y[i];
			break;
		case 5:
			// thread 5 is 5 * chunk_size to ARRAY_SIZE
			for (int i = 5 * chunk_size; i < ARRAY_SIZE; i++)
				z[i] = x[i] + y[i];
			break;
		};
	}
}

