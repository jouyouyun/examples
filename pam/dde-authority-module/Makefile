TARGET=pam_dde_authority.so
CC=gcc
CFLAGS=-Wall -g `pkg-config --cflags --libs libsystemd`
SRCS=pam.c

build: ${TARGET}

${TARGET}:${SRCS}
	${CC} ${CFLAGS} -fPIC -shared $^ -o $@

install:
	mkdir -p /lib/x86_64-linux-gnu/security/
	cp ${TARGET} /lib/x86_64-linux-gnu/security/

clean:
	rm -f ${TARGET}

rebuild: clean build
