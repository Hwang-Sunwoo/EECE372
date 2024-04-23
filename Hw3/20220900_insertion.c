#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void presort(int arr[], int num);
void shuffle(int arr[], int arr1[], int arr2[], int num);
void insertion_C(int arr[], int num);
void insertion_ASM(int arr[], int num);
int main(int argc, const char * argv[]) {

    
    int n, i;
    double spent_time_C, spent_time_ASM;
    struct timespec C_start_time, C_end_time, ASM_start_time, ASM_end_time;
    
    //scanf("%d", &n);
    n = atoi(argv[1]);
    int array[n], post_C[n], post_ASM[n];
    
    presort(array, n);
    shuffle(array, post_C, post_ASM, n);
    
    clock_gettime(CLOCK_MONOTONIC, &C_start_time); //the time until the start time
    insertion_C(post_C, n);
    clock_gettime(CLOCK_MONOTONIC, &C_end_time); //the time until the end time
    spent_time_C = (C_end_time.tv_sec - C_start_time.tv_sec) + (C_end_time.tv_nsec - C_start_time.tv_nsec) / 1e9; //the time spent during bubble
    
    
    clock_gettime(CLOCK_MONOTONIC, &ASM_start_time); //the time until the start time
    insertion_ASM(post_ASM, n);
    clock_gettime(CLOCK_MONOTONIC, &ASM_end_time); //the time until the end time
    spent_time_ASM = (ASM_end_time.tv_sec - ASM_start_time.tv_sec) + (ASM_end_time.tv_nsec - ASM_start_time.tv_nsec) / 1e9; //the time spent during bubble
    
    if(n <= 20){
        
        printf("Before sort     : [ ");
        for(i = 0; i < n; i++){
            
            printf("%d ", array[i]);
        }
        printf("]\n");
        
        printf("After sort    (C): [ ");
        for(i = 0; i < n; i++){
            
            printf("%d ", post_C[i]);
        }
        printf("]\n");
        
        printf("After sort (ASM): [ ");
        for(i = 0; i < n; i++){
            
            printf("%d ", post_ASM[i]);
        }
        printf("]\n");
    }
    printf("Execution Time    (C): %.6f[s]\n", spent_time_C);
    printf("Execution Time (ASM): %.6f[s]\n", spent_time_ASM);

    
    return 0;
}
void presort(int arr[], int num){
    
    int i = 0;
    
    for(i = 0; i < num; i++){
        arr[i] = i + 1;
    }
}
void shuffle(int arr[], int arr1[], int arr2[], int num){
    
    int i, j, temp;
    srand((unsigned)time(NULL)); //making a random number
//shuffle elements using rand()
    for(i = num - 1; i > 0; i--){
        j = rand() % (i + 1);
        temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
        arr1[i] = arr[i];
        arr2[i] = arr[i];
    }
    arr1[0] = arr[0];
    arr2[0] = arr[0];
    
    return;
}
void insertion_C(int arr[], int num){

    int i, j, v;
    for(i = 1; i <= num - 1; i++){
        v = arr[i];
        j = i;
        while(arr[j - 1] > v && j >= 1){
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = v;
    }
    return;
}void insertion_ASM(int arr[], int num){
        // r2: i / r3 j /
        // r5: v / r6: j + 1 / r1: arr[j]
    asm(
        "MOV r2, #1\n\t" //i = 1;
        "start_for:\n\t" //for(){
        
            "CMP r2, %[num]\n\t" // i < num
            "BGE end_func\n\t" //for(){}
            "LDR r5, [%[arr], r2, LSL #2]\n\t" //v = arr[i]
            "SUB r3, r2, #1\n\t" // j = i - 1
        
        "start_while:\n\t" //while(){
        
            "CMP r3, #0\n\t" //j >= 1
            "BLT end_while\n\t" //while(){}
        
            "LDR r1, [%[arr], r3, LSL #2]\n\t" //arr[j]
            "CMP r1, r5\n\t" //arr[j] > v
            "BLE end_while\n\t" //while(){}
        
            "ADD r6, r3, #1\n\t"
            "STR r1, [%[arr], r6, LSL #2]\n\t" //arr[j + 1] = arr[j]
            "SUB r3, r3, #1\n\t"  //j--
            "B start_while\n\t" //while(){
        
        "end_while:\n\t"
            "ADD r3, r3, #1\n\t"
            "STR r5, [%[arr], r3, LSL #2]\n\t" //arr[j] = v
            "ADD r2, r2, #1\n\t" //i++
            "B start_for\n\t" //for()
        
        "end_func:\n\t"    
        : // input operands
        : [num] "r"(num), [arr] "r"(arr)  // output operands
        : "r1", "r2", "r3", "r6", "r5" //clobbers
        );
        return;
}
