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

void merge_ASM(int arr[], int left, int mid, int right) {
    
    int *LA;
    int *RA;
    LA = (int*)malloc(sizeof(int) * (mid - left + 1));
    RA = (int*)malloc(sizeof(int) * (right - mid));
    // r4: i / r5: j / r6: k
    // LA: left array / RA: right array
    // r8 / r9
    asm(
        "MOV r4, #0\n\t" //i = 0
        "MOV r5, #0\n\t" //j = 0

        "left_array:\n\t"//for(i = 0; i < a; i++){
        "SUB r8, %[mid], %[left]\n\t"
        "ADD r8, r8, #1\n\t" //a = mid - left + 1
        "CMP r4, r8\n\t" //i < a
        "BGE right_array\n\t" //for(j = 0; j < b; j++){
        "ADD r8, %[left], r4\n\t" //left + i
        "LDR r9, [%[arr], r8, LSL #2]\n\t" //arr[left + i]
        "STR r9, [%[LA], r4, LSL #2]\n\t" //L[i] = arr[left + i]
        "ADD r4, #1\n\t" //i++
        "B left_array\n\t" //continue
        
        "right_array:\n\t" //for(j = 0; j < b; j++){
        "SUB r8, %[right], %[mid]\n\t" //b = right - mid
        "CMP r5, r8\n\t" //j < b
        "BGE prepare_merge\n\t" //while(i < a && j < b){
        "ADD r9, %[mid], #1\n\t" //mid + 1
        "ADD r9, r9, r5\n\t" //mid + 1 + j
        "LDR r9, [%[arr], r8, LSL #2]\n\t" //arr[mid + 1 + j]
        "STR r9, [%[RA], r5, LSL #2]\n\t" //R[j] = arr[mid + 1+j]
        "ADD r5, r5, #1\n\t" //j++
        "B right_array\n\t" //continue

        /*
        "prepare_merge:\n\t"
        "MOV r4, #0\n\t" //i = 0
        "MOV r5, #0\n\t" //j = 0
        "MOV r6, %[left]\n\t" //k = left
        
        "merge_loop:\n\t" //while(i < a && j < b){
            "SUB r8, %[mid], %[left]\n\t"
            "ADD r8, r8, #1\n\t" //getting a
            "CMP r4, r8\n\t" //i < a
            "BGE compare_right\n\t" //go while(j < b){
            "SUB r8, %[right], %[mid]\n\t" //getting b
            "CMP r5, r8\n\t" //j < b
            "BGE compare_left\n\t" //go while(i < a)
        
            "LDR r8, [%[LA], r4, LSL #2]\n\t" //L[i]
            "LDR r9, [%[RA], r5, LSL #2]\n\t" //R[j]
            "CMP r8, r9\n\t" //L[i] <= R[j]
            "BLE copy_left\n\t"
            "B copy_right\n\t"
            
            "B end_merge\n\t"
        
        "update_k:\n\t"
        "CMP r6, %[right]\n\t" //k <= right
        "BGE end_merge\n\t"
        "ADD r6, r6, #1\n\t" //k++
        "B merge_loop\n\t" //continue
        
        "compare_left:\n\t" //while(i < a){
        "SUB r8, %[mid], %[left]\n\t"
        "ADD r8, r8, #1\n\t" //getting a
        "CMP r4, r8\n\t" //i < a
        "BGE compare_right\n\t" //while(j < b)
        "B copy_left\n\t"
        
        "compare_right:\n\t" //while(j < b)
        "SUB r8, %[right], %[mid]\n\t" //getting b
        "CMP r5, r8\n\t" //j < b
        "BGE end_merge\n\t" //return
        "B copy_right\n\t"
        
        
        "copy_left:\n\t"
        "LDR r8, [%[LA], r4, LSL #2]\n\t" //L[i]
        "STR r8, [%[arr], r6, LSL #2]\n\t" //arr[k] = L[i]
        "ADD r4, r4, #1\n\t" //i++
        "B update_k\n\t"
        
        "copy_right:\n\t"
        "LDR r8, [%[RA], r5, LSL #2]\n\t" //R[j]
        "STR r8, [%[arr], r6, LSL #2]\n\t" //arr[k] = R[j]
        "ADD r5, r5, #1\n\t" //j++
        "B update_k\n\t"
        */
        //가짜
        "prepare_merge:\n\t"
        "MOV r4, #0\n\t" //i = 0
        "MOV r5, #0\n\t" //j = 0
        "MOV r6, %[left]\n\t" //k = left
        
        "copy_left:\n\t"
        "SUB r8, %[mid], %[left]\n\t"
        "ADD r8, r8, #1\n\t" //getting a
        "CMP r4, r8\n\t" //i < a
        "BGE copy_right\n\t"
        "LDR r8, [%[LA], r4, LSL #2]\n\t" //L[i]
        "STR r8, [%[arr], r6, LSL #2]\n\t" //arr[k] = L[i]
        "ADD r4, r4, #1\n\t" //i++
        "B copy_left\n\t"
        
        "copy_right:\n\t"
        "SUB r8, %[right], %[mid]\n\t" //getting b
        "CMP r5, r8\n\t" //j < b
        "BGE end_merge\n\t" //return
        "LDR r8, [%[RA], r5, LSL #2]\n\t" //R[j]
        "STR r8, [%[arr], r6, LSL #2]\n\t" //arr[k] = R[j]
        "ADD r5, r5, #1\n\t" //j++
        "B copy_right\n\t"
        
        "end_merge:\n\t"
        :
        : [arr] "r"(arr), [left] "r"(left), [mid] "r"(mid), [right] "r"(right), [LA] "r"(LA), [RA] "r"(RA)
        : "r4", "r5", "r6", "r8", "r9"
    );
    free(LA);
    free(RA);
    return;
}
/*
void merge_ASM(int arr[], int left, int mid, int right) {
    int leftIndex, rightIndex, tempIndex;
    int* temp;
    temp = (int*)malloc(sizeof(int) * (right - left + 1)); // 임시 배열을 위한 메모리 할당
    int n;

    asm (
        // 초기 레지스터 설정
        "mov %[n], %[right]\n\t"
        "sub %[n], %[left]\n\t"
        "add %[n], #1\n\t"                 // n을 high-low+1으로 초기화
        "mov %[li], %[left]\n\t"            // leftIndex를 low 값으로 초기화
        "mov %[ri], %[mid]\n\t"            // rightIndex를 mid 값으로 초기화
        "add %[ri], %[ri], #1\n\t"         // rightIndex를 mid+1로 설정하여 오른쪽 부분 배열의 시작점으로 설정
        "mov %[ti], #0\n\t"                // tempIndex를 0으로 초기화

        // 병합 루프 시작
        "loop_merge:\n\t"
        "cmp %[li], %[mid]\n\t"            // leftIndex와 mid 비교
        "bgt left_done\n\t"                 // leftIndex가 mid보다 크면 left 부분 배열이 끝났음을 의미, left_done으로 점프
        "cmp %[ri], %[right]\n\t"           // rightIndex와 high 비교
        "bgt right_done\n\t"                // rightIndex가 high보다 크면 right 부분 배열이 끝났음을 의미, right_done으로 점프

        // 두 부분 배열의 현재 요소를 로드
        "ldr r5, [%[arr], %[li], LSL #2]\n\t"  // a[leftIndex]의 값을 r5에 로드
        "ldr r6, [%[arr], %[ri], LSL #2]\n\t"  // a[rightIndex]의 값을 r6에 로드

        // 비교 및 temp에 저장
        "cmp r5, r6\n\t"
        "bgt copy_right\n\t"                // r5 > r6 이면 오른쪽 요소를 temp에 복사

        "copy_left:\n\t"
        "str r5, [%[temp], %[ti], LSL #2]\n\t"  // temp[tempIndex]에 a[leftIndex]값 저장
        "add %[li], %[li], #1\n\t"         // leftIndex 증가
        "b increment_temp\n\t"             // tempIndex 증가로 점프

        "copy_right:\n\t"
        "str r6, [%[temp], %[ti], LSL #2]\n\t"  // temp[tempIndex]에 a[rightIndex]값 저장
        "add %[ri], %[ri], #1\n\t"         // rightIndex 증가

        "increment_temp:\n\t"
        "add %[ti], %[ti], #1\n\t"         // tempIndex 증가
        "b loop_merge\n\t"                 // 병합 루프로 돌아가기

        "left_done:\n\t"
        "right_done:\n\t"
        "check_left:\n\t"
        // 남은 왼쪽 부분 배열 요소를 temp에 복사
        // leftIndex가 mid 보다 크지 않은 경우 (즉, 아직 왼쪽 부분 배열에 요소가 남아있는 경우) 계속 진행
        "cmp %[li], %[mid]\n\t"
        "bgt end_left\n\t"           // rightIndex가 high보다 크면 루프를 종료하고 merge 작업을 마무리
        "ldr r5, [%[arr], %[li], LSL #2]\n\t" // 업데이트된 a[leftIndex]의 값을 r5에 로드
        "str r5, [%[temp], %[ti], LSL #2]\n\t" // r5 레지스터의 값을 temp[tempIndex]에 저장
        "add %[li], %[li], #1\n\t"       // leftIndex 증가
        "add %[ti], %[ti], #1\n\t"       // tempIndex 증가
        "b check_left\n\t"                 // 다시 check_left 레이블로 점프하여 남은 오른쪽 요소를 계속 복사

        "end_left:\n\t"
        "check_right:\n\t"
        // 남은 오른쪽 부분 배열 요소를 temp에 복사
        // rightIndex가 high 보다 크지 않은 경우 (즉, 아직 오른쪽 부분 배열에 요소가 남아있는 경우) 계속 진행
        "cmp %[ri], %[right]\n\t"
        "bgt finish_merge\n\t"           // leftIndex가 mid보다 크면 루프를 종료하고 merge 작업을 마무리
        "ldr r6, [%[arr], %[ri], LSL #2]\n\t"  // 업데이트된 a[rightIndex]의 값을 r6에 로드
        "str r6, [%[temp], %[ti], LSL #2]\n\t" // r6 레지스터의 값을 temp[tempIndex]에 저장
        "add %[ri], %[ri], #1\n\t"       // rightIndex 증가
        "add %[ti], %[ti], #1\n\t"       // tempIndex 증가
        "b check_right\n\t"                // 다시 check_right 레이블로 점프하여 남은 왼쪽 요소를 계속 복사

        "finish_merge:\n\t"
        // temp의 내용을 원래의 배열 a에 복사
        "mov %[ti], #0\n\t"              // tempIndex를 0으로 초기화
        "copy_back_loop:\n\t"
        "cmp %[ti], %[n]\n\t"            // tempIndex와 n을 비교
        "bge end_copy_back\n\t"          // tempIndex가 n 이상이면 모든 요소를 복사했음을 의미하고, 복사 루프를 종료
        "ldr r5, [%[temp], %[ti], LSL #2]\n\t" // temp[tempIndex]에서 요소를 r5 레지스터로 로드
        "str r5, [%[arr], %[left], LSL #2]\n\t" // r5 레지스터의 값을 a[low + tempIndex]에 저장
        "add %[left], %[left], #1\n\t"     // low 값을 증가시키며 배열 인덱스를 조정 (여기서 실수했음)
        "add %[ti], %[ti], #1\n\t"       // tempIndex 증가
        "b copy_back_loop\n\t"           // 다시 copy_back_loop 레이블로 점프하여 나머지 요소를 계속 복사

        "end_copy_back:\n\t"
        :
        : [arr] "r" (arr), [left] "r" (left), [mid] "r" (mid), [right] "r" (right),
        [li] "r" (leftIndex), [ri] "r" (rightIndex), [ti] "r" (tempIndex), [temp] "r" (temp), [n] "r" (n)
        : "r5", "r6", "cc", "memory"
        );

    free(temp); // 임시 배열 해제
}*/
/*
void merge_ASM(int arr[], int left, int mid, int right) {
    int tempIndex;
    int *LA;
    int *RA;
    LA = (int*)malloc(sizeof(int) * mid - left +1);
    RA = (int* )malloc(sizeof(int) * right - mid);
    // r5, r6 / r8: a, b, leftindex, rightindex, etc / r9: etc
    asm (
         "MOV r5, #0\n\t" //i = 0
         "MOV r6, #0\n\t" //j = 0
         
         "left_array:\n\t"//for(i = 0; i < a; i++){
         "SUB r8, %[mid], %[left]\n\t"
         "ADD r8, r8, #1\n\t" //a = mid - left + 1
         "CMP r5, r8\n\t" //i < a
         "BGE right_array\n\t" //for(j = 0; j < b; j++){
         "ADD r8, %[left], r5\n\t" //left + i
         "LDR r9, [%[arr], r8, LSL #2]\n\t" //arr[left + i]
         "STR r9, [%[LA], r5, LSL #2]\n\t" //L[i] = arr[left + i]
         "ADD r5, #1\n\t" //i++
         "B left_array\n\t" //continue
         
         "right_array:\n\t" //for(j = 0; j < b; j++){
         "SUB r8, %[right], %[mid]\n\t" //b = right - mid
         "CMP r6, r8\n\t" //j < b
         "BGE prepare_loop\n\t" //while(i < a && j < b){
         "ADD r8, %[mid], #1\n\t" //mid + 1
         "ADD r8, r8, r6\n\t" //mid + 1 + j
         "LDR r9, [%[arr], r8, LSL #2]\n\t" //arr[mid + 1 + j]
         "STR r9, [%[RA], r6, LSL #2]\n\t" //R[j] = arr[mid + 1+j]
         "ADD r6, r6, #1\n\t" //j++
         "B right_array\n\t" //continue
         
         "prepare_loop:\n\t"
         "MOV r5, #0\n\t" //i = 0
         "MOV r6, #0\n\t" //j = 0
         "MOV %[ti], %[left]\n\t" //index = left
         
         "merge_loop:\n\t"
         "SUB r8, %[mid], %[left]\n\t"
         "ADD r8, r8, #1\n\t" //a = mid - left + 1
         "CMP r5, r8\n\t" //i < a            // leftIndex와 mid 비교
         "BGT left_done\n\t"                 // leftIndex가 mid보다 크면 left 부분 배열이 끝났음을 의미, left_done으로 점프
         "SUB r8, %[right], %[mid]\n\t" //b = right - mid
         "CMP r6, r8\n\t" //j < b           // rightIndex와 high 비교
         "BGT right_done\n\t"                // rightIndex가 high보다 크면 right 부분 배열이 끝났음을 의미, right_done으로 점프
         
         // 두 부분 배열의 현재 요소를 로드
         "LDR r8, [%[LA], r5, LSL #2]\n\t"  // a[leftIndex]의 값을 r5에 로드
         "LDR r9, [%[RA], r6, LSL #2]\n\t"  // a[rightIndex]의 값을 r6에 로드
         
         // 비교 및 temp에 저장
         "CMP r8, r9\n\t"
         "BGT copy_right\n\t"                // r5 > r6 이면 오른쪽 요소를 temp에 복사
         
         "copy_left:\n\t"
         "STR r8, [%[arr], %[ti], LSL #2]\n\t"  // temp[tempIndex]에 a[leftIndex]값 저장
         "ADD r8, r5, #1\n\t"         // leftIndex 증가
         "B increment_temp\n\t"             // tempIndex 증가로 점프
         
         "copy_right:\n\t"
         "STR r9, [%[arr], %[ti], LSL #2]\n\t"  // temp[tempIndex]에 a[rightIndex]값 저장
         "ADD r9, r6, #1\n\t"         // rightIndex 증가
         
         "increment_temp:\n\t"
         "ADD %[ti], %[ti], #1\n\t"         // tempIndex 증가
         "B merge_loop\n\t"                 // 병합 루프로 돌아가기
         
         "left_done:\n\t"
         "right_done:\n\t"
         "check_left:\n\t"
         // 남은 왼쪽 부분 배열 요소를 temp에 복사
         // leftIndex가 mid 보다 크지 않은 경우 (즉, 아직 왼쪽 부분 배열에 요소가 남아있는 경우) 계속 진행
         "SUB r8, %[mid], %[left]\n\t"
         "ADD r8, r8, #1\n\t" //a = mid - left + 1
         "CMP r5, r8\n\t" //i < a
         "BGT end_left\n\t"           // rightIndex가 high보다 크면 루프를 종료하고 merge 작업을 마무리
         "LDR r8, [%[LA], r5, LSL #2]\n\t" // 업데이트된 a[leftIndex]의 값을 r5에 로드
         "STR r8, [%[arr], %[ti], LSL #2]\n\t" // r5 레지스터의 값을 temp[tempIndex]에 저장
         "ADD r5, r5, #1\n\t"       // leftIndex 증가
         "ADD %[ti], %[ti], #1\n\t"       // tempIndex 증가
         "B check_left\n\t"                 // 다시 check_left 레이블로 점프하여 남은 오른쪽 요소를 계속 복사
         
         "end_left:\n\t"
         "check_right:\n\t"
         // 남은 오른쪽 부분 배열 요소를 temp에 복사
         // rightIndex가 high 보다 크지 않은 경우 (즉, 아직 오른쪽 부분 배열에 요소가 남아있는 경우) 계속 진행
         "SUB r8, %[right], %[mid]\n\t" //b = right - mid
         "CMP r6, r8\n\t" //j < b
         "BGT end_merge\n\t"           // leftIndex가 mid보다 크면 루프를 종료하고 merge 작업을 마무리
         "LDR r8, [%[RA], r6, LSL #2]\n\t"  // 업데이트된 a[rightIndex]의 값을 r6에 로드
         "STR r8, [%[arr], %[ti], LSL #2]\n\t" // r6 레지스터의 값을 temp[tempIndex]에 저장
         "ADD r6, r6, #1\n\t"       // rightIndex 증가
         "ADD %[ti], %[ti], #1\n\t"       // tempIndex 증가
         "B check_right\n\t"                // 다시 check_right 레이블로 점프하여 남은 왼쪽 요소를 계속 복사
         
         "end_merge:\n\t"
         // temp의 내용을 원래의 배열 a에 복사
         
         :
         : [arr] "r" (arr), [left] "r" (left), [mid] "r" (mid), [right] "r" (right),
         [ti] "r" (tempIndex), [RA] "r" (RA), [LA] "r"(LA)
         : "r5", "r6", "r8", "r9", "cc", "memory"
         );
    
    free(LA);
    free(RA);
    return;
}
*/
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
void mergesort_ASM(int arr[], int left, int right) {
    // r0: arr/ r1: left/ r2: right/ r3: mid/ r4: temp/
    asm(
        // Initialize registers r1, r2 only if they are not set (can use a flag or check if r1, r2 are zero)
        "CMP r1, #0\n\t"
        "CMPEQ r2, #0\n\t"
        "MOVEQ r1, %[left]\n\t"               // Set r4 to low if not already set
        "MOVEQ r2, %[right]\n\t"               // Set r5 to high if not already set
        
        // if(left < right)
        "MOV r0, %[arr]\n\t"                 // 배열 포인터 a를 r0에 설정
        "CMP r1, r2\n\t"               // 비교: low >= high
        "BGE end_sort\n\t"            // 만약 low >= high 이면, 재귀의 베이스 케이스에 도달했으므로 end_mergesort로 분기

        // mid = left + (right - left) / 2
        "SUB r3, r2, r1\n\t"           // r3 = high - low, 배열 길이 계산
        "LSR r3, r3, #1\n\t"               // r3 = (high - low) / 2, 오른쪽으로 한 비트 시프트하여 2로 나눔
        "ADD r3, r1, r3\n\t"             // r3 = low + (high - low) / 2, 중간 인덱스 계산

        // mergesort_ASM(arr, left, mid)
        "PUSH {r0-r3, lr}\n\t"             // r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 스택에 저장
        "MOV r2, r3\n\t"                   // 중간 인덱스 mid를 r2에 설정
        "BL mergesort_ASM\n\t"             // mergesort_ASM 함수 호출
        "POP {r0-r3, lr}\n\t"              // 스택에서 r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 복구

        // mergesort_ASM(arr, mid + 1, right)
        "PUSH {r0-r3, lr}\n\t"             // r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 스택에 저장
        "MOV r1, r3\n\t"                   // 중간 인덱스 mid를 r1에 다시 설정
        "ADD r1, r1, #1\n\t"               // r1 = mid + 1, 오른쪽 부분 배열의 시작 인덱스 설정
        "MOV r2, %[right]\n\t"                 // 종료 인덱스 high를 r2에 설정
        "BL mergesort_ASM\n\t"             // mergesort_ASM 함수 호출
        "POP {r0-r3, lr}\n\t"              // 스택에서 r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 복구

        // merge_ASM(arr, left, mid, right)
        "PUSH {r0-r3, lr}\n\t"             // r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 스택에 저장
        "MOV r4, r2\n\t"                   // 종료 인덱스 high를 temp로 r4에 잠시 저장
        "MOV r2, r3\n\t"                   // 중간 인덱스 mid를 r2에 설정
        "MOV r3, r4\n\t"                   // 종료 인덱스 high를 r3에 설정
        "BL merge_ASM\n\t"                 // merge_ASM 함수 호출
        "POP {r0-r3, lr}\n\t"              // 스택에서 r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 복구

        "end_sort:\n\t"               // 재귀의 베이스 케이스 및 함수 종료 지점 레이블
        :
        : [arr] "r" (arr), [left] "r" (left), [right] "r" (right)  // 입력: 배열 포인터, 시작 인덱스, 종료 인덱스
        : "r0", "r1", "r2", "r3", "r4", "lr", "memory", "cc"  // clobbered: 사용된 레지스터와 메모리, 조건 코드
        );
}
