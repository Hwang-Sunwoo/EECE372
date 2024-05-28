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

	clock_t start, end;

	for (int i = 0; i < ARRAY_SIZE; i++) {
		z[i] = 0;
		x[i] = rand() % 10 + 1;
		y[i] = rand() % 10 + 1;
	}

	start = clock();

	global_sum = dotp(x, y);

	printf("Execution time (dotp) : %.3lf ms\n", (double)clock()-start);


	for (int i = 0; i < ARRAY_SIZE; i++)
		global_sum_ref += x[i] * y[i];

	for(int i = 0; i < ARRAY_SIZE; i++) {
		error = global_sum - global_sum_ref;
	}

	if (error == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");

	error=0;
	global_sum=0;
	global_sum_ref=0;
	start=0;

	start=clock();
	global_sum=dotp_omp(x,y);
	printf("Execution time(dotp_omp) : %.3lf ms\n",(double)clock()-start);

	for (int i = 0; i < ARRAY_SIZE; i++)
		global_sum_ref += x[i] * y[i];

	for(int i = 0; i < ARRAY_SIZE; i++) {
		error = global_sum - global_sum_ref;
	}

	if (error == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");

	
	return 0;
}

double dotp(double* x, double* y) {

	double global_sum = 0.0;

	for (int i = 0; i < ARRAY_SIZE; i++)
		global_sum += x[i] * y[i];

	return global_sum;
}

double dotp_omp(double* x, double* y) {
	omp_set_num_threads(6);//total number of threads
	double global_sum = 0.0;//result reset
	#pragma omp parallel//executed in parallel by multiple threads.
	
	// change to your own code (start)
	#pragma omp for//Execute for statements in parallel		
	for (int i = 0; i < ARRAY_SIZE; i++){
		#pragma omp critical//define a critical section within a parallel region
		global_sum += x[i] * y[i];
	}	

	// change to your own code (end)
	
	return global_sum;//return result
}

