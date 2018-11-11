/*
 * Copyright © 2018-2018 jouyouyun <jouyouwen717@gmail.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Red Hat
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.  Red
 * Hat makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct touch_post_event {
	int id;
	char *name;
};

typedef struct _touch_post_list {
	unsigned int length;
	struct touch_post_event *events;
}TouchPostList;

static void touch_event_dump();
static void touch_post_list_free();

static TouchPostList touch_list;

int
touch_post_list_append(struct touch_post_event *post)
{
	struct touch_post_event *tmp = (struct touch_post_event*)realloc(touch_list.events, touch_list.length+1);
	touch_list.events = tmp;
	touch_list.events[touch_list.length] = *post;
	touch_list.length++;
	return 0;
}

int
main(int argc, char *argv[])
{
	printf("Push tmp1\n");
	struct touch_post_event tmp1 = {.id = 0, .name="one"};
	touch_post_list_append(&tmp1);
	touch_event_dump();
	printf("Push tmp2\n");
	struct touch_post_event tmp2 = {.id = 1, .name="two"};
	touch_post_list_append(&tmp2);
	touch_event_dump();

	printf("Free\n");
	touch_post_list_free();
	touch_event_dump();

	printf("Repush tmp1\n");
	touch_post_list_append(&tmp1);
	touch_event_dump();

	printf("Refree\n");
	touch_post_list_free();
	touch_event_dump();
	return 0;
}

static void
touch_post_list_free()
{
	if (touch_list.events == NULL) {
		return;
	}

	free(touch_list.events);
	touch_list.events = NULL;
	touch_list.length = 0;
}

static void touch_event_dump()
{
	printf("Length: %u\n", touch_list.length);
	unsigned int i = 0;
	for (;i < touch_list.length; i++) {
		printf("The [%u]: id: %d, name: %s\n", i,
		       touch_list.events[i].id, touch_list.events[i].name);
	}
}
