SERVER=test_server
CC=gcc

CFLAGS = -Wall -g `pkg-config --cflags --libs gio-2.0`

build: ${SERVER}

${SERVER} : server.c
	${CC} -o $@ $< ${CFLAGS}

clean:
	rm -f ${SERVER}

rebuild: clean ${SERVER}
