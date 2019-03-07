CPP=g++
PROG_NAME=runner_test
PWD=$(shell pwd)

CFLAGS=-Wall -g
LDFLAGS=-lboost_system -lboost_filesystem -lboost_thread -lpthread

SRCS := main.cpp task.cpp task_manager.cpp

build: ${PROG_NAME}

${PROG_NAME}:${SRCS}
	${CPP} ${CFLAGS} $^ ${LDFLAGS} -o $@

clean:
	rm -f ${PROG_NAME}

rebuild: clean ${PROG_NAME}
