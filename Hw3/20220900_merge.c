#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void presort(int arr[], int num);
void shuffle(int arr[], int arr1[], int arr2[], int num);
void merge_C(int arr[], int left, int mid, int right);
void mergesort_C(int arr[], int left, int right);
void merge_ASM(int arr[], int left, int mid, int right);
void mergesort_ASM(int arr[], int left, int right);
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
    mergesort_C(post_C, 0, n - 1);
    clock_gettime(CLOCK_MONOTONIC, &C_end_time); //the time until the end time
    spent_time_C = (C_end_time.tv_sec - C_start_time.tv_sec) + (C_end_time.tv_nsec - C_start_time.tv_nsec) / 1e9; //the time spent during bubble
    
    
    clock_gettime(CLOCK_MONOTONIC, &ASM_start_time); //the time until the start time
    mergesort_ASM(post_ASM, 0, n - 1);
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
void merge_C(int arr[], int left, int mid, int right){

    int i, j, k;
    int a = mid - left + 1;
    int b = right - mid;
    int L[a], R[b];
    
    for(i = 0; i < a; i++){
        L[i] = arr[left + i];
    }
    for(j = 0; j < b; j++){
        R[j] = arr[mid + 1 + j];
    }
    
    i = 0;
    j = 0;
    k = left;
    while(i < a && j < b){
        if(L[i] <= R[j]){
            arr[k] = L[i];
            i++;
        }else{
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    while(i < a){
        arr[k] = L[i];
        i++;
        k++;
    }
    while(j < b){
        arr[k] = R[j];
        j++;
        k++;
    }
    return;
}
void merge_ASM(int arr[], int left, int right, int mid) {
    
    int *LA;
    int *RA;
    int a, b;
    LA = (int*)malloc(sizeof(int) * (mid - left + 1));
    RA = (int*)malloc(sizeof(int) * right - mid);
    // r0: arr / r1: left / r2: right / r3: mid
    // r4: i / r5: j / r6: k
    // LA: left array / RA: right array
    // r8 / r9
    
    asm(

        "SUB %[a], %[mid], %[right]\n\t"
        "ADD %[a], %[a], #1\n\t"
        "SUB %[b], %[right], %[mid]\n\t"
        
        "left_array:\n\t"
        "MOV r4, #0\n\t"
        "CMP r4, %[a]\n\t"
        "BGE right_array\n\t"
        "ADD r8, %[left], r4\n\t"
        "LDR r8, [%[arr], r8, LSL #2]\n\t"
        "STR r8, [%[LA], r4]\n\t"
        "ADD r4, #1\n\t" //i++
        "B left_array\n\t"
        
        "right_array:\n\t"
        "MOV r5, #0\n\t"
        "CMP r5, %[b]\n\t"
        "BGE merge_loop"
        "ADD r8, %[mid], #1\n\t"
        "ADD r8, r9, r5\n\t"
        "LDR r8, [%[arr], r8, LSL #2]\n\t"
        "STR r8, [%[RA], r5]\n\t"
        "ADD r5, r5, #1\n\t"
        "B rigth_array\n\t"
        
        "MOV r4, #0\n\t"
        "MOV r5, #0\n\t"
        "MOV r6, %[left]\n\t"
        
        // 왼쪽 배열과 오른쪽 배열을 병합하면서 임시 배열에 저장
        "merge_loop:\n\t"
            "CMP r4, %[a]\n\t"
            "BGE compare\n\t"
            "CMP r5, %[b]\n\t"
            "BGE compare_left\n\t"
        
        "LDR r8, [%[LA], r4]\n\t"
        "LDR r9, [%[RA], r5]\n\t"
        "CMP r8, r9\n\t"
        "BLT copy_left\n\t"
        "B copy_right\n\t"
        
        "update_k:\n\t"
        "ADD, r6, r6, #1\n\t"
        "B merge_loop\n\t"
        

        "compare_left:\n\t"
        "CMP r4, %[a]\n\t"
        "BGE compare_right\n\t"
        "B copy_left\n\t"
        
        "compare_right:\n\t"
        "CMP r5, %[b]\n\t"
        "BGE end_merge\n\t"
        
        
        "copy_left:\n\t"
        "LDR r8, [%[LA], r4]\n\t"
        "STR r8, [%[arr], r6, LSL #2]\n\t"
        "ADD r4, r4, #1\n\t"
        "B update_k\n\t"
        
        "copy_right:\n\t"
        "LDR r8, [%[RA], r5]\n\t"
        "STR r8, [%[arr], r6, LSL #2]\n\t"
        "ADD r5, r5, #1\n\t"
        "B update_k\n\t"
        

        :
        : [arr] "r"(arr), [left] "r"(left), [mid] "r"(mid), [right] "r"(right), [LA] "r"(LA), [RA] "r"(RA), [a] "r"(a), [b] "r"(b)
        : "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9"
    );
}

void mergesort_C(int arr[], int left, int right){
    
    int mid;
    
    if(left < right){
        mid = left + (right - left) / 2;
        mergesort_C(arr, left, mid);
        mergesort_C(arr, mid + 1, right);
        
        merge_C(arr, left, mid, right);
    }
    
    return;
}
void mergesort_ASM(int arr[], int left, int right){
    
    asm(
        // r0: arr / r1: left / r2: right / r3:mid / r4: length
                
        "SUB r4, %[right], %[left]\n\t"
        "ADD r4, r4, #1\n\t"
        
        "CMP r4, #1\n\t"        // 배열 크기가 1 이하이면 종료
        "BLE end_sort\n\t" //return
        
        // 배열의 중간 인덱스 계산
        "ADD r3, %[left], %[right]\n\t"       // r3 = left + right
        "LSR r3, r3, #1\n\t"       // r3 = (left + right) / 2
        
        // 배열의 왼쪽 부분을 병합 정렬
        "mov r2, r3\n\t"           // 배열의 시작 주소를 r5에 저장
        "bl mergesort_ASM\n\t"     // 재귀 호출
        
        // 배열의 오른쪽 부분을 병합 정렬
        "add r1, r3, #1\n\t" // 배열의 시작 주소를 중간으로 이동
        "bl mergesort_ASM\n\t"     // 재귀 호출
        
        // 배열 병합
        "mov r1, %[left]\n\t"           // r2에 왼쪽 배열의 시작 주소를 복사
        "mov r2, %[right]\n\t"           // r3에 오른쪽 배열의 시작 주소를 복사
        "bl merge_ASM\n\t"         // 병합 함수 호출
        
    "end_sort:"

        :
        : [arr] "r"(arr), [left] "r"(left), [right] "r"(right)
        : "r0", "r1", "r2", "r3", "r4"
        );
    return;
}
