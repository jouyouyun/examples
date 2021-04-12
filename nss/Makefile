NSS_WEN=libnss_wen.so.2
PAM_WEN=pam_wen.so
FLAGS=`pkg-config --libs --cflags libsystemd`

build : nss pam

nss:
	gcc -g -fPIC -shared -o ${NSS_WEN} -Wl,-soname,libnss_wen.so.2 nss_wen.c common.c ${FLAGS}

pam:
	gcc -Wall -g -fPIC -shared -o ${PAM_WEN}  pam_wen.c common.c ${FLAGS}

clean:
	rm -f ${NSS_WEN} ${PAM_WEN}

rebuild: clean build
