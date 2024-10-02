#include <limits.h>

#include "hashtable/hash-pointer.h"

uint32_t hash_pointer(void *ptr) {
  // Cast the pointer to an unsigned 64-bit integer (uintptr_t is used for casting pointers to integer types)
  uintptr_t address = (uintptr_t) ptr;

  // Split the 64-bit address into two 32-bit parts
  uint32_t high = (uint32_t) (address >> 32);  // Higher 32 bits
  uint32_t low = (uint32_t) (address & 0xFFFFFFFF);  // Lower 32 bits

  // Combine the high and low parts using a bitwise XOR
  uint32_t hash = high ^ low;

  // Optionally, mix the bits further to reduce potential patterns
  hash ^= (hash >> 16);
  hash *= 0x85ebca6b;
  hash ^= (hash >> 13);
  hash *= 0xc2b2ae35;
  hash ^= (hash >> 16);

  return hash;
}