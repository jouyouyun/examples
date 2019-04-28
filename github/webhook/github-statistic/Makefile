PWD=$(shell pwd)
TARGET=github_statistic

build:
	go build -o ${TARGET} cmd/main.go

clean:
	rm -f ${TARGET}

rebuild: clean build
