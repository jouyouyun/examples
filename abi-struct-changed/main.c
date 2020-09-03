#include <stdio.h>
#include <stdlib.h>

#include "./origin/test.h"

int
main(int argc, char *argv[])
{
	if (argc > 1) {
		printf("Usage: %s", argv[0]);
		return -1;
	}

	struct test_p *p = create_test();
	if (!p) {
		printf("Failed to create test\n");
		return -2;
	}

	printf("Test: (%d, %d)\n", p->a, p->b);
	free(p);

	return 0;
}
