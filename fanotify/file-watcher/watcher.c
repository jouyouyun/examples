#define _GNU_SOURCE /* for O_LARGEFILE */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fanotify.h>

#include <poll.h>

#include "watcher.h"

static struct pollfd *fds = NULL;
static nfds_t nfds = 0;

static int handle_event(int fd, const struct fanotify_event_metadata *metadata,
                        ssize_t len) {
	int ret = EXIT_SUCCESS;
	char *path = NULL;
        char *prog_path = NULL;
        ssize_t path_len;
        char procfd_path[PATH_MAX];
	struct fanotify_response response;

	/* filepath */
        memset(procfd_path, 0, PATH_MAX);
        snprintf(procfd_path, sizeof(procfd_path), "/proc/self/fd/%d",
                 metadata->fd);
        path = (char*)calloc(PATH_MAX, sizeof(char));
	path_len = readlink(procfd_path, path, PATH_MAX - 1);
	if (path_len == -1) {
		perror("readlink");
	}
	path[path_len] = '\0';
	printf("PATH: %s\n", path);

	/* program name */
	memset(procfd_path, 0, PATH_MAX);
        snprintf(procfd_path, sizeof(procfd_path), "/proc/%d/exe",
                 metadata->pid);
        prog_path = (char *)calloc(PATH_MAX, sizeof(char));
        path_len = readlink(procfd_path, prog_path, PATH_MAX - 1);
        if (path_len == -1) {
		perror("readlink");
	}
	prog_path[path_len] = '\0';
	printf("PROG PATH: %d -- %s\n", metadata->pid, prog_path);

        if (metadata->mask & FAN_OPEN_EXEC_PERM) { /* OPEN_EXEC_PERM  */
		printf("FAN_OPEN_EXEC_PERM: '%s' <-- '%s'\n", path, prog_path);
		response.fd = metadata->fd;
		response.response = FAN_ALLOW;
		write(fd, &response, sizeof(response));
        } else if (metadata->mask & FAN_CLOSE_WRITE) {/* CLOSE_WRITE */
		printf("FAN_CLOSE_WRITE: '%s' <-- '%s'\n", path, prog_path);
        } else if (metadata->mask & FAN_MODIFY) {/* MODIFY */
		printf("FAN_MODIFY: '%s' <-- '%s'\n", path, prog_path);
	}

	free(path);
	free(prog_path);
	close(metadata->fd);
	return ret;
}

#define MAX_BUF 256

static int read_event(int fd) {
	const struct fanotify_event_metadata *metadata = NULL;
	struct fanotify_event_metadata buf[MAX_BUF];
	ssize_t len;

	len = read(fd, buf, sizeof(buf));
	if (len == -1 && errno != EAGAIN) {
		perror("read");
		return -11;
	}

	/* check if end of available data reached. */
	if (len <= 0)
		return -12;

	/* point to the first event in the buffer */
	metadata = buf;

	/* loop over all events in the buffer */
	while (FAN_EVENT_OK(metadata, len)) {
		/* check that runtime and compile-time structures match */
		if (metadata->vers != FANOTIFY_METADATA_VERSION) {
			fprintf(stderr, "Mismatch of fanotify metadata version.\n");
			return -13;
		}

		/* metadata->fd contains either FAN_NOFD, indicating a
		   queue overflow, or a file descriptor (a nonnegative
		   integer). Here, we simply ignore queue overflow. */
		if (metadata->fd < 0) {
			continue;
		}

		// TODO(jouyouyun): thread
		if (handle_event(fd, metadata, len) == EXIT_FAILURE)
			return -14;

		metadata = FAN_EVENT_NEXT(metadata, len);
	}

	return 0;
}

static int
setup_fanotify(const char *filename, unsigned int init_flags,
	       unsigned int mark_flags, unsigned int event_flags)
{
	int fd = 0;

	fd = fanotify_init(init_flags, O_RDONLY|O_LARGEFILE);
	if (fd == -1) {
		perror("fanotify_init");
		return -1;
	}

	if (fanotify_mark(fd, mark_flags, event_flags, AT_FDCWD, filename) == -1) {
		perror("fanotify_mark");
		close(fd);
		return -2;
	}
	
	return fd;
}

static int setup_poll(int fd)
{
	struct pollfd *tmp = NULL;
	
	tmp = realloc(fds, sizeof(struct pollfd) * (nfds + 1));
	if (!tmp) {
		perror("realloc");
		return -3;
	}

	fds = tmp;
	fds[nfds].fd = fd;
	fds[nfds].events = POLLIN;
	nfds++;

	return 0;
}

int watch_file(const char *filename, unsigned int init_flags,
               unsigned int mark_flags, unsigned int event_flags)
{
	int fd = 0;
	int ret = 0;

	fd = setup_fanotify(filename, init_flags, mark_flags, event_flags);
	if (fd < 0) {
		return fd;
	}

        ret = setup_poll(fd);
	if (ret < 0) {
		close(fd);
		return ret;
	}
	
        return fd;
}

int watch_remove(int fd)
{
	int i = 0;
	int len = sizeof(struct pollfd);
	struct pollfd *tmp = NULL;

	// TODO(jouyouyun): lock
	for (; i < nfds; i++) {
		if (fd == fds[i].fd) {
			break;
		}
	}
	if (i == nfds)
		return -31;

	tmp = calloc(nfds - 1, len);
	memcpy(tmp, fds, len * i);
	memcpy(tmp+i, fds+i+1, len * (nfds - i));

	free(fds);
	fds = tmp;
	nfds -= 1;
	close(fd);

	return 0;
}

int poll_event() {
	int i = 0;
	int ret = 0;
	int num = 0;
	int timeout = 3;
	static int loop = 0;

	if (loop)
		return 0;

	if (nfds < 1)
		return -21;

	loop = 1;
	while (1) {
		num = poll(fds, nfds, timeout);
		if (num == -1) {
			perror("poll");
			goto error;
		}
		if (num < 1)
			goto error;

		for (i = 0; i < nfds; i++) {
			if (fds[i].revents & POLLIN) {
				ret = read_event(fds[i].fd);
				if (ret < 0)
					fprintf(stderr, "Failed to handle event: %d\n", ret);
			}
		}
		continue;

	error:
		usleep(500 * 1000); // 500ms
	}
}
