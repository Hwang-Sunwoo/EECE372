CC = gcc
TARGET = exec
CFLAGS = -fopenmp -lwiringPi -mfpu=neon -march=native


$(TARGET) : 20220900_opti.c
	$(CC) -o $(TARGET) 20220900_opti.c -lm $(CFLAGS)
