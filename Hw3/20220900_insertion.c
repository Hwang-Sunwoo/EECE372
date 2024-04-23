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
}

void insertion_ASM(int arr[], int num){
asm(
        // r2: i / r3 j / r4: j - 1 / r8: num
        // r5: v / r6: arr[j] / r7: arr / r1: arr[j - 1]
        "MOV r2, #1\n\t" // i = 1;
        "LDR r7, %[arr]\n\t" //r7 = arr
        "LDR r8, %[num]\n\t"
        "CMP r8, #1\n\t" //num <= 1
        "BLE end_func\n\t" //return
        
        "start_for:\n\t" //for(){
            "CMP r2, r8\n\t" // i < num
            "BGE end_func\n\t" //for(){}
            "LDR r5, [r7, r2, LSL #2]\n\t" // v = arr[i]
            "MOV r3, r2\n\t" //j = i
            
        "start_while:\n\t" //while(){
        
            "CMP r3, #1\n\t" //j >= 1
            "BLT end_while\n\t" //while(){}
           
            "SUB r4, r3, #1\n\t" //j - 1
            "LDR r1, [r7, r4, LSL #2]\n\t" //arr[j - 1]
            "CMP r1, r5\n\t" //arr[j - 1] > v
            "BLE end_while\n\t" //while(){}

            "STR r1, [r7, r3, LSL #2]\n\t" //arr[j] = arr[j - 1]
            "SUB r3, r3, #1\n\t" //j--
            "B start_while\n\t" //while(){
        
        "end_while:\n\t"
            "STR r5, [r7, r3, LSL #2]\n\t" //arr[j] = v
            "ADD r2, r2, #1\n\t" //i++
            "B start_for\n\t" //for()
            
        "end_func:\n\t"
        
        :
        : [arr] "r"(arr), [num] "r"(num)
        : "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8"
        );
        return;
}
