CPP=g++
PROG_NAME=software_guard
PWD=$(shell pwd)
RUNNER_DIR=${PWD}/../runner

CFLAGS=-Wall -g
LDFLAGS=-lboost_system -lboost_filesystem -lboost_thread -lpthread

vpath %.cpp ${PWD} : ${RUNNER_DIR}

OBJS := main.o software_guard.o software_history.o netlink_monitor.o blacklist.o task_manager.o task.o

build: ${PROG_NAME}

${PROG_NAME}:${OBJS}
	${CPP} $^ ${LDFLAGS} -o $@

%.o:%.cpp
	${CPP} ${CFLAGS} -c $<

clean:
	rm -f *.o ${PROG_NAME}

rebuild: clean ${PROG_NAME}
