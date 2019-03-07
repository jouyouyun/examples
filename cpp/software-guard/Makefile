CPP=g++
PROG_NAME=software_guard
PWD=$(shell pwd)

CFLAGS=-Wall -g
LDFLAGS=

SRCS := main.cpp software_guard.cpp netlink_monitor.cpp blacklist.cpp

build: ${PROG_NAME}

${PROG_NAME}:${SRCS}
	${CPP} ${CFLAGS} $^ ${LDFLAGS} -o $@

clean:
	rm -f ${PROG_NAME}

rebuild: clean ${PROG_NAME}
