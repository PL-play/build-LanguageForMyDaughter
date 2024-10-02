//
// Created by ran on 2024/1/13.
//

#ifndef ZCOLLECTION_HASHTABLE_HASH_SET_H_
#define ZCOLLECTION_HASHTABLE_HASH_SET_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void *HashsetKey;

/**
 * Hash function used to generate hash values for keys used in a hash
 * set.
 *
 * @param value  The value to generate a hash value for.
 * @return       The hash value.
 */
typedef unsigned int(*HashsetHashFunc)(HashsetKey hashtable_key);

/**
 * Function used to compare two keys for equality.
 *
 * @return   zero if the two keys are equal
 */
typedef int(*HashsetEqualsFunc)(HashsetKey key1, HashsetKey key2);

/**
 * Type of function used to free keys when entries are removed from a
 * hash string_intern.
 */
typedef void(*HashsetKeyFreeFunc)(HashsetKey key);

/**
 * structure representing an entry in hash set
 */
typedef struct KEntry KEntry;

/**
 * A node in the hash set bucket array.
 */
typedef struct SetNode SetNode;

/**
 * A hash set structure.
 */
typedef struct Hashset Hashset;

typedef struct HashsetEntrySet {
  unsigned int size;
  KEntry **entry_set;
} HashsetEntrySet;

typedef struct HashsetIterator HashsetIterator;

/**
 * Create a new hashset.
 *
 * @param hash_func     Function used to generate hash keys for the keys used in the set.
 * @param equals_func   Function used to test keys used in the string_intern for equality.
 * @return              Pointer of new hash string_intern.NULL if unable to allocate new hash set.
 */
Hashset *new_hash_set(HashsetHashFunc hash_func, HashsetEqualsFunc equals_func);

/**
 * Insert a value into a hash string_intern, overwriting any existing entry
 * using the same key.
 *
 * @param hashset   The hash set
 * @param key       The key for the new value.
 * @return          Non-zero if the value was added successfully,
 *                  or zero if it was not possible to allocate
 *                  memory for the new entry.
 */
int put_hash_set(Hashset *hashset, HashsetKey key);

/**
 * Insert a value into a hash string_intern, overwriting any existing entry
 * using the same key. Free the key parameter if key already existed
 *
 * @param hashset   The hash set
 * @param key       The key for the new value.
 * @return          Non-zero if the value was added successfully,
 *                  or zero if it was not possible to allocate
 *                  memory for the new entry.
 */
int put_free_new_key_hash_set(Hashset *hashset, HashsetKey key, HashsetKeyFreeFunc free_func);

/**
 *
 * @param hashset
 * @param key
 * @return          Non-zero if the value was remove successfully,
 *                  or zero if it was not found in the set.
 */
int remove_hash_set(Hashset *hashset, HashsetKey key);

/**
 *
 * @param hashtable
 * @param key
 * @return
 */
HashsetKey get_key_in_hash_set(Hashset *hashset, HashsetKey key);

/**
 *
 * @param hashset
 * @param key
 * @return
 */
int contains_in_hash_set(Hashset *hashset, HashsetKey key);

/**
 *
 * @param hashset
 * @param key_free_func
 * @param value_free_func
 */
void register_hashset_free_functions(Hashset *hashset, HashsetKeyFreeFunc key_free_func);
HashsetKeyFreeFunc get_hashset_free_function(Hashset *hashset);
/**
 *
 * @param hashset
 * @return
 */
unsigned int size_of_hash_set(Hashset *hashset);

/**
 *
 * @param hashset
 * @param key
 * @return
 */
void remove_with_flag_hash_set(Hashset *hashset, HashsetKey key, int *flag);

/**
 * set load factor
 *
 * @param hashset
 * @param load_factor
 */
void set_load_factor_hash_set(Hashset *hashset, float load_factor);

/**
 *
 * @param hashtable
 */
void free_hash_set(Hashset *hashset);

/**
 *
 * @return
 */
HashsetEntrySet *hashset_entry_set(Hashset *hashset);

/**
 *
 * @param hashtable_entry_set
 */
void free_hashset_entry_set(HashsetEntrySet *hashset_entry_set);

/**
 *
 * @param entry
 * @return
 */
HashsetKey set_entry_key(KEntry *entry);

/**
 * Union two sets
 *
 * @param hashset1
 * @param hashset2
 * @return
 */
Hashset *union_hashset(Hashset *hashset1, Hashset *hashset2);

/**
 * Perform an intersection of two sets.
 *
 * @param hashset1
 * @param hashset2
 * @return
 */
Hashset *intersect_hashset(Hashset *hashset1, Hashset *hashset2);

/**
 * Returns items present only in hashset1
 *
 * @param hashset1
 * @param hashset2
 * @return
 */
Hashset *difference_hashset(Hashset *hashset1, Hashset *hashset2);

/**
 *
 * @param hashtable
 * @return
 */
HashsetIterator *hashset_iterator(Hashset *hashset);

/**
 *
 * @param iter
 * @return
 */
int hashset_iter_has_next(HashsetIterator *iter);

/**
 *
 * @param iter
 * @return
 */
KEntry *hashset_next_entry(HashsetIterator *iter);

/**
 *
 * @param iter
 */
void free_hashset_iter(HashsetIterator *iter);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_HASHTABLE_HASH_SET_H_
