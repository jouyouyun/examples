CPP=g++
PROG_NAME=software_test
PWD=$(shell pwd)

CFLAGS=-Wall -g
LDFLAGS=-lboost_system -lboost_filesystem -lboost_thread -lpthread

vpath %.cpp ${PWD}
OBJS := main.o software_info.o software.o executor.o task.o task_manager.o

build: ${PROG_NAME}

${PROG_NAME}:${OBJS}
	${CPP} ${CFLAGS} $^ ${LDFLAGS} -o $@

%.o: %.cpp
	${CPP} ${CFLAGS} -c $<

clean:
	rm -f *.o ${PROG_NAME}

rebuild: clean ${PROG_NAME}
