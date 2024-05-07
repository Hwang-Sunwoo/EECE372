#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define SWITCH_PIN 29  // 입력 스위치에 연결된 GPIO 핀
int SEGMENT_PINS[8] = {0, 7, 24, 23, 22, 2, 3, 25}  // 7-segment에 연결된 GPIO 핀들

void setting();

int main() {

    int count = 0;
    int btn_state = 0;
    int b;
    int sevseq[16][8]={
	{1,1,1,1,1,1,0,0},
	{0,1,1,0,0,0,0,0},
	{1,1,0,1,1,0,1,0},
	{1,1,1,1,0,0,1,0},
	{0,1,1,0,0,1,1,0},
	{1,0,1,1,0,1,1,0},
	{1,0,1,1,1,1,1,0},
	{1,1,1,0,0,1,0,0},
	{1,1,1,1,1,1,1,0},
	{1,1,1,1,0,1,1,0},
	{1,1,1,0,1,1,1,0},
	{0,0,1,1,1,1,1,0},
	{1,0,0,1,1,1,0,0},
	{0,1,1,1,1,0,1,0},
	{1,0,0,1,1,1,1,0},
	{1,0,0,0,1,1,1,0}
    };
    
    setting();
    
    
    while(1) {
        if(btn_state == 0){
            if(digitalRead(SWITCH_PINS) == 1){
                count++;
                count = count % 16;
                btn_state = 1;
            }
        }else if(btn_state == 1){
            if(digitalRead(SWITCH_PINS) == 0){
                btn_state = 0;
            }

        }

            for(b = 0; b < 8; b++){
                digitalWrite(SEGMENT_PINS[b], sevseq[count][b]);
            }
            delay(100);
        }
    }
    return 0;
}

void setting() {
    wiringPiSetupGpio();
    
    pinMode(SWITCH_PIN, INPUT); // 입력 스위치 설정

    for (int i = 0; i < 8; i++) { // 7-segment 설정
        pinMode(SEGMENT_PINS[i], OUTPUT);
    }
};
