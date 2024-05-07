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
#define g 24
#define f 23
#define a 22
#define b 21
#define e 29
#define d 25
#define c 28
#define DP 27

int fd;
char buf[256];

void callback_function(int status)
{
	int cnt = read(fd, buf, 256);
	buf[cnt] = '\0';
	write(fd, "echo: ", 6);
	write(fd, buf, cnt);
	write(fd, "\r\n", 2);
	switch(buf[0]){
					case '0':
						digitalWrite(a, 1);
						digitalWrite(b, 1);
						digitalWrite(c, 1);
						digitalWrite(d, 1);
						digitalWrite(e, 1);
						digitalWrite(f, 1);
						digitalWrite(g, 0);
						digitalWrite(DP, 0);
						break;
					case '1':
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 0);
				digitalWrite(f, 0);
				digitalWrite(g, 0);
				digitalWrite(DP, 0);
				break;
					case '2':
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case '3':
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 0);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case '4':
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case '5':
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case '6':
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case '7':
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 0);
				digitalWrite(DP, 0);
				break;
					case '8':
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case '9':
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 0);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'a':
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'b':
				digitalWrite(a, 0);
				digitalWrite(b, 0);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'c':
				digitalWrite(a, 0);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'd':
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'e':
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'f':
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 0);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'A':
				digitalWrite(a, 1);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'B':
				digitalWrite(a, 0);
				digitalWrite(b, 0);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'C':
				digitalWrite(a, 0);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'D':
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 0);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'E':
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 1);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					case 'F':
				digitalWrite(a, 1);
				digitalWrite(b, 0);
				digitalWrite(c, 0);
				digitalWrite(d, 0);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
					default:
				digitalWrite(a, 0);
				digitalWrite(b, 1);
				digitalWrite(c, 1);
				digitalWrite(d, 0);
				digitalWrite(e, 1);
				digitalWrite(f, 1);
				digitalWrite(g, 1);
				digitalWrite(DP, 0);
				break;
				}
	printf("Received: %s\r\n", buf);

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

	write(fd, "interrupt method\r\n", 18);
	if (wiringPiSetup() == -1) { //library include ½ÇÆÐ½Ã Á¾·á
		return 1;
	}
	
	pinMode(g, OUTPUT);
	pinMode(f, OUTPUT);
	pinMode(a, OUTPUT);
	pinMode(b, OUTPUT);
	pinMode(e, OUTPUT);
	pinMode(d, OUTPUT);
	pinMode(c, OUTPUT);
	pinMode(DP, OUTPUT);
	
	digitalWrite(a, 0);
	digitalWrite(b, 0);
	digitalWrite(c, 0);
	digitalWrite(d, 0);
	digitalWrite(e, 0);
	digitalWrite(f, 0);
	digitalWrite(g, 0);
	digitalWrite(DP, 0);
	while(1) {

		task();
	}
	return 0;
}
