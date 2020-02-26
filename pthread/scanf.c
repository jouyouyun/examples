/**
 ** Compile: gcc -Wall -g scanf.c -o scanf_test -lpthread
 **/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define DEFAULT_INTERVAL 3
#define DEFAULT_TIMEOUT 15

static pthread_t tid = 0;
static int done = 0;

static void*
start_thread(void *data)
{
	int num = 0;
	char buf[1024] = {0};

	memset(buf, 0 , 1024);
	fprintf(stderr, "Please input some words: ");
	num = scanf("%s", buf);
	if (num < 0) {
		fprintf(stderr, "Failed to scanf: %s\n", strerror(errno));
	} else {
		fprintf(stderr, "Success to received: %d\n", num);
	}

	done = 1;
	return NULL;
}

static void
cancel_thread()
{
	int error = pthread_cancel(tid);
	if (error) {
		fprintf(stderr, "Failed to cancel thread: %s\n", strerror(errno));
	}
}

int
main(int argc, char *argv[])
{
	int error = 0;
	int interval = 0;
	int timeout = 0;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <interval> <timeout>\n", argv[0]);
		return -1;
	}

	interval = atoi(argv[1]);
	if (interval <= 0) {
		interval = DEFAULT_INTERVAL;
	}
	timeout = atoi(argv[2]);
	if (timeout <= 0) {
		timeout = DEFAULT_TIMEOUT;
	}

	error = pthread_create(&tid, NULL, start_thread, NULL);
	if (error) {
		fprintf(stderr, "Failed to create thread: %s\n", strerror(errno));
		return error;
	}
	pthread_detach(tid);

	fprintf(stderr, "Will wait thread done\n");
	do {
		if (timeout < 0) {
			fprintf(stderr, "Timeout, cancel thread\n");
			cancel_thread();
			break;
		}

		sleep(interval);
		timeout -= interval;
	} while (!done);

	fprintf(stderr, "Exit...\n");
	return 0;
}
