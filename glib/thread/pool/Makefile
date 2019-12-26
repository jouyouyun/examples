PROG=test_thrd_pool
CC=gcc

CFLAGS = -Wall -g `pkg-config --cflags --libs glib-2.0`

build: ${PROG}

${PROG} : main.c
	${CC} -o $@ $< ${CFLAGS}

clean:
	rm -f ${PROG}

rebuild: clean ${PROG}
