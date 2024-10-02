//
// Created by ran on 2024/6/27.
//

#ifndef ZCOLLECTION_HASHTABLE_SHA256_H_
#define ZCOLLECTION_HASHTABLE_SHA256_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  uint8_t data[64];
  uint32_t datalen;
  uint64_t bitlen;
  uint32_t state[8];
} SHA256_CTX;

/**
 * Initializes the SHA-256 context.
 *
 * @param ctx
 */
void sha256_init(SHA256_CTX *ctx);
/**
 * Processes a 512-bit chunk of the message.
 *
 * @param ctx
 * @param data
 */
void sha256_transform(SHA256_CTX *ctx, const uint8_t *data);

/**
 * Updates the SHA-256 context with a portion of the message.
 *
 * @param ctx
 * @param data
 * @param len
 */
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);

/**
 * Finalizes the SHA-256 hash and produces the output.
 *
 * @param ctx
 * @param hash
 */
void sha256_final(SHA256_CTX *ctx, uint8_t *hash);

/**
 *
 * @param hash uint8_t [32]
 * @param len 32
 * @param dest char[65]
 */
void stringify_hash(uint8_t *hash, size_t len, char *dest);

void print_hash(uint8_t *hash, size_t len);

/**
 *
 * @param file
 * @param hash uint8_t [32]
 * @return
 */
void sha256_file(FILE *file, uint8_t *hash);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_HASHTABLE_SHA256_H_
