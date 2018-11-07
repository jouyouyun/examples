#include <stdio.h>
#include <string.h>
#include <signal.h>

static void
handler(int sig)
{
	printf("Recieved signal: %d\n", sig);
}

int
main(int argc, char *argv[])
{
	signal(SIGINT, handler);

	printf("Caught SIGINT, input 'quit' to exit...\n");
	// wait signal caught
	char buf[1024] = {0};
	while (1) {
		printf("Please input: ");
		scanf("%s", buf);
		if (strcmp(buf, "quit") == 0) {
			break;
		}
	}
	printf("Exit...\n");
	return 0;
}
