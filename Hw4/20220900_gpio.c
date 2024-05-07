#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define SWITCH_PIN 29  // switch to GPIO
int SEGMENT_PINS[8] = {0, 7, 3, 22, 23, 24, 25, 2};// 7-segment to GPIO
int main(){
    int row = 0; // row vraible
    int btn_state = 0; // state variable
    int sev_seg[16][8]={ // printing 1 to F using 7-segment using double array
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
	{0,0,0,1,1,0,1,0},
	{0,1,1,1,1,0,1,0},
	{1,0,0,1,1,1,1,0},
	{1,0,0,0,1,1,1,0}
    };
    if (wiringPiSetup() == -1){ // initializing wiringPi
	return 1;
    }
	
    pinMode(SWITCH_PIN, INPUT); //setting switch as an input
   for(int i = 0; i < 8; i++){ // setting 7-segment as an output
	pinMode(SEGMENT_PINS[i], OUTPUT);
   }
	
    while(1){
	if(btn_state == 0) { // implementing debouncing
		if(digitalRead(SWITCH_PIN) == HIGH){ // when switch is closed, stop changing number
			row++;
			row = row % 16;
			btn_state = 1;
		}
	}
	else if(btn_state == 1){ // when switch is opened, preparing to recieve the input
		if(digitalRead(SWITCH_PIN) == LOW){
			btn_state = 0;
		}
	}
	for(int i = 0; i < 8; i++){ // printing sev_seg[][]
    		digitalWrite(SEGMENT_PINS[i], sev_seg[row][i]);
	}
	delay(100);
    }
}
