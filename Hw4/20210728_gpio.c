#include<stdio.h>
#include<time.h>
#include<wiringPi.h>
#include<stdlib.h>
#include<signal.h>

#define pin_btn 17

int pins[8]={2,3,4,5,6,7,8,9};

void clear(){
	int b;
	for(b=0;b<8;b++){
		digitalWrite(pins[b],LOW);
	}
	exit(0);
}

int main(){

	int count=0;
	int btn_state;

	signal(SIGINT,clear);
	
	wiringPiSetupGpio();
	

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
	
		
	for(int i=0;i<8;i++)
		pinMode(pins[i],OUTPUT);
	pinMode(pin_btn,INPUT);
	digitalWrite(pin_btn,HIGH);	


	while(1){
	
		btn_state=digitalRead(pin_btn);
		if(btn_state==HIGH){
		
			if(count>15){
				count=0;
			}
			for(int b=0;b<8;b++)
				digitalWrite(pins[b],sevseq[count][b]);
			count++;
			delay(1000);
		}
	}
	return 0;
}
