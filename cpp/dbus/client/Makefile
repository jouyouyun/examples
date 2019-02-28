PROG_CLIENT=dbus_client
CC=g++
CFLAGS=-Wall -g $(shell pkg-config --cflags dbus-c++-1)
LDFLAGS= $(shell pkg-config --libs dbus-c++-1)

SRCS := client.cpp

build:${PROG_CLIENT}

${PROG_CLIENT}:${SRCS}
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

clean:
	rm -f ${PROG_CLIENT} *.o

rebuild:clean build
