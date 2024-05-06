#include <wiringPi.h>

#define SW 29
#define A 0
#define B 7
#define C 24
#define D 23
#define E 22
#define F 2
#define G 3
#define DP 25

int main(){
	int count = 0;
	int before_state = 0;
	
	if (wiringPiSetup() == -1){
		return 1;
	}
	
	pinMode(SW, INPUT);
	pinMode(A, OUTPUT);
	pinMode(B, OUTPUT);
	pinMode(C, OUTPUT);
	pinMode(D, OUTPUT);
	pinMode(E, OUTPUT);
	pinMode(F, OUTPUT);
	pinMode(G, OUTPUT);
	pinMode(DP, OUTPUT);

	while(1){
		if(before_state == 0) {
			if(digitalRead(SW) == 1){
				count++;
				count = count % 16;
				before_state = 1;
			}
		}
		else if(before_state == 1){
			if(digitalRead(SW) == 0){
				before_state = 0;
			}
		}
		
		switch(count){
			case 0:
				digitalWrite(A, 1);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 1);
				digitalWrite(G, 0);
				digitalWrite(DP, 0);
				break;
			case 1:
				digitalWrite(A, 0);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 0);
				digitalWrite(E, 0);
				digitalWrite(F, 0);
				digitalWrite(G, 0);
				digitalWrite(DP, 0);
				break;
			case 2:
				digitalWrite(A, 1);
				digitalWrite(B, 1);
				digitalWrite(C, 0);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 0);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 3:
				digitalWrite(A, 1);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 0);
				digitalWrite(F, 0);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 4:
				digitalWrite(A, 0);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 0);
				digitalWrite(E, 0);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 5:
				digitalWrite(A, 1);
				digitalWrite(B, 0);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 0);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 6:
				digitalWrite(A, 1);
				digitalWrite(B, 0);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 7:
				digitalWrite(A, 1);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 0);
				digitalWrite(E, 0);
				digitalWrite(F, 1);
				digitalWrite(G, 0);
				digitalWrite(DP, 0);
				break;
			case 8:
				digitalWrite(A, 1);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 9:
				digitalWrite(A, 1);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 0);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 10:
				digitalWrite(A, 1);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 0);
				digitalWrite(E, 1);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 11:
				digitalWrite(A, 0);
				digitalWrite(B, 0);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 12:
				digitalWrite(A, 0);
				digitalWrite(B, 0);
				digitalWrite(C, 0);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 0);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 13:
				digitalWrite(A, 0);
				digitalWrite(B, 1);
				digitalWrite(C, 1);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 0);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 14:
				digitalWrite(A, 1);
				digitalWrite(B, 0);
				digitalWrite(C, 0);
				digitalWrite(D, 1);
				digitalWrite(E, 1);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
			case 15:
				digitalWrite(A, 1);
				digitalWrite(B, 0);
				digitalWrite(C, 0);
				digitalWrite(D, 0);
				digitalWrite(E, 1);
				digitalWrite(F, 1);
				digitalWrite(G, 1);
				digitalWrite(DP, 0);
				break;
		}
			
		delay(100);
	}
}


#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define SWITCH_PIN 40  // 입력 스위치에 연결된 GPIO 핀
#define SEGMENT_PINS {11, 7, 23, 21, 19, 13, 15, 29}  // 7-segment에 연결된 GPIO 핀들
#define NUM_SEGMENTS 8

int segmentValues[] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // B
    0b00111001, // C
    0b01011110, // D
    0b01111001, // E
    0b01110001  // F
};

int currentNumber = 0; // 현재 7-segment에 표시되는 숫자

void setup() {
    wiringPiSetupGpio();
    
    // 입력 스위치 설정
    pinMode(SWITCH_PIN, INPUT);
    pullUpDnControl(SWITCH_PIN, PUD_UP);
    
    // 7-segment 설정
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        pinMode(SEGMENT_PINS[i], OUTPUT);
        digitalWrite(SEGMENT_PINS[i], LOW);
    }
}

void displayNumber(int number) {
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        digitalWrite(SEGMENT_PINS[i], (segmentValues[number] >> i) & 1);
    }
}

void loop() {
    int switchState = digitalRead(SWITCH_PIN);
    if (switchState == LOW) {
        delay(200); // 디바운싱을 위한 딜레이
        
        // 스위치 눌림 감지
        currentNumber++;
        if (currentNumber > 15) {
            currentNumber = 0; // 0부터 다시 시작
        }
        displayNumber(currentNumber);
        
        // 눌림 감지 후 스위치 놓을 때까지 기다림
        while (digitalRead(SWITCH_PIN) == LOW) {
            delay(50);
        }
    }
}

int main() {
    setup();
    while(1) {
        loop();
    }
    return 0;
}
