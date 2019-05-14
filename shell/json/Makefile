CC=g++ -Wall -g
PWD=$(shell pwd)
PROG=json_shell

vpath %.cpp ${PWD} ../../cpp/runner
LDFLAGS := -lboost_system -lboost_filesystem -lboost_thread -lpthread

OBJS := json_shell.o task.o task_manager.o

build: ${PROG}

${PROG}: ${OBJS}
	${CC} -o $@ $^ ${LDFLAGS}

%.o:%.cpp
	${CC} -c $<

clean:
	rm -f *.o ${PROG}

rebuild: clean build
