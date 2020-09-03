CC = gcc
TARGET = libtest.so
SRC := test.c

build:
	${CC} -shared -fPIC -o ${TARGET} ${SRC}

clean:
	rm -f ${TARGET}

rebuild: clean build
