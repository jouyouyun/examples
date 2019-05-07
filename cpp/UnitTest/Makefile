CPP=g++ -Wall -g
PWD=$(shell pwd)
CATCH_TEST=catch_test
GOOGLE_TEST=google_test


vpath %.cpp ${PWD}

CATCH_OBJS = hostname.o catch_test.o
# needed compile gtest
GOOGLE_OBJS = hostname.o google_test.o
GOOGLE_LIBS=-lgtestd

build: ${CATCH_TEST} ${GOOGLE_TEST}

test:build
	${PWD}/${CATCH_TEST}; ${PWD}/${GOOGLE_TEST}

${CATCH_TEST}: ${CATCH_OBJS}
	${CPP} -o $@ $^

${GOOGLE_TEST}: ${GOOGLE_OBJS}
	${CPP} -o $@ $^ ${GOOGLE_LIBS}

%.o:%.cpp
	${CPP} -c $<

clean:
	rm -rf *.o ${CATCH_TEST} ${GOOGLE_TEST}

rebuild: clean build
