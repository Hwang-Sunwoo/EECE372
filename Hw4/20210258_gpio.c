#include <wiringPi.h>
#include <stdio.h>

#define BTN 29
#define g 3
#define f 2
#define a 0
#define b 7
#define e 22
#define d 23
#define c 24
#define DP 25

int main(void) {
	int cnt = 0;

	if (wiringPiSetup() == -1) { //library include ˝ÇĆĐ˝Ă Ážˇá
		return 1;
	}
	pinMode(BTN, INPUT); //pinľé ¸đľĺźłÁ¤
	pinMode(g, OUTPUT);
	pinMode(f, OUTPUT);
	pinMode(a, OUTPUT);
	pinMode(b, OUTPUT);
	pinMode(e, OUTPUT);
	pinMode(d, OUTPUT);
	pinMode(c, OUTPUT);
	pinMode(DP, OUTPUT);

	digitalWrite(a, 1);
	digitalWrite(b, 1);
	digitalWrite(c, 1);
	digitalWrite(d, 1);
	digitalWrite(e, 1);
	digitalWrite(f, 1);
	digitalWrite(g, 0);
	digitalWrite(DP, 0);

	while (1) {
		if (digitalRead(BTN) == 1) {
			cnt = (cnt+1)%16;
			switch (cnt) {
			case 0:
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 0);
				digitalWrite(DP, 0);
				break;
			case 1:
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 0);
				digitalWrite(f, 0);
				digitalWrite(g, 0);
				digitalWrite(DP, 0);
				break;
			case 2:
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 3:
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 0);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 4:
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 5:
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 6:
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 7:
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 0);
				digitalWrite(DP, 0);
				break;
			case 8:
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 9:
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 10:
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 11:
				digitalWrite(a, 0);
				digitalWrite(b, 0);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 12:
				digitalWrite(a, 0);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 13:
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 14:
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			case 15:
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 0);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
			}
			delay(300);
		}
	}
}
