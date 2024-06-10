CC = gcc
TARGET = exec
CFLAGS = -fopenmp -lwiringPi -mfpu=neon -march=armv7-a


$(TARGET) : 20220900_opt2.c
	$(CC) -o $(TARGET) 20220900_opt2.c -lm $(CFLAGS)
