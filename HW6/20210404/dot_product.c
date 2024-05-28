/* student ID : 20210404 */

#include "omp.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 100000

double dotp(double* x, double* y);
double dotp_omp(double* x, double* y);

int main() {

	double z[ARRAY_SIZE];
	double x[ARRAY_SIZE];
	double y[ARRAY_SIZE];

	double global_sum = 0;
	double global_sum_ref = 0;

	double error = 0;
	double error_omp = 0;

	clock_t start, end;

	for (int i = 0; i < ARRAY_SIZE; i++) {
		z[i] = 0;
		x[i] = rand() % 10 + 1;
		y[i] = rand() % 10 + 1;
	}

	///////////////////  dot product (start)  ///////////////////
	start = clock();

	global_sum = dotp(x, y);

	printf("Execution time (dotp) : %lf ms\n", (double)clock()-start);

	for (int i = 0; i < ARRAY_SIZE; i++)
		global_sum_ref += x[i] * y[i];

	for(int i = 0; i < ARRAY_SIZE; i++) {
		error = global_sum - global_sum_ref;
	}

	if (error == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");
	///////////////////  dot product (end)  ///////////////////
	



	///////////////////  dot product omp (start)  /////////////////
	start = clock();

	global_sum = dotp_omp(x, y);

	printf("Execution time (dotp_omp) : %lf ms\n", (double)clock() - start);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		error_omp = global_sum - global_sum_ref;
	}

	if (error_omp == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");
	///////////////////  dot product omp (end)  /////////////////

	return 0;
}

double dotp(double* x, double* y) {

	double global_sum = 0.0;

	for (int i = 0; i < ARRAY_SIZE; i++)
		global_sum += x[i] * y[i];

	return global_sum;
}

double dotp_omp(double* x, double* y) {

	omp_set_num_threads(6);

	double global_sum = 0.0;

	#pragma omp parallel
	{
		// change to your own code (start)
		int num_thread = omp_get_num_threads();
		int thread_ID = omp_get_thread_num();
		
		#pragma omp for // for statement parallelization
		for (int i = 0; i < ARRAY_SIZE; i++) {
			#pragma omp critical // Set critical section
			{ 
			global_sum += x[i] * y[i];
			}
		}
		// change to your own code (end)
	}

	return global_sum;
}
