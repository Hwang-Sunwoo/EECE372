#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define SWITCH_PIN 29  // 입력 스위치에 연결된 GPIO 핀
int SEGMENT_PINS[8] = {0, 7, 24, 23, 22, 2, 3, 25};  // 7-segment에 연결된 GPIO 핀들

int main(){
    int row = 0;
    int btn_state = 0;
    int sev_seg[16][8]={
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
    if (wiringPiSetup() == -1){
	return 1;
    }
	
    pinMode(SWITCH_PIN, INPUT);
   for(int i = 0; i < 8; i++){
	pinMode(SEGMENT_PINS[i], OUTPUT);
   }
	
    while(1){
	if(before_state == 0) {
		if(digitalRead(SW) == 1){
			row++;
			row = row % 16;
			btn_state = 1;
		}
	}
	else if(before_state == 1){
		if(digitalRead(SW) == 0){
			btn_state = 0;
		}
	}
	for(b = 0; b < 8; b++){
    		digitalWrite(SEGMENT_PINS[b], sev_seg[row][b]);
	}
	delay(100);
    }
}
