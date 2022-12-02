/**
 * Copyright (C) 2022 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * sm4.c -- implement sm4 algorithm
 *
 * Written on 星期一, 28 十一月 2022.
 */

#include "sm4.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void print_bytes(const char *msg, const uint8_t *data, int len) {
  int i = 0;

  printf("%s: ", msg);
  for (; i < len; i++)
    printf("%02x", data[i] & 0xFF);
  printf("\n");
}

static inline uint8_t sbox(uint8_t num) {
  /**
   * From GM/T 0002-2012 SM4
   *
   * SBOX
   **/
  static uint8_t s_box[] = {
      0xD6, 0x90, 0xE9, 0xFE, 0xCC, 0xE1, 0x3D, 0xB7, 0x16, 0xB6, 0x14, 0xC2,
      0x28, 0xFB, 0x2C, 0x05, 0x2B, 0x67, 0x9A, 0x76, 0x2A, 0xBE, 0x04, 0xC3,
      0xAA, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99, 0x9C, 0x42, 0x50, 0xF4,
      0x91, 0xEF, 0x98, 0x7A, 0x33, 0x54, 0x0B, 0x43, 0xED, 0xCF, 0xAC, 0x62,
      0xE4, 0xB3, 0x1C, 0xA9, 0xC9, 0x08, 0xE8, 0x95, 0x80, 0xDF, 0x94, 0xFA,
      0x75, 0x8F, 0x3F, 0xA6, 0x47, 0x07, 0xA7, 0xFC, 0xF3, 0x73, 0x17, 0xBA,
      0x83, 0x59, 0x3C, 0x19, 0xE6, 0x85, 0x4F, 0xA8, 0x68, 0x6B, 0x81, 0xB2,
      0x71, 0x64, 0xDA, 0x8B, 0xF8, 0xEB, 0x0F, 0x4B, 0x70, 0x56, 0x9D, 0x35,
      0x1E, 0x24, 0x0E, 0x5E, 0x63, 0x58, 0xD1, 0xA2, 0x25, 0x22, 0x7C, 0x3B,
      0x01, 0x21, 0x78, 0x87, 0xD4, 0x00, 0x46, 0x57, 0x9F, 0xD3, 0x27, 0x52,
      0x4C, 0x36, 0x02, 0xE7, 0xA0, 0xC4, 0xC8, 0x9E, 0xEA, 0xBF, 0x8A, 0xD2,
      0x40, 0xC7, 0x38, 0xB5, 0xA3, 0xF7, 0xF2, 0xCE, 0xF9, 0x61, 0x15, 0xA1,
      0xE0, 0xAE, 0x5D, 0xA4, 0x9B, 0x34, 0x1A, 0x55, 0xAD, 0x93, 0x32, 0x30,
      0xF5, 0x8C, 0xB1, 0xE3, 0x1D, 0xF6, 0xE2, 0x2E, 0x82, 0x66, 0xCA, 0x60,
      0xC0, 0x29, 0x23, 0xAB, 0x0D, 0x53, 0x4E, 0x6F, 0xD5, 0xDB, 0x37, 0x45,
      0xDE, 0xFD, 0x8E, 0x2F, 0x03, 0xFF, 0x6A, 0x72, 0x6D, 0x6C, 0x5B, 0x51,
      0x8D, 0x1B, 0xAF, 0x92, 0xBB, 0xDD, 0xBC, 0x7F, 0x11, 0xD9, 0x5C, 0x41,
      0x1F, 0x10, 0x5A, 0xD8, 0x0A, 0xC1, 0x31, 0x88, 0xA5, 0xCD, 0x7B, 0xBD,
      0x2D, 0x74, 0xD0, 0x12, 0xB8, 0xE5, 0xB4, 0xB0, 0x89, 0x69, 0x97, 0x4A,
      0x0C, 0x96, 0x77, 0x7E, 0x65, 0xB9, 0xF1, 0x09, 0xC5, 0x6E, 0xC6, 0x84,
      0x18, 0xF0, 0x7D, 0xEC, 0x3A, 0xDC, 0x4D, 0x20, 0x79, 0xEE, 0x5F, 0x3E,
      0xD7, 0xCB, 0x39, 0x48};

  return s_box[num & 0xFF];
}

// 循环左移 n 位
static inline uint32_t lshift(uint32_t x, uint32_t n) {
  return (x >> (32 - (n % 32))) | (x << (n % 32));
}

// L 变换: L(B) = B ^ (B <<< 2) ^ (B <<< 10) ^ (B <<< 18) ^ (B <<< 24)
// 用于加解密
static inline uint32_t linear(uint32_t n) {
  return n ^ lshift(n, 2) ^ lshift(n, 10) ^ lshift(n, 18) ^ lshift(n, 24);
}

// L' 变换: L'(B) = B ^ (B <<< 13) ^ (B <<< 23)
// 用于密钥扩展
static inline uint32_t linear1(uint32_t n) {
  return n ^ lshift(n, 13) ^ lshift(n, 23);
}

// 非线性变换 τ(.): B = τ(A) = (sbox(a0), sbox(a1), sbox(a2), sbox(a3))
// A: int32
// a0 ~ a3: int8
// τ(A) = (sbox(A >> 24 & 0xFF) << 24) ^ (sbox(A >> 16 & 0xFF) << 16) ^
//         (sbox(A >> 8 & 0xFF) << 8) ^ sbox(A & 0xFF)
static inline uint32_t nonlinear_r(uint32_t n) {
  return ((uint32_t)(sbox(n >> 24 & 0xFF) << 24) ^
          (uint32_t)(sbox(n >> 16 & 0xFF) << 16) ^
          (uint32_t)(sbox(n >> 8 & 0xFF) << 8) ^ (uint32_t)sbox(n & 0xFF));
}

// F(x0, x1, x2, x3, rk) = x0 ^ T(x1, x2, x3, rk)
// T(x1, x2, x3, rk) = L(τ(x1 ^ x2 ^ x3 ^ rk))
// 用于加解密，使用线性变换 L
static inline uint32_t feistel(uint32_t x0, uint32_t x1, uint32_t x2,
                               uint32_t x3, uint32_t rk) {
  return x0 ^ linear(nonlinear_r(x1 ^ x2 ^ x3 ^ rk));
}

// F(x0, x1, x2, x3, rk) = x0 ^ T'(x1, x2, x3, rk)
// T'(x1, x2, x3, rk) = L'(τ(x1 ^ x2 ^ x3 ^ rk))
// 用于密钥扩展，使用线性变换 L'
static inline uint32_t feistel1(uint32_t x0, uint32_t x1, uint32_t x2,
                                uint32_t x3, uint32_t rk) {
  return x0 ^ linear1(nonlinear_r(x1 ^ x2 ^ x3 ^ rk));
}

// 4 bytes to int32
static inline uint32_t bytes2int(const uint8_t *data) {
  return (uint32_t)(((data[0] & 0xFF) << 24) | ((data[1] & 0xFF) << 16) |
                    ((data[2] & 0xFF) << 8) | (data[3] & 0xFF));
}

// int32 to 4 bytes
static inline void int2bytes(const uint32_t num, uint8_t *data) {
  data[0] = ((uint8_t)(num >> 24)) & 0xFF;
  data[1] = ((uint8_t)(num >> 16)) & 0xFF;
  data[2] = ((uint8_t)(num >> 8)) & 0xFF;
  data[3] = ((uint8_t)num) & 0xFF;
}

// 16 bytes to 4 int32
static inline void split_bytes(const uint8_t *data, uint32_t *array) {
  int i = 0;

  for (; i < 4; i++)
    array[i] = bytes2int(data + (i * 4));
}

// 密钥扩展
static void generate_subkeys(uint8_t *key, uint32_t *rk) {
  /**
   * From GM/T 0002-2012 SM4
   *
   * 系统参数
   **/
  static const uint32_t fk[] = {0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC};
  /**
   * From GM/T 0002-2012 SM4
   *
   * 固定参数
   **/
  static const uint32_t ck[] = {
      0x00070E15, 0x1C232A31, 0x383F464D, 0x545B6269, 0x70777E85, 0x8C939AA1,
      0xA8AFB6BD, 0xC4CBD2D9, 0xE0E7EEF5, 0xFC030A11, 0x181F262D, 0x343B4249,
      0x50575E65, 0x6C737A81, 0x888F969D, 0xA4ABB2B9, 0xC0C7CED5, 0xDCE3EAF1,
      0xF8FF060D, 0x141B2229, 0x30373E45, 0x4C535A61, 0x686F767D, 0x848B9299,
      0xA0A7AEB5, 0xBCC3CAD1, 0xD8DFE6ED, 0xF4FB0209, 0x10171E25, 0x2C333A41,
      0x484F565D, 0x646B7279};
  uint32_t key_tmp[4] = {0};
  int i = 0;

  // 将密钥转为 4 个 int32，并与 fk 异或
  for (; i < 4; i++) {
    key_tmp[i] = bytes2int(key + (i * 4));
    key_tmp[i] ^= fk[i];
  }

  // 32 轮密钥拓展
  for (i = 0; i < 32; i++) {
    rk[i] = feistel1(key_tmp[0], key_tmp[1], key_tmp[2], key_tmp[3], ck[i]);
    key_tmp[0] = key_tmp[1];
    key_tmp[1] = key_tmp[2];
    key_tmp[2] = key_tmp[3];
    key_tmp[3] = rk[i];
  }
}

// 块加解密
static void crypt_block(uint8_t *key, uint8_t *input, int mode,
                        uint8_t *output) {
  uint32_t xarr[4] = {0};
  uint32_t rk[32] = {0};
  int i = 0, idx = 0;
  uint32_t tmp = 0;

  // 拓展密钥
  generate_subkeys(key, rk);

  // 转换为 4 个 int32
  split_bytes(input, xarr);

  for (; i < 32; i++) {
    // 根据加解密模式，修改 rk 顺序
    idx = (mode == SM4_ENCRYPT) ? i : (31 - i);
    tmp = feistel(xarr[0], xarr[1], xarr[2], xarr[3], rk[idx]);
    xarr[0] = xarr[1];
    xarr[1] = xarr[2];
    xarr[2] = xarr[3];
    xarr[3] = tmp;
  }

  for (i = 0; i < 4; i++) {
    int2bytes(xarr[3 - i], output + (i * 4));
  }
}

static uint8_t *sm4_ecb(uint8_t *key, uint8_t *data, int32_t len, int mode) {
  uint8_t *target = NULL;
  int i = 0;

  target = calloc(len, sizeof(uint8_t));
  if (!target)
    return NULL;

  for (; i < len / BLOCK_SIZE; i++)
    crypt_block(key, data + (i * 16), mode, target + (i * 16));

  return target;
}

uint8_t *sm4_ecb_encrypt(uint8_t *key, uint8_t *cleartext, int32_t len) {
  return sm4_ecb(key, cleartext, len, SM4_ENCRYPT);
}

uint8_t *sm4_ecb_decrypt(uint8_t *key, uint8_t *ciphertext, int32_t len) {
  return sm4_ecb(key, ciphertext, len, SM4_DECRYPT);
}

uint8_t *sm4_cbc_encrypt(uint8_t *key, uint8_t *iv, uint8_t *cleartext,
                         int32_t len) {
  return NULL;
}

uint8_t *sm4_cbc_decrypt(uint8_t *key, uint8_t *iv, uint8_t *ciphertext,
                         int32_t len) {
  return NULL;
}
