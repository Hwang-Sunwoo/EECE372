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

int pins[8]={2,3,4,5,6,7,8,9};

void task()
{
	int i;
	for(i=0; i<400000000; i++);
}

int main()
{	

	wiringPiSetupGpio();

	for(int i=0;i<8;i++){
		pinMode(pins[i],OUTPUT);
	}

	int fd;
	struct termios newtio;
	struct pollfd poll_handler;
	char buf[256];

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
	char count;

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
	while(1) {
		task();
		if(poll((struct pollfd*)&poll_handler, 1, 2000) > 0) {
			if(poll_handler.revents & POLLIN) {
				int cnt = read(fd, buf, sizeof(buf));
				buf[cnt] = '\0';
				write(fd, "echo: ", 6);
				write(fd, buf, cnt);
				write(fd, "\r\n", 2);
				count=buf[0];
			}
			else if(poll_handler.revents & POLLERR) {
				printf("Error in communication. Abort program\r\n");
				return 0;
			}
		}
		if(48<=count && count<=57){
			for(int i=0;i<8;i++)
				digitalWrite(pins[i],sevseq[count-48][i]);
		}
		else if(65<=count && count<=70){
			for(int i=0;i<8;i++)
				digitalWrite(pins[i],sevseq[count-55][i]);
		}
		else{
			for(int i=0;i<8;i++)
				digitalWrite(pins[i],sevseq[16][i]);
		}

	}

	return 0;
}
