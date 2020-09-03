#include "test.h"

#include <stdlib.h>

struct test_p*
create_test(void)
{
	struct test_p *p = calloc(1, sizeof(struct test_p));
	if (!p)
		return NULL;

	p->a = 1;
	p->aa = 2;
	p->b = 3;

	return p;
}
