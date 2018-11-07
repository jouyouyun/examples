#include <stdio.h>
#include <string.h>
#include <signal.h>

#define SIG SIGINT

static void
sig_handler(int sig, siginfo_t *si, void *data)
{
	printf("Caught signal: %d\n", sig);
	printf("Sender pid: %d\n", si->si_pid);
	printf("Sender uid: %d\n", si->si_uid);
}

static int
sig_caught(int sig)
{
	printf("Start caught signal: %d\n", sig);
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sig_handler;
	sigemptyset(&sa.sa_mask);
	int ret = sigaction(sig, &sa, NULL);
	if (ret == -1) {
		printf("Failed to caught signal: %d\n", sig);
		return -1;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	if (sig_caught(SIG) == -1) {
		return -1;
	}

	printf("Caught signal(%d), input 'quit' to exit...\n", SIG);
	char buf[1024] = {0};
	while(1) {
		printf("Please input: ");
		scanf("%s", buf);
		if (strcmp(buf, "quit") == 0) {
			break;
		}
	}
	printf("Exit...\n");
	return 0;
}
