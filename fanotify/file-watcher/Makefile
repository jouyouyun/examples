PROG=file_watcher

CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-lpthread

PWD=$(shell pwd)
vpath %.c ${PWD}
OBJS := watcher.o main.o

build: ${OBJS}
	${CC} $^ ${LDFLAGS} -o ${PROG}

%.o: %.c
	${CC} ${CFLAGS} -c $<

clean:
	rm -f ${PROG} ${OBJS}
