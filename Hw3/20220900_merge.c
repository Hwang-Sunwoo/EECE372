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
void merge_ASM(int arr[], int left, int mid, int right) {
    int *temp;
    asm(
        "push {r4, r5, r6, r7, r9, lr}\n\t" // 레지스터 보존

        // 병합된 배열을 저장할 임시 메모리 공간 할당
        "add r4, %[left], %[right]\n\t"       // r4 = left + right
        "lsl r4, r4, #2\n\t"                  // 병합된 배열의 크기 * 4 (한 요소의 크기)
        "bl malloc\n\t"                       // malloc 함수 호출하여 메모리 할당
        "mov r9, r0\n\t"                      // 할당된 메모리 주소를 r9에 저장 (임시 배열의 시작 주소)

        // 왼쪽 배열과 오른쪽 배열을 병합하면서 임시 배열에 저장
        "merge_loop:\n\t"
            // 왼쪽 배열이 비어있으면 오른쪽 배열의 요소를 임시 배열에 복사
            "cmp %[left], %[mid]\n\t"
            "beq copy_right\n\t"

            // 오른쪽 배열이 비어있으면 왼쪽 배열의 요소를 임시 배열에 복사
            "cmp %[mid], %[right]\n\t"
            "beq copy_left\n\t"

            // 왼쪽 배열과 오른쪽 배열의 첫 번째 요소 비교
            "ldr r7, [%[arr], %[left], lsl #2]\n\t"   // r7 = arr[left]
            "ldr r8, [%[arr], %[mid], lsl #2]\n\t"    // r8 = arr[mid]
            "cmp r7, r8\n\t"                          // if (arr[left] <= arr[mid])
            "ble left_smaller\n\t"                    // 왼쪽 배열의 첫 번째 요소가 더 작거나 같으면 left_smaller 레이블로 분기
            "bgt right_smaller\n\t"                   // 오른쪽 배열의 첫 번째 요소가 더 작으면 right_smaller 레이블로 분기

        "left_smaller:\n\t"
            // 왼쪽 배열의 첫 번째 요소를 임시 배열에 복사하고 포인터를 이동
            "ldr r7, [%[arr], %[left], lsl #2]\n\t"    // r7 = arr[left]
            "str r7, [r9], #4\n\t"                     // 임시 배열에 저장 후 포인터 이동
            "add %[left], %[left], #1\n\t"             // 왼쪽 배열의 포인터 이동
            "b merge_loop\n\t"                         // 다음 반복

        "right_smaller:\n\t"
            // 오른쪽 배열의 첫 번째 요소를 임시 배열에 복사하고 포인터를 이동
            "ldr r8, [%[arr], %[mid], lsl #2]\n\t"     // r8 = arr[mid]
            "str r8, [r9], #4\n\t"                     // 임시 배열에 저장 후 포인터 이동
            "add %[mid], %[mid], #1\n\t"               // 오른쪽 배열의 포인터 이동
            "b merge_loop\n\t"                         // 다음 반복

        // 왼쪽 배열이 비어있는 경우 오른쪽 배열의 요소를 임시 배열에 복사
        "copy_right:\n\t"
            "ldr r8, [%[arr], %[mid], lsl #2]\n\t"     // r8 = arr[mid]
            "str r8, [r9], #4\n\t"                     // 임시 배열에 저장 후 포인터 이동
            "add %[mid], %[mid], #1\n\t"               // 오른쪽 배열의 포인터 이동
            "b merge_loop\n\t"                         // 다음 반복

        // 오른쪽 배열이 비어있는 경우 왼쪽 배열의 요소를 임시 배열에 복사
        "copy_left:\n\t"
            "ldr r7, [%[arr], %[left], lsl #2]\n\t"    // r7 = arr[left]
            "str r7, [r9], #4\n\t"                     // 임시 배열에 저장 후 포인터 이동
            "add %[left], %[left], #1\n\t"             // 왼쪽 배열의 포인터 이동
            "b merge_loop\n\t"                         // 다음 반복

        // 함수 에필로그
        "pop {r4, r5, r6, r7, r9, pc}\n\t" // 레지스터 복원 및 리턴

        :
        : [arr] "r"(arr), [left] "m"(left), [mid] "m"(mid), [right] "m"(right)
        : "r7", "r8"
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
        // r0: arr / r1: left / r2: right
        
        "PUSH {r4, r5, r6, lr}\n\t" // 레지스터 보존
        
        // 배열 크기가 1 이하이면 종료
        "CMP r1, #1\n\t"
        "BLE end_sort\n\t"
        
        // 배열의 중간 인덱스 계산
        "ADD r4, r1, r2\n\t"       // r4 = left + right
        "LSR r4, r4, #1\n\t"       // r4 = (left + right) / 2
        
        // 배열의 왼쪽 부분을 병합 정렬
        "mov r5, r0\n\t"           // 배열의 시작 주소를 r5에 저장
        "mov r6, r4\n\t"           // 배열의 크기를 r6에 저장
        "bl mergesort_ASM\n\t"     // 재귀 호출
        
        // 배열의 오른쪽 부분을 병합 정렬
        "add r5, r5, r4\n\t" // 배열의 시작 주소를 중간으로 이동
        "sub r6, r1, r4\n\t"       // 배열의 크기를 계산 (r1 - r4)
        "bl mergesort_ASM\n\t"     // 재귀 호출
        
        // 배열 병합
        "mov r2, r0\n\t"           // r2에 왼쪽 배열의 시작 주소를 복사
        "mov r3, r5\n\t"           // r3에 오른쪽 배열의 시작 주소를 복사
        "bl merge_ASM\n\t"         // 병합 함수 호출
        
    "end_sort:"
        // 배열 크기가 1 이하이므로 정렬할 필요 없음
        "POP {r4, r5, r6, pc}\n\t" // 레지스터 복원 및 리턴
        );
}
