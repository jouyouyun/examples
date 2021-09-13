/*
 * A fanotify example: monitor MOUNT file write and execute, print the file path
 * and program path.
 * Compile: gcc -Wall -g demo -o fanotify_demo
 * Usage: sudo ./fanotify_demo DIR
*/
#define _GNU_SOURCE /* for O_LARGEFILE */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <sys/fanotify.h>

static int
handle_event(int fd, const struct fanotify_event_metadata *metadata,
	     ssize_t len)
{
	int ret = EXIT_SUCCESS;
	char path[PATH_MAX] = {0};
	ssize_t path_len;
	char procfd_path[PATH_MAX];
	struct fanotify_response response;

	/* OPEN_EXEC_PERM  */
	if (metadata->mask & FAN_OPEN_EXEC_PERM) {
		printf("FAN_OPEN_EXEC_PERM: ");
		response.fd = metadata->fd;
		response.response = FAN_ALLOW;
		write(fd, &response, sizeof(response));
	}

	/* CLOSE_WRITE */
	if (metadata->mask & FAN_CLOSE_WRITE)
		printf("FAN_CLOSE_WRITE: ");

	/* filepath */
	snprintf(procfd_path, sizeof(procfd_path),
		 "/proc/self/fd/%d", metadata->fd);
	path_len = readlink(procfd_path, path, sizeof(path) - 1);
	if (path_len == -1) {
		perror("readlink");
		ret = EXIT_FAILURE;
		goto out;
	}

	path[path_len] = '\0';
	printf("File %s\n", path);

	if (metadata->pid <= 0) {
		goto out;
	}

	/* program name */
        memset(procfd_path, 0, PATH_MAX);
        memset(path, 0, PATH_MAX);
        snprintf(procfd_path, sizeof(procfd_path), "/proc/%d/exe",
                 metadata->pid);
        path_len = readlink(procfd_path, path, sizeof(path) - 1);
        if (path_len == -1) {
		perror("readlink");
		ret = EXIT_FAILURE;
		goto out;
        }

        path[path_len] = '\0';
        printf("\tProgram %s\n", path);

 out:
	close(metadata->fd);
	return ret;
}

#define MAX_BUF 256

static void
loop_events(int fd)
{
	const struct fanotify_event_metadata *metadata = NULL;
	struct fanotify_event_metadata buf[MAX_BUF];
	ssize_t len;

	for (;;) {
		len = read(fd, buf, sizeof(buf));
		if (len == -1 && errno != EAGAIN) {
			perror("read");
			return;
		}

		/* check if end of available data reached. */
		if (len <= 0)
			break;

		/* point to the first event in the buffer */
		metadata = buf;

		/* loop over all events in the buffer */
		while (FAN_EVENT_OK(metadata, len)) {
			/* check that runtime and compile-time structures match */
			if (metadata->vers != FANOTIFY_METADATA_VERSION) {
				fprintf(stderr, "Mismatch of fanotify metadata version.\n");
				return;
			}

                        /* metadata->fd contains either FAN_NOFD, indicating a
                           queue overflow, or a file descriptor (a nonnegative
			   integer). Here, we simply ignore queue overflow. */
			if (metadata->fd < 0) {
				continue;
			}

			if (handle_event(fd, metadata, len) == EXIT_FAILURE)
				return;
			
			metadata = FAN_EVENT_NEXT(metadata, len);
		}
	}
}

int
main(int argc, char *argv[])
{
	int fd;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s MOUNT\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = fanotify_init(FAN_CLOEXEC | FAN_CLASS_CONTENT,
			   O_RDONLY | O_LARGEFILE);
	if (fd == -1) {
		perror("fanotify_init");
		exit(EXIT_FAILURE);
	}

        /*
         * set MOUNT mask:
         * - OPEN_EXEC_PERM
	 * - CLOSE_WRITE
	 */
	if (fanotify_mark(fd, FAN_MARK_ADD | FAN_MARK_MOUNT, /* watch the mountpoint of the special dir */
			  FAN_OPEN_EXEC_PERM | FAN_CLOSE_WRITE |
			  FAN_EVENT_ON_CHILD, /* for files in this dir */
			  AT_FDCWD, argv[1]) == -1) {
		perror("fanotify_mark");
		exit(EXIT_FAILURE);
	}

	printf("Listening for events.\n");
	loop_events(fd);

	printf("Listening for events stopped.\n");
	close(fd);
	exit(EXIT_SUCCESS);
}
