#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static void
handler(int sig)
{
	printf("alarm arrived: %d\n", sig);
}

int
main(int argc, char *argv[])
{
	signal(SIGALRM, handler);

	alarm(2);

	sleep(2);
	printf("alarm 5s over\n");

	alarm(10);
	sleep(1);

	unsigned int remaining = alarm(3);
	printf("alarm 10s remain: %u, reset to 3\n", remaining);
	sleep(3);
	printf("alarm 3s over\n");

	alarm(20);
	sleep(3);

	remaining = alarm(0);
	printf("cancel alarm 20s, remian: %u, exit...\n", remaining);
}
