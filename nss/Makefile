NSS_WEN=libnss_wen.so.2
PAM_WEN=pam_wen.so
TEST_COMM=common_test
FLAGS=`pkg-config --libs --cflags libsystemd`

build : nss pam

nss:
	gcc -g -fPIC -shared -o ${NSS_WEN} -Wl,-soname,libnss_wen.so.2 nss_wen.c common.c ${FLAGS}

pam:
	gcc -Wall -g -fPIC -shared -o ${PAM_WEN}  pam_wen.c common.c ${FLAGS}

test:
	gcc -Wall -g -o ${TEST_COMM} common_test.c common.c message.c ${FLAGS}

clean:
	rm -f ${NSS_WEN} ${PAM_WEN} ${TEST_COMM}

install:
	cp ${NSS_WEN} /lib/`uname -m`-linux-gnu/
	cp ${PAM_WEN} /lib/`uname -m`-linux-gnu/security/
	cp -rf ./testdata /etc/wen

uninstall:
	rm -rf /etc/wen
	rm -f /lib/`uname -m`-linux-gnu/${NSS_WEN}
	rm -f /lib/`uname -m`-linux-gnu/security/${PAM_WEN}

rebuild: clean build
