CC = gcc
TARGET = exec
CFLAGS = -fopenmp -lwiringPi -mfpu=neon -march=native


$(TARGET) : 20220900_opt.c
	$(CC) -o $(TARGET) 20220900_opt.c -lm $(CFLAGS)
