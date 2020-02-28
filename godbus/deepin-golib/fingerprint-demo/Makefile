PROG=deepin-fingerprint-demo
CC=go build

build:
	${CC} -o ${PROG}

install:
	cp -f ${PROG} ${DESTDIR}${PREFIX}/bin/
	cp -f ./misc/com.deepin.Fingerprint.Demo.conf ${DESTDIR}${PREFIX}/share/dbus-1/system.d/
	cp -f ./misc/com.deepin.Fingerprint.Demo.service ${DESTDIR}${PREFIX}/share/dbus-1/system-services/
	cp -f ./misc/demo.json ${DESTDIR}${PREFIX}/share/deepin-authentication/interfaces/

uninstall:
	rm -f ${DESTDIR}${PREFIX}/share/dbus-1/system.d/com.deepin.Fingerprint.Demo.conf
	rm -f ${DESTDIR}${PREFIX}/share/dbus-1/system-services/com.deepin.Fingerprint.Demo.service
	rm -f ${DESTDIR}${PREFIX}/share/deepin-authentication/interfaces/demo.json
	rm -f ${DESTDIR}${PREFIX}/bin/${PROG}
	killall ${PROG} > /dev/null 2>&1 || true

clean:
	rm -f ${PROG}

rebuild: clean build
