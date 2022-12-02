/**
 * Copyright (C) 2022 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * sm4.h -- implement sm4 algorithm
 *
 * Written on 星期一, 28 十一月 2022. */

#pragma once

#include <stdint.h>

#define SM4_ENCRYPT 0
#define SM4_DECRYPT 1

#define BLOCK_SIZE 16

uint8_t* sm4_ecb_encrypt(uint8_t* key, uint8_t* cleartext, int32_t len);
uint8_t *sm4_ecb_decrypt(uint8_t *key, uint8_t *ciphertext, int32_t len);
uint8_t *sm4_cbc_encrypt(uint8_t *key, uint8_t *iv, uint8_t *cleartext,
                        int32_t len);
uint8_t *sm4_cbc_decrypt(uint8_t *key, uint8_t *iv, uint8_t *ciphertext,
                        int32_t len);
