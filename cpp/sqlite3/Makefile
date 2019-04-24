PWD=$(shell pwd)
PROGRAM=sqlite_test

CPP=g++
CFLAGS=-Wall -g
LDFLAGS=`pkg-config --libs sqlite3`

vpath %.cpp ${PWD}
OBJS=sqlite_modern_test.o

build: ${PROGRAM}

${PROGRAM}:${OBJS}
	${CPP} ${CFLAGS} $^ -o $@ ${LDFLAGS}

%.0:*.cpp
	${CPP} ${CFLAGS} -c $<

clean:
	rm -f *.o ${PROGRAM}

rebuild: clean build
