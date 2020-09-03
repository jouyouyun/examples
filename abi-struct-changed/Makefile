CC = gcc
TARGET = abi_test
ORIGIN = ./origin
CHANGED = ./changed

build :
	cd ${ORIGIN} && make rebuild && cd ..
	cd ${CHANGED} && make rebuild && cd ..
	gcc -Wall -g -L${ORIGIN} -ltest -o ${TARGET} main.c

run-origin:
	env LD_LIBRARY_PATH=${ORIGIN} ./${TARGET}

run-changed:
	env LD_LIBRARY_PATH=${CHANGED} ./${TARGET}

clean:
	cd ${ORIGIN} && make clean && cd ..
	cd ${CHANGED} && make clean && cd ..
	rm -f ${TARGET}

rebuild: clean build
