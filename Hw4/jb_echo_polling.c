#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <wiringPi.h>

#define BAUDRATE B1000000

int pin_num[] = {0, 7, 24, 23, 22, 2, 3}; // GPIO pins based on wiringPi numbering

void updateLEDs(char firstChar) {
    const int PIN_COUNT = 7;
    int hex_table[16][7] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 0, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 0, 0, 1}, // 3
        {0, 1, 1, 0, 0, 1, 1}, // 4
        {1, 0, 1, 1, 0, 1, 1}, // 5
        {1, 0, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 0, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}, // 9
        {1, 1, 1, 0, 1, 1, 1}, // A
        {0, 0, 1, 1, 1, 1, 1}, // B
        {0, 0, 0, 1, 1, 0, 1}, // C
        {0, 1, 1, 1, 1, 0, 1}, // D
        {1, 0, 0, 1, 1, 1, 1}, // E
        {1, 0, 0, 0, 1, 1, 1}  // F
    };

    int index = (firstChar >= '0' && firstChar <= '9') ? firstChar - '0' :
                (firstChar >= 'A' && firstChar <= 'F') ? firstChar - 'A' + 10 : -1;

    if (index != -1) {
        for (int i = 0; i < PIN_COUNT; i++) {
            digitalWrite(pin_num[i], hex_table[index][i]);
        }
    } else {
        // Display 'X' for invalid input
        digitalWrite(29, 0);
        digitalWrite(28, 1);
        digitalWrite(23, 1);
        digitalWrite(22, 0);
        digitalWrite(21, 1);
        digitalWrite(27, 1);
        digitalWrite(26, 1);
    }
}

void task() {
    int i;
    for (i = 0; i < 400000000; i++);
}

int main() {
    wiringPiSetup();
    for (int i = 0; i < 7; i++) {
        pinMode(pin_num[i], OUTPUT);
    }

    int fd;
    struct termios newtio;
    struct pollfd poll_handler;
    char buf[256];

    fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open port: %s.\r\n", strerror(errno));
        return -1; // Exit if device error
    }

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcsetattr(fd, TCSANOW, &newtio);
    tcflush(fd, TCIFLUSH);

    poll_handler.fd = fd;
    poll_handler.events = POLLIN | POLLERR;
    poll_handler.revents = 0;

    printf("Polling method active. Waiting for input...\n");

    while (1) {
        task();
        if (poll(&poll_handler, 1, 2000) > 0) {
            if (poll_handler.revents & POLLIN) {
                int cnt = read(fd, buf, sizeof(buf));
                if (cnt > 0) {
                    buf[cnt] = '\0';
                    printf("Received: %s\n", buf);
                    updateLEDs(buf[0]);
                }
            }
            if (poll_handler.revents & POLLERR) {
                printf("Error in communication. Abort program\r\n");
                break;
            }
        }
    }

    close(fd);
    return 0;
}
