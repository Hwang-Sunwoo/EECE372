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

#define A 0
#define B 7
#define C 24
#define D 23
#define E 22
#define F 2
#define G 3
#define DP 25

int fd;
char buf[256];

void callback_function(int status)
{
	int cnt = read(fd, buf, 256);
	buf[cnt] = '\0';
	write(fd, "echo: ", 6);
	write(fd, buf, cnt);
	write(fd, "\r\n", 2);
	printf("Received: %s\r\n", buf);
	
	switch(buf[0]){
		case '0':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 0);
			digitalWrite(DP, 0);
			break;
		case '1':
			digitalWrite(A, 0);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 0);
			digitalWrite(E, 0);
			digitalWrite(F, 0);
			digitalWrite(G, 0);
			digitalWrite(DP, 0);
			break;
		case '2':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 0);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 0);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case '3':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 0);
			digitalWrite(F, 0);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case '4':
			digitalWrite(A, 0);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 0);
			digitalWrite(E, 0);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case '5':
			digitalWrite(A, 1);
			digitalWrite(B, 0);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 0);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case '6':
			digitalWrite(A, 1);
			digitalWrite(B, 0);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case '7':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 0);
			digitalWrite(E, 0);
			digitalWrite(F, 1);
			digitalWrite(G, 0);
			digitalWrite(DP, 0);
			break;
		case '8':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case '9':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 0);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'a':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 0);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'b':
			digitalWrite(A, 0);
			digitalWrite(B, 0);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'c':
			digitalWrite(A, 0);
			digitalWrite(B, 0);
			digitalWrite(C, 0);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 0);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'd':
			digitalWrite(A, 0);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 0);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'e':
			digitalWrite(A, 1);
			digitalWrite(B, 0);
			digitalWrite(C, 0);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'f':
			digitalWrite(A, 1);
			digitalWrite(B, 0);
			digitalWrite(C, 0);
			digitalWrite(D, 0);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'A':
			digitalWrite(A, 1);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 0);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'B':
			digitalWrite(A, 0);
			digitalWrite(B, 0);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'C':
			digitalWrite(A, 0);
			digitalWrite(B, 0);
			digitalWrite(C, 0);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 0);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'D':
			digitalWrite(A, 0);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 0);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'E':
			digitalWrite(A, 1);
			digitalWrite(B, 0);
			digitalWrite(C, 0);
			digitalWrite(D, 1);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		case 'F':
			digitalWrite(A, 1);
			digitalWrite(B, 0);
			digitalWrite(C, 0);
			digitalWrite(D, 0);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
		default:
			digitalWrite(A, 0);
			digitalWrite(B, 1);
			digitalWrite(C, 1);
			digitalWrite(D, 0);
			digitalWrite(E, 1);
			digitalWrite(F, 1);
			digitalWrite(G, 1);
			digitalWrite(DP, 0);
			break;
	}
						
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
	

	if (wiringPiSetup() == -1){
		return 1;
	}
	
	pinMode(A, OUTPUT);
	pinMode(B, OUTPUT);
	pinMode(C, OUTPUT);
	pinMode(D, OUTPUT);
	pinMode(E, OUTPUT);
	pinMode(F, OUTPUT);
	pinMode(G, OUTPUT);
	pinMode(DP, OUTPUT);
	

	while(1) {

		task();
	}
	return 0;
}
