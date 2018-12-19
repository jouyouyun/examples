PROG_NAME=mysql_backup_helper

build:
	go build -o ${PROG_NAME}

install:
	cp -f ${PROG_NAME} /usr/bin
	mkdir -p /usr/share/mysql-backup-helper/
	cp -f data/config.json /usr/share/mysql-backup-helper/
	cp -f data/mysql_backup_helper.service /lib/systemd/system/

clean:
	rm -f ${PROG_NAME}

rebuild: clean build
