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
        "PUSH {r4, r5, lr}\n\t" // 레지스터 보존
        
        "MOV r4, #1\n\t"          // i = 1;
        "LDR r5, #0\n\t"        // r5 = 0
        "LDR r5, [%[arr], r5]\n\t"  // r5 = &arr[0]
        "LDR r3, %[num]\n\t"        // r3 = num
        "CMP r3, #1\n\t"          // if (num <= 1)
        "BLE L1\n\t"
        //for(){
    "L2:\n\t"
        "LDR r2, [r5, r4]\n\t" // r2 = arr[i]
        "MOV r6, r4\n\t"              // j = i;
        "CMP r6, #0\n\t"              // if (j >= 1)
        "BLT L4\n\t"
        //while(){
    "L3:\n\t"
        "LDR r1, [r5, r6]\n\t" // r1 = arr[j]
        "CMP r1, r2\n\t"              // if (arr[j - 1] <= arr[i])
        "BLE L4\n\t"
        "STR r1, [r5, r6]\n\t" // arr[j] = arr[j - 1];
        "SUBS r6, r6, #1\n\t"          // j--;
        "CMP r6, #0\n\t"               // if (j >= 1)
        "BGE L3\n\t"
        //}
    "L4:\n\t"
        "STR r2, [r5, r6]\n\t" // arr[j] = v;
        "ADDS r4, r4, #1\n\t"          // i++;
        "CMP r4, r3\n\t"               // if (i < num)
        "BLT L2\n\t"
        //}
    "L1:\n\t"

        "POP {r4, r5, pc}\n\t" // 레지스터 복원 및 리턴

        : // 출력 (output) 부분은 없으므로 비워둡니다.
        : [arr] "r"(arr), [num] "m"(num) // 입력 (input) 목록에 arr과 num을 추가합니다.
        : "r2", "r3", "r4", "r5", "r6" // 어셈블리어 코드에서 사용된 레지스터들을 나열합니다.
    
        );
        return;
}
