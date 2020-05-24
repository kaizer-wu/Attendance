CC=gcc
CFLAGS=-c -g -I./include -Wall
LDFLAGS=-I./include -lpthread -lwiringPi -ljpeg
VPATH=src:include


SRCS = $(wildcard ./src/*.c) 
OBJS = $(patsubst %.c,%.o,$(SRCS))


Attendance:$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.PHONY:clean

clean:
	rm ./src/*.o Attendance 
