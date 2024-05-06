#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define SWITCH_PIN 40  // 입력 스위치에 연결된 GPIO 핀
#define SEGMENT_PINS {11, 7, 35, 33, 31, 13, 15, 37}  // 7-segment에 연결된 GPIO 핀들
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

void setup() {
    wiringPiSetupGpio();
    
    // 입력 스위치 설정
    pinMode(SWITCH_PIN, INPUT);
    pullUpDnControl(SWITCH_PIN, PUD_UP);
    
    // 7-segment 설정
    int segment_pins[NUM_SEGMENTS] = SEGMENT_PINS;
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        pinMode(segment_pins[i], OUTPUT);
        digitalWrite(segment_pins[i], LOW);
    }
}

void displayNumber(int number) {
    int segment_pins[NUM_SEGMENTS] = SEGMENT_PINS;
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        digitalWrite(segment_pins[i], (segmentValues[number] >> i) & 1);
    }
}

int main() {
    setup();
    
    int count = 0;
    int btn_state;
    
    while(1) {
        btn_state = digitalRead(SWITCH_PIN);
        if (btn_state == HIGH) {
            if (count > 15) {
                count = 0;
            }
            displayNumber(count);
            count++;
            delay(1000);
        }
    }
    return 0;
}
