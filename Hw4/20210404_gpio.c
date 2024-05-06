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
