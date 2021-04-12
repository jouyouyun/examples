NSS_WEN=libnss_wen.so.2

build : nss

nss:
	gcc -g -fPIC -shared -o ${NSS_WEN} -Wl,-soname,libnss_wen.so.2 nss_wen.c common.c `pkg-config --libs --cflags libsystemd`

clean:
	rm -f ${NSS_WEN}

rebuild: clean build
