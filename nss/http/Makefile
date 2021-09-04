NSS_PROG = libnss_http.so

CC = gcc
INCS = common/
CFLAGS = -Wall -g -I${INCS} `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0`

NSS_SRCS = common/passwd.c common/group.c common/shadow.c \
		nss/pwd.c nss/grp.c nss/spwd.c

build : ${NSS_PROG}

rebuild : clean build

${NSS_PROG} : ${NSS_SRCS}
	${CC} ${CFLAGS} ${LDFLAGS} -fPIC -shared $^ -o $@

clean:
	rm -f ${NSS_PROG}
