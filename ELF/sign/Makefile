PROG_SIGN=elf_sign
PROG_VERIFY=elf_verify

CC=gcc
PWD=$(shell pwd)
CFLAGS=-Wall -g
LDFLAGS=-lelf -lcrypto

COMMON_OBJS := hash.o
SIGN_OBJS := sign.o ${COMMON_OBJS}
VERIFY_OBJS := verify.o ${COMMON_OBJS}

vpath %.c ${PWD}

build : ${PROG_SIGN} ${PROG_VERIFY}

${PROG_SIGN} : ${SIGN_OBJS}
	${CC} -o $@ $^ ${LDFLAGS}

${PROG_VERIFY} : ${VERIFY_OBJS}
	${CC} -o $@ $^ ${LDFLAGS}

$.o : %.c
	${CC} ${CFLAGS} -c @< -o $@

clean :
	rm -f ${SIGN_OBJS} ${VERIFY_OBJS} ${PROG_SIGN} ${PROG_VERIFY}

rebuild: clean build
