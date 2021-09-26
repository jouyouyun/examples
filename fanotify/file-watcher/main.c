#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/fanotify.h>

#include "watcher.h"

static int *fd_list = NULL;
static int fd_num = 0;

static void *handle_thread(void *data)
{
	int ret = 0;
	
	ret = poll_event();
	printf("Poll event: %d\n", ret);

	return NULL;
}

static int append_fd(int fd)
{
	int *tmp = NULL;

	tmp = realloc(fd_list, sizeof(int)*(fd_num + 1));
	if (!tmp) {
		perror("realloc");
		return -1;
	}

	fd_list = tmp;
	fd_list[fd_num] = fd;
	fd_num++;
	return 0;
}

static void free_fd_list()
{
	int i = 0;

	for (; i < fd_num; i++)
		close(fd_list[i]);
	free(fd_list);
	fd_list = NULL;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	pthread_t thrd;

	ret = watch_file("/usr/local/bin/hostname", FAN_CLASS_CONTENT, FAN_MARK_ADD,
		FAN_OPEN_EXEC_PERM);
	if (ret < 0)
		return ret;

	if (append_fd(ret) != 0) {
		close(ret);
		return -1;
	}
	
	ret = pthread_create(&thrd, NULL, handle_thread, NULL);
	if (ret != 0) {
		perror("pthread_create");
		close(ret);
		return ret;
	}
	if (pthread_detach(thrd) != 0)
		perror("pthread_detach");

	printf("Test hostname, will wait 5s\n");
	sleep(5);

	ret = watch_file("/home/wen/deepin.xsettingsd", FAN_CLOEXEC, FAN_MARK_ADD,
			 FAN_MODIFY | FAN_CLOSE_WRITE);
        if (ret < 0) {
		free_fd_list();
		return ret;
	}

        if (append_fd(ret) != 0) {
		close(ret);
                free_fd_list();
                return -1;
        }

        printf("Test deepin.xsettingsd, will wait 15s\n");
        sleep(15);

	watch_remove(fd_list[0]);
	printf("Remove hostname, will wait 30s\n");
        sleep(30);

	pthread_cancel(thrd);
	free_fd_list();
	
        return 0;
}
