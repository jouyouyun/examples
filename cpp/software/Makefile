CPP=g++
PROG_NAME=software_test
PWD=$(shell pwd)

CFLAGS=-Wall -g
LDFLAGS=-lboost_system -lboost_thread

SRCS := main.cpp software_info.cpp software.cpp executor.cpp

build: ${PROG_NAME}

${PROG_NAME}:${SRCS}
	${CPP} ${CFLAGS} $^ ${LDFLAGS} -o $@

clean:
	rm -f ${PROG_NAME}

rebuild: clean ${PROG_NAME}
