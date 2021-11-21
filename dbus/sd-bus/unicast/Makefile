PROG_SERVER = server
PROG_LISTEN = listen
PROG_CLIENT = client

PWD = $(shell pwd)

CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags libsystemd`
LDFLAGS = -Wall -g `pkg-config --libs libsystemd`

vpath %.c ${PWD}

all: ${PROG_SERVER} ${PROG_LISTEN} ${PROG_CLIENT}

${PROG_SERVER} : ${PROG_SERVER}.o
	${CC} ${LDFLAGS} $^ -o $@

${PROG_LISTEN} : ${PROG_LISTEN}.o
	${CC} ${LDFLAGS} $^ -o $@

${PROG_CLIENT} : ${PROG_CLIENT}.o
	${CC} ${LDFLAGS} $^ -o $@

%.o : %.c
	${CC} ${CFLAGS} -c $<

clean:
	rm -f *.o ${PROG_SERVER} ${PROG_LISTEN} ${PROG_CLIENT}

rebuild: clean all
