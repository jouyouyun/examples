#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <systemd/sd-journal.h>

#define PORT_START 7050
#define MAXLINE 4096

static int bind_wrapper(int *fd, struct sockaddr_in *addr, int *port)
{
	int ret = 0;

	do {
		ret = bind(*fd, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
		if (ret == 0) {
			break;
		} else if (ret == EADDRINUSE) {
			(*port)++;
			addr->sin_port = htons(*port);
		} else {
			break;
		}
	} while(1);

	return ret;
}

static int create_pidfile(int port)
{
	char buf[16];
	pid_t pid = getpid();

	memset(buf, 0, 16);
	sprintf(buf, "%u:%d", pid, port);

	return 0;
}

int main()
{
	int ret = 0;
	int sock_fd = 0, conn_fd = 0;
	int port = PORT_START;
	struct sockaddr_in srvaddr;

	if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		sd_journal_print(LOG_ERR, "failed to open socket: %s", strerror(errno));
		return -1;
	}

	memset(&srvaddr, 0, sizeof(srvaddr));
	srvaddr.sin_family = AF_UNIX;
	srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvaddr.sin_port = htons(PORT_START);

	if((ret = bind_wrapper(&sock_fd, &srvaddr, &port)) != 0) {
		sd_journal_print(LOG_ERR, "failed to bind: %s", strerror(ret));
		goto out;
	}

out:
	close(sock_fd);

	return ret;
}
