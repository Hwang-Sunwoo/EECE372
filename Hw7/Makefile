/*CC = gcc
TARGET = exec
CFLAGS = -fopenmp -lwiringPi -mfpu=neon -march=armv7-a


$(TARGET) : 20220900_opt2.c
	$(CC) -o $(TARGET) 20220900_opt2.c -lm $(CFLAGS)
*/
CC = gcc
TARGET = exec
CFLAGS = -g -mfpu=neon -march=armv7-a
LDFLAGS = -lm -fopenmp -lwiringPi

$(TARGET): 20220900_opt2.o
	$(CC) -o $(TARGET) 20220900_opt2.o $(LDFLAGS)

20220900_opt2.o: 20220900_opt2.c
	$(CC) -c 20220900_opt2.c $(CFLAGS)

clean:
	rm -f $(TARGET) 20220900_opt2.o
