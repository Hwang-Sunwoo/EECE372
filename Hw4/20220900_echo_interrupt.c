#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <wiringPi.h>

#define BAUDRATE B1000000
int SEGMENT_PINS[8] = {0, 7, 24, 23, 22, 2, 3, 25};
int fd;
char buf[256];
char row;

void callback_function(int status)
{
	int cnt = read(fd, buf, 256);
	buf[cnt] = '\0';
	write(fd, "echo: ", 6);
	write(fd, buf, cnt);
	write(fd, "\r\n", 2);
	printf("Received: %s\r\n", buf);
	row = buf[0];
}

void task()
{
	int i;
	for(i=0; i<400000000; i++);
}

int main()
{
	struct termios newtio;
	struct sigaction saio;
	int sev_led[17][8]={
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

	memset(&saio, 0, sizeof(saio));	
	saio.sa_handler = callback_function;
	saio.sa_restorer = NULL;
	sigaction(SIGIO, &saio, NULL);

	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, FASYNC);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	//	speed_t baudRate = B1000000;      //Use when there is a problem with Baudrate
	//	cfsetispeed(&newtio, baudRate);
	//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

 wiringPiSetupGpio();

for (int i = 0; i < 8; i++) { // 7-segment 설정
	pinMode(SEGMENT_PINS[i], OUTPUT);
}
	
	write(fd, "interrupt method\r\n", 18);
	while(1) {

		task();
		if('0' <= count && count <= '9'){
			for(int i = 0; i < 8; i++)
				digitalWrite(SEGMENT_PINS[i], sev_led[row - 48][i]);
		}
		else if('A' <= count && count <= 'F'){
			for(int i = 0; i < 8; i++)
				digitalWrite(SEGMENT_PINS[i], sev_led[row - 55][i]);
		}
		else{
			for(int i = 0; i < 8; i++)
				digitalWrite(SEGMENT_PINS[i], sev_led[16][i]);
		}
	}
	return 0;
}
