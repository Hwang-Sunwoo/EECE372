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
	
//	speed_t baudRate = B1000000;
//	cfsetispeed(&newtio, baudRate);
//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	while(1) {
		// Insert your code
		memset(fbuf, 0 sizeof(fbuf));
		int cnt = read(fd, buf, sizeof(buf));
		buf[cnt] = '\0';
	
		if(buf[0] == 'c' || buf[0] == 'C'){
			printf("kimch\r\n");
			system("libcamera-still --width 640 --height 480 -o image.jpg");
			
			FILE *fp = fopen("image.jpg", "rb");
			if(fp == NULL){
				fprintf(stderr, "failed to open image file: %s.\r\n", strerror(errno));
				continue;
			}
			for (int i = 0; i < 480; i++){
				fread(fbuf, 1, 640, fp);
				write(fd, fbuf, 640);
			}
			fclose(fp);		
		}

	}
	return 0;
}
