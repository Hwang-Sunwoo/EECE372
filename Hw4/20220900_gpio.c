#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define SWITCH_PIN 40  // 입력 스위치에 연결된 GPIO 핀
int SEGMENT_PINS[8] = {11, 7, 35, 33, 31, 13, 15, 37}  // 7-segment에 연결된 GPIO 핀들
#define NUM_SEGMENTS 8
void setting();

int main() {

    int count = 0;
    int btn_state;
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
        btn_state = digitalRead(SWITCH_PIN);
        if (btn_state == HIGH) {
            if (count > 15) {
                count = 0;
            }
            for(b = 0; b < 8; b++){
                digitalWrite(SEGMENT_PINS[b], sevseq[count][b]);
            }
            count++;
            delay(1000);
        }
    }
    return 0;
}

void setting() {
    wiringPiSetupGpio();
    
    // 입력 스위치 설정
    pinMode(SWITCH_PIN, INPUT);
    digitalWrite(SWITCH_PIN, HIGH);

    // 7-segment 설정
    int segment_pins[NUM_SEGMENTS] = SEGMENT_PINS;
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        pinMode(segment_pins[i], OUTPUT);
    }
};
