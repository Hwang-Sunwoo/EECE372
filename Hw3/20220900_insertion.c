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
    
    scanf("%d", &n);
    //n = atoi(argv[1]);
    int array[n], post_C[n], post_ASM[n];
    
    presort(array, n);
    shuffle(array, post_C, post_ASM, n);
    

    
    
    clock_gettime(CLOCK_MONOTONIC, &C_start_time); //the time until the start time
    insertion_C(post_C, n);
    clock_gettime(CLOCK_MONOTONIC, &C_end_time); //the time until the end time
    spent_time_C = (C_end_time.tv_sec - C_start_time.tv_sec) + (C_end_time.tv_nsec - C_start_time.tv_nsec) / 1e9; //the time spent during bubble
    
    /*
    clock_gettime(CLOCK_MONOTONIC, &ASM_start_time); //the time until the start time
    insertion_ASM(post_ASM, n);
    clock_gettime(CLOCK_MONOTONIC, &ASM_end_time); //the time until the end time
    spent_time_ASM = (ASM_end_time.tv_sec - ASM_start_time.tv_sec) + (ASM_end_time.tv_nsec - ASM_start_time.tv_nsec) / 1e9; //the time spent during bubble
    */
    
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
    //printf("Execution Time (ASM): %.6f[s]\n", spent_time_ASM);

    
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
/*
void insertion_ASM(int arr[], int num){
    
    asm(
        "mov r4, #1\n\t" // i = 1;
        "movl %[num], r5\n\t" // r5 = num
        "loop_start:\n"
            "movl (%%edi, %1, 4), %%eax\n" // eax = arr[i]
            "mov %%edi, %%esi\n" // esi = i
            "cmp $0, %%esi\n" // if (j >= 1)
            "jge inner_loop:\n"
            "jmp loop_end:\n" // else break;
            "inner_loop:\n"
                "movl (%%esi, %1, 4), %%ebx\n" // ebx = arr[j]
                "cmp %%eax, %%ebx\n" // if (arr[j - 1] > arr[i])
                "jle loop_end:\n" // break;
                "movl %%ebx, (%%esi, %1, 4)\n" // arr[j] = arr[j - 1];
                "sub $1, %%esi\n" // j--;
                "cmp $0, %%esi\n" // if (j >= 1)
                "jge inner_loop:\n"
            "loop_end:\n"
            "mov %%eax, (%%esi, %1, 4)\n" // arr[j] = v;
            "add $1, %%edi\n" // i++;
            "cmp %%ecx, %%edi\n" // if (i < num)
            "jl loop_start:\n"
        :
        : "r" (num), "r" (arr)
        : "%eax", "%ebx", "%ecx", "%edi", "%esi"
        );

    return;
}
*/
