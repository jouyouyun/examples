/**
 * Copyright (C) 2022 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * termios.c -- set terminal echo off when input pin
 *
 * gcc -Wall -g termios.c -o test_term_echo
 *
 * Written on 星期四, 13 十月 2022.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define _MAX_PIN_LEN 32

char *getPIN(int isFirst) {
  struct termios oldt, newt;
  char *tmp = NULL;
  char *pin = calloc(_MAX_PIN_LEN, sizeof(char));

  if (pin == NULL)
    return NULL;

  if (isFirst) {
    printf("请输入PIN：");
  } else {
    printf("请再次输入PIN：");
  }

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  tmp = fgets(pin, _MAX_PIN_LEN, stdin);
  printf("\n");
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  if (tmp == NULL) {
    free(pin);
    return NULL;
  }

  if (pin[strlen(pin) - 1] == '\n')
    pin[strlen(pin) - 1] = '\0';

  return pin;
}

int main() {
  int ret = 0;
  char *pin1 = NULL, *pin2 = NULL;

  pin1 = getPIN(1);
  pin2 = getPIN(0);
  if (!pin1 || !pin2 || strcmp(pin1, pin2)) {
	fprintf(stderr, "两次输入的PIN不一致！\n");
	ret = -1;
	goto out;
  }

  printf("PIN: %s\n", pin1);

out:
  if (pin1)
	free(pin1);
  if (pin2)
	free(pin2);

  return ret;
}
