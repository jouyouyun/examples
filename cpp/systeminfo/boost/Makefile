CPP=g++
PROG_NAME=systeminfo_test
PWD=$(shell pwd)

CFLAGS=-Wall -g
LDFLAGS=-lboost_system -lboost_filesystem -lboost_thread -lpthread -lcrypto++

SRCS := main.cpp file.cpp os.cpp dmi.cpp systeminfo.cpp network_device.cpp lsblk.cpp task_manager.cpp task.cpp

build: ${PROG_NAME}

${PROG_NAME}:${SRCS}
	${CPP} ${CFLAGS} $^ ${LDFLAGS} -o $@

clean:
	rm -f ${PROG_NAME}

rebuild: clean ${PROG_NAME}
