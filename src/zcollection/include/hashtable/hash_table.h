//
// Created by ran on 2024/1/10.
//
/**
 * A hashtable, also known as a hash map, is a data structure that implements an associative array,
 * also called a dictionary, which is an abstract data type that maps keys to values. A hashtable
 * uses a hash function to compute an index, also called a hash code, into an array of buckets or
 * slots, from which the desired value can be found. During lookup, the key is hashed and the
 * resulting hash indicates where the corresponding value is stored.
 *
 * This hashtable uses linked list and red-black tree to store the data if there is hash conflict.
 * If hash conflict number is greater than the threshold(default 8), it will turn to red black tree.
 * Also, the 'NULL' key is supported and always stored in the index 0 slot.
 *
 */
#ifndef ZCOLLECTION_HASHTABLE_HASH_TABLE_H_
#define ZCOLLECTION_HASHTABLE_HASH_TABLE_H_
#ifdef __cplusplus
extern "C" {
#endif
/**
 * A key to look up a value in a @ref HashTable.
 */
typedef void *HashtableKey;

/**
 * A value stored in a @ref HashTable.
 */
typedef void *HashtableValue;

/**
 * Hash function used to generate hash values for keys used in a hash
 * string_intern.
 *
 * @param value  The value to generate a hash value for.
 * @return       The hash value.
 */
typedef unsigned int(*HashtableHashFunc)(HashtableKey hashtable_key);

/**
 * Function used to compare two keys for equality.
 *
 * @return   zero if the two keys are equal
 */
typedef int(*HashtableEqualsFunc)(HashtableKey key1, HashtableKey key2);

/**
 * Type of function used to free keys when entries are removed from a
 * hash string_intern.
 */
typedef void(*HashtableKeyFreeFunc)(HashtableKey key);

/**
 * Type of function used to free values when entries are removed from a
 * hash string_intern.
 */
typedef void(*HashtableValueFreeFunc)(HashtableValue value);

/**
 * structure representing an entry in hash string_intern
 */
typedef struct KVEntry KVEntry;

/**
 * A node in the hash string_intern bucket array.
 */
typedef struct TableNode TableNode;

/**
 * A hash string_intern structure.
 */
typedef struct Hashtable Hashtable;

typedef struct HashtableEntrySet {
  unsigned int size;
  KVEntry **entry_set;
} HashtableEntrySet;

typedef struct HashtableIterator HashtableIterator;

/**
 * Create a new hashtable.
 *
 * @param hash_func     Function used to generate hash keys for the keys used in the string_intern.
 * @param equals_func   Function used to test keys used in the string_intern for equality.
 * @return              Pointer of new hash string_intern.NULL if unable to allocate new hash string_intern.
 */
Hashtable *new_hash_table(HashtableHashFunc hash_func, HashtableEqualsFunc equals_func);

/**
 * Insert a value into a hash string_intern, overwriting any existing entry
 * using the same key.
 *
 * @param hashtable The hash string_intern
 * @param key       The key for the new value.
 * @param value     The value to insert.
 * @return          Non-zero if the value was added successfully,
 *                  or zero if it was not possible to allocate
 *                  memory for the new entry.
 */
int put_hash_table(Hashtable *hashtable, HashtableKey key, HashtableValue value);

/**
 * Insert a value into a hash string_intern, overwriting any existing entry
 * using the same key and free the exist value
 *
 * @param hashtable The hash string_intern
 * @param key       The key for the new value.
 * @param value     The value to insert.
 * @return          Non-zero if the value was added successfully,
 *                  or zero if it was not possible to allocate
 *                  memory for the new entry.
 */
int put_free_exist_hash_table(Hashtable *hashtable,
                              HashtableKey key,
                              HashtableValue value,
                              void(*free_func)(HashtableValue));

/**
 *
 * @param hashtable
 * @param key_free_func
 * @param value_free_func
 */
void register_hashtable_free_functions(Hashtable *hashtable,
                                       HashtableKeyFreeFunc key_free_func,
                                       HashtableValueFreeFunc value_free_func);

/**
 *
 * @param hashtable
 * @return
 */
unsigned int size_of_hash_table(Hashtable *hashtable);

/**
 *
 * @param hashtable
 * @param key
 * @return
 */
HashtableValue get_hash_table(Hashtable *hashtable, HashtableKey key);

/**
 * get hash string_intern. if key not exist, return the default value.
 *
 * @param hashtable
 * @param key
 * @param default_value
 * @return
 */
HashtableValue get_default_hash_table(Hashtable *hashtable, HashtableKey key, HashtableValue default_value);

/**
 *
 * @param hashtable
 * @param key
 * @return
 */
int contains_in_hash_table(Hashtable *hashtable, HashtableKey key);

/**
 *
 * @param hashtable
 * @param key
 * @return
 */
HashtableValue remove_hash_table(Hashtable *hashtable, HashtableKey key);

/**
 *
 * @param hashtable
 * @param key
 * @param flag
 * @return
 */
HashtableValue remove_with_flag_hash_table(Hashtable *hashtable, HashtableKey key, int *flag);

/**
 * set load factor
 *
 * @param hashtable
 * @param load_factor
 */
void set_load_factor_hash_table(Hashtable *hashtable, float load_factor);

/**
 *
 * @param hashtable
 */
void free_hash_table(Hashtable *hashtable);

/**
 *
 * @return
 */
HashtableEntrySet *hashtable_entry_set(Hashtable *hashtable);

/**
 *
 * @param hashtable_entry_set
 */
void free_hashtable_entry_set(HashtableEntrySet *hashtable_entry_set);

/**
 *
 * @param entry
 * @return
 */
HashtableKey table_entry_key(KVEntry *entry);

/**
 *
 * @param entry
 * @return
 */
HashtableValue table_entry_value(KVEntry *entry);

/**
 *
 * @param hashtable
 * @return
 */
HashtableIterator *hashtable_iterator(Hashtable *hashtable);

/**
 *
 * @param iter
 * @return
 */
int hashtable_iter_has_next(HashtableIterator *iter);

/**
 *
 * @param iter
 * @return
 */
KVEntry *hashtable_next_entry(HashtableIterator *iter);

/**
 *
 * @param iter
 */
void free_hashtable_iter(HashtableIterator *iter);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_HASHTABLE_HASH_TABLE_H_
