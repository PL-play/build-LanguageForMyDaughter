/**
 * @file hash-pointer.h
 *
 * Hash function for a generic (void) pointer.  See @ref pointer_hash.
 */

#ifndef ALGORITHM_HASH_POINTER_H
#define ALGORITHM_HASH_POINTER_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generate a hash key for a pointer.
 *
 * @param ptr
 * @return
 */
uint32_t hash_pointer(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef ALGORITHM_HASH_POINTER_H */

