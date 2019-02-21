# $@  目标文件名
# $<  第一个依赖的文件名
# $^  所有依赖的文件名


CC =  gcc
PROG_NAME = Test

#当前目录
PWD = $(shell pwd)

#头文件所在目录
INCS := /home/iwen/Yunio/My_Headers
#源文件目录
SRC_DIR := /home/iwen/Yunio/My_Sources

#vpath PATTERN DIRECTORIES
#在 DIRECTORIES 目录中搜索符合条件 PATTERN 的文件
#多个目录用 : 隔开
#vpath %.c ${SRC_DIR} : ${PWD}/img
vpath %.c ${SRC_DIR}

CFLAGS  = -Wall -g -I${INCS}
LDFLAGS = -Wall -g -I${INCS}

OBJS := my_file.o main.o

all : ${PROG_NAME}

${PROG_NAME} : ${OBJS}
	${CC} ${CFLAGS} $^ -o $@
	
%.o : %.c
	${CC} ${CFLAGS} -c $<

clean :
	rm -f *.o ${PROG_NAME}
	
rebuild : clean all
