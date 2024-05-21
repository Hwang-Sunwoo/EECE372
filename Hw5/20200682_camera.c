#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define BAUDRATE B1000000

int main()
{
	int fd;
	struct termios newtio;
	char fbuf[1024];
	char buf[256];

	fd = open("/dev/ttyAMA0", O_RDWR|O_NOCTTY);
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
	
//	speed_t baudRate = B1000000;
//	cfsetispeed(&newtio, baudRate);
//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	while(1) {
		int cnt=0;
		for (i=0;i<sizeof(fbuf);i++){
			fbuf[i]=0;
		}
		cnt=read(fd, buf, sizeof(buf));
		
		if(buf[0]=='c'||buf[0]=='C'){
			system("raspistill -w 640 -h 480 -o output.bmp");
			FILE* fp = fopen("output.bmp","rb");
			while(1){
				fread(fbuf, sizeof(char), sizeof(fbuf),fp);
				write(fd, fbuf, sizeof(fbuf));
				if(feof(fp)){
					break;
				}
			}
			fclose(fp);
			buf[0]=0;
		}
	}
	return 0;
}
