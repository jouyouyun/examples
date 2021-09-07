/**
 * Copyright (C) 2020 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * main.c -- Test hook register
 *
 * Written on 星期三, 24 六月 2020.
 */

#include <stdio.h>
#include <stdint.h>

static int
test_int(char *msg)
{
	printf("[%s] message: %s\n", __func__, msg);
	return 0;
}

static void
test_void(char *msg)
{
	printf("[%s] message: %s\n", __func__, msg);
	return;
}

int
main(int argc, char *argv[])
{
	int ret = 0;
	uint64_t int_addr = (uint64_t)test_int;
	uint64_t void_addr = (uint64_t)test_void;

	printf("test_int addr: %lu\n", int_addr);
	ret = ((int(*)(char*))int_addr)("INT");
	printf("Exec return: %d\n", ret);
	
	printf("test_void addr: %lu\n", void_addr);
	((void(*)(char*))void_addr)("VOID");

	return 0;
}
