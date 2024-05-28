#include "omp.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ARRAY_SIZE 100000

void v_add(double* x, double* y, double* z);
void vec_slicing(double* x,double* y,double* z);
void vec_chunking(double* x,double* y,double* z);
int main() {

	double z[ARRAY_SIZE];
	double x[ARRAY_SIZE];
	double y[ARRAY_SIZE];

	double error = 0;

	clock_t start, end;

	for (int i = 0; i < ARRAY_SIZE; i++) {
		z[i] = 0;
		x[i] = rand() % 10 + 1;
		y[i] = rand() % 10 + 1;
	}

	start = clock();

	v_add(x, y, z);

	printf("Execution time (simple) : %lf ms\n", (double)clock()-start);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		error = error + (z[i] - (x[i] + y[i]));
	}

	if (error == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");

	error=0;
	for(int i=0;i<ARRAY_SIZE;i++){
		z[i]=0;
	}
	start=clock();
	vec_slicing(x,y,z);
	printf("Execution time(slicing): %.3f ms\n",(double)clock()-start);
	
	for (int i = 0; i < ARRAY_SIZE; i++) {
		error = error + (z[i] - (x[i] + y[i]));
	}

	if (error == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");
	error=0;
	
	for(int i=0;i<ARRAY_SIZE;i++){
		z[i]=0;
	}
	start=clock();
	vec_chunking(x,y,z);
	printf("Execution time(chunking): %.3f ms\n",(double)clock()-start);
	
	for (int i = 0; i < ARRAY_SIZE; i++) {
		error = error + (z[i] - (x[i] + y[i]));
	}

	if (error == 0)
		printf("PASS\n");
	else
		printf("FAIL\n");

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
void vec_slicing(double* x,double* y,double* z){//vec_slicing function
	omp_set_num_threads(6);//num of threads=6
	#pragma omp parallel//executed in parallel by multiple thread
	{
		int num_thread=omp_get_num_threads();//num of threads
		int thread_ID=omp_get_thread_num();//number of threads running

		for(int i=0;i<ARRAY_SIZE;i++){
			if(i%6==thread_ID){
				//When the number of indexes is divided by the total number of threads, the remainder is equal to the number of threads currently running.
				z[i]=x[i]+y[i];//Add the i - th vector component
			}
		}
	}
}

void vec_chunking(double* x,double* y,double* z){//vec_chunking function
	omp_set_num_threads(6);//num of threads=6
	#pragma omp parallel//executed in parallel by multiple thread
	{
		int num_thread=omp_get_num_threads();//num of threads
		int thread_ID=omp_get_thread_num();//number of threads running
		int n=ARRAY_SIZE/num_thread;//The quotient of the array size divided by the total number of threads
		for(int i=0;i<ARRAY_SIZE;i++){
			if(thread_ID==5){
				if(i>=(ARRAY_SIZE/num_thread)*5){
					z[i]=x[i]+y[i];//When the fifth thread runs, it performs the vector addition of the remaining elements.
				}
			}
			else{//If thread id is not 5
				if(thread_ID*n<= i && i<(thread_ID+1)*n){
					z[i]=x[i]+y[i];//Perform vector addition in order of n vector components
				}
			}
		}

	}
}


