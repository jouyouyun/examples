CPP=g++ -Wall -g
PWD=$(shell pwd)
PROG=net_dev_list

LDFLAGS = -lboost_system -lboost_filesystem -lboost_thread -lpthread
OBJS = main.o network_device.o file.o task.o task_manager.o

vpath %.cpp ${PWD} ../runner

build:${PROG}

${PROG}:${OBJS}
	${CPP} -o $@ $^ ${LDFLAGS}

%.o:%.cpp
	${CPP} -c $<

clean:
	rm -rf *.o ${PROG}

rebuild: clean build