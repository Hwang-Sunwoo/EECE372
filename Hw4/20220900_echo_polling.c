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

int SEGMENT_PINS[8] = {0, 7, 24, 23, 22, 2, 3, 25};  // 7-segment에 연결된 GPIO 핀들

void task()
{
	int i;
	for(i=0; i<400000000; i++);
}

int main(){
	int fd;
	struct termios newtio;
	struct pollfd poll_handler;
	char buf[256];
    char row;
    int sevseq[17][8]={
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
            {1,0,0,0,1,1,1,0},
            {0,1,1,0,1,1,1,0}
        };
    
	fd = open("/dev/serial0", O_RDWR|O_NOCTTY);
	if(fd<0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	//	speed_t baudRate = B1000000;   //Use when there is a problem with Baudrate
	//	cfsetispeed(&newtio, baudRate);
	//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	poll_handler.fd = fd;
	poll_handler.events = POLLIN|POLLERR;
	poll_handler.revents = 0;

	write(fd, "Polling method\r\n", 16);

    wiringPiSetupGpio();

for (int i = 0; i < 8; i++) { // 7-segment 설정
	pinMode(SEGMENT_PINS[i], OUTPUT);
}
    
	while(1) {
		task();
		if(poll((struct pollfd*)&poll_handler, 1, 2000) > 0) {
			if(poll_handler.revents & POLLIN) {
				int cnt = read(fd, buf, sizeof(buf));
				buf[cnt] = '\0';
				write(fd, "echo: ", 6);
				write(fd, buf, cnt);
				write(fd, "\r\n", 2);
                		row = char(buf[0]);
                
                		if(48 <= row && row <= 57){
                   			for(int b = 0; b < 8; b++){
                       			digitalWrite(SEGMENT_PINS[b], sevseq[row - 48][b]);
                    		}
                		}else if(65 <= row && row <= 70){
                    
                    			for(int b = 0; b < 8; b++){
                        			digitalWrite(SEGMENT_PINS[b], sevseq[row - 55][b]);
                    			}
                		}else{
                    			for(int b = 0; b < 8; b++){
                        			digitalWrite(SEGMENT_PINS[b], sevseq[16][b]);
                    			}
                		}
			}
			else if(poll_handler.revents & POLLERR) {
				printf("Error in communication. Abort program\r\n");
				return 0;
			}
		}
	}
	return 0;
}
