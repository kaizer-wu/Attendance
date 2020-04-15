CC=gcc
CFLAGS=-c -g -I./include
LDFLAGS=-I./include -lpthread -lwiringPi
VPATH=src:include


SRCS = $(wildcard ./src/*.c) 
OBJS = $(patsubst %.c,%.o,$(SRCS))


Attendance:$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

.PHONY:clean

clean:
	rm ./src/*.o Attendance 
