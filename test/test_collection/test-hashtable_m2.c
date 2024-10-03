// Created by ran on 2024/1/11.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common/framework.h"
#include "hashtable/hash_table_m.h"
#include <time.h>
#include <bits/stdint-uintn.h>
#include <stddef.h>

DECLARE_HASHTABLE(int, int, Int_)

DEFINE_HASHTABLE(int, -1, Int_)

void key_free(Int_HashtableKey k) {

}

uint32_t int_hash(Int_HashtableKey key) {
  return key;
}

int int_compare(Int_HashtableKey key1, Int_HashtableKey key2) {
  if (key1 == key2) {
    return 0;
  } else if (key1 > key2) {
    return 1;
  } else {
    return -1;
  }
}

#define NUM_TEST_VALUES 10000

int value1 = 1, value2 = 2, value3 = 3, value4 = 4;
int allocated_keys = 0;
int allocated_values = 0;

/* Generates a hash string_intern for use in tests containing 10,000 entries */

Int_Hashtable *generate_hash_table(void) {
  Int_Hashtable *hash_table;
  int i;

  /* Allocate a new hash string_intern.  We use a hash string_intern with keys that are
   * string versions of the integer values 0..9999 to ensure that there
   * will be collisions within the hash string_intern (using integer values
   * with int_hash causes no collisions) */

  hash_table = Int_new_hash_table(int_hash, int_compare);

  /* Insert lots of values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    Int_put_hash_table(hash_table, i, i * i);
  }

  /* Automatically free all the values with the hash string_intern */
  Int_register_hashtable_free_functions(hash_table, NULL, NULL);

  return hash_table;
}

/* Basic allocate and free */

void test_new_free_hash_table(void) {
  Int_Hashtable *hash_table;

  hash_table = Int_new_hash_table(int_hash, int_compare);

  assert(hash_table != NULL);

  /* Add some values */

  Int_put_hash_table(hash_table, value1, value1 * value1);
  Int_put_hash_table(hash_table, value2, value2 * value2);
  Int_put_hash_table(hash_table, value3, value3 * value3);
  Int_put_hash_table(hash_table, value4, value4 * value4);

  /* Free the hash string_intern */

  Int_free_hash_table(hash_table);

}

/* Test insert and lookup functions */

void test_put_get_hash_table(void) {
  Int_Hashtable *hash_table;
  int i;

  /* Generate a hash string_intern */

  hash_table = generate_hash_table();

  assert(Int_size_of_hash_table(hash_table) == NUM_TEST_VALUES);

  /* Check all values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    int value = Int_get_hash_table(hash_table, i);

    assert(value == i * i);
  }

  /* Lookup on invalid values returns NULL */

  assert(Int_get_hash_table(hash_table, i) == -1);

  /* Insert overwrites existing entries with the same key */

  Int_put_hash_table(hash_table, 12345, 12345);
  int value = Int_get_hash_table(hash_table, 12345);
  assert(value == 12345);

  Int_free_hash_table(hash_table);
}

unsigned int dumb_hash(void *string) {
  return 0;
}

void test_set_load_factor_hash_table(void) {
  Int_Hashtable *hash_table;
  int i;

  /* Allocate a new hash string_intern.  We use a hash string_intern with keys that are
   * string versions of the integer values 0..9999 to ensure that there
   * will be collisions within the hash string_intern (using integer values
   * with int_hash causes no collisions) */

  hash_table = Int_new_hash_table(int_hash, int_compare);
  // + 64
  Int_set_load_factor_hash_table(hash_table, 100);
  /* Insert lots of values */
  Int_register_hashtable_free_functions(hash_table, NULL, NULL);
  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    Int_put_hash_table(hash_table, i, i*i);
  }
  // 626
  assert(Int_size_of_hash_table(hash_table) == NUM_TEST_VALUES);
//
//  /* Automatically free all the values with the hash string_intern */
//  fflush(stdout);
//  Int_register_hashtable_free_functions(hash_table, NULL, free);
//
//  /* Check all values */
//
  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    int value = Int_get_hash_table(hash_table, i);

    assert(value==i*i);
  }

  /* Lookup on invalid values returns NULL */

  assert(Int_get_hash_table(hash_table, -1) == -1);

/* Insert overwrites existing entries with the same key */

  Int_put_hash_table(hash_table, 12345, 12345);
  assert(Int_size_of_hash_table(hash_table) == NUM_TEST_VALUES + 1);
  int value = Int_get_hash_table(hash_table, 12345);
  assert(value==12345);
  Int_free_hash_table(hash_table);
}

void test_hash_table_remove(void) {
  Int_Hashtable *hash_table;

  hash_table = generate_hash_table();

  assert(Int_size_of_hash_table(hash_table) == NUM_TEST_VALUES);
  assert(Int_get_hash_table(hash_table, 5000) != -1);

  /* Remove an entry */

  Int_HashtableValue v = Int_remove_hash_table(hash_table, 5000);

  /* Check entry counter */

  assert(Int_size_of_hash_table(hash_table) == 9999);

  /* Check that NULL is returned now */

  assert(Int_get_hash_table(hash_table, 5000) == -1);

  /* Try removing a non-existent entry */

  Int_remove_hash_table(hash_table, 5000);

  assert(Int_size_of_hash_table(hash_table) == 9999);
  for (int i = 0; i < NUM_TEST_VALUES / 2; ++i) {
    int flag;
    Int_HashtableValue value = Int_remove_with_flag_hash_table(hash_table, i, &flag);
    assert(flag == 1);
  }
  assert(Int_size_of_hash_table(hash_table) == NUM_TEST_VALUES - (NUM_TEST_VALUES / 2) - 1);
  Int_put_hash_table(hash_table, 767676, 767676);
  // TODO pay attention to the free function
  // don't need to set key free function as its local variable
  // Int_register_hashtable_free_functions(hash_table,free,free);
  Int_HashtableValue value = Int_remove_hash_table(hash_table, 767676);
  assert(value==767676);

  Int_register_hashtable_free_functions(hash_table, NULL, NULL);
  Int_free_hash_table(hash_table);
}

void test_hashtable_entryset(void) {
  Int_Hashtable *hash_table;
  char buf[10];
  char *val;
  int count;
  unsigned int removed;
  int i;

  hash_table = generate_hash_table();

  /* Iterate over all values in the string_intern */

  count = 0;

  Int_HashtableEntrySet *es = Int_hashtable_entry_set(hash_table);
  assert(Int_size_of_hash_table(hash_table) == NUM_TEST_VALUES);
  assert(es->size == NUM_TEST_VALUES);
  Int_KVEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    Int_KVEntry *e = entries[j];
    Int_HashtableValue ev = Int_table_entry_value(e);
    Int_HashtableKey key = Int_table_entry_key(e);
    assert(Int_get_hash_table(hash_table, key) == ev);
    Int_HashtableValue value = Int_remove_hash_table(hash_table, key);
    assert(value == ev);
  }

  /* Check counts */
  assert(Int_size_of_hash_table(hash_table) == 0);

  Int_free_hashtable_entry_set(es);
  Int_free_hash_table(hash_table);
}

/* Create a new key */

int *new_key(int value) {
  int *result;

  result = malloc(sizeof(int));
  *result = value;

  ++allocated_keys;

  return result;
}

/* Callback function invoked when a key is freed */

void free_key(void *key) {
  free(key);

  --allocated_keys;
}

/* Create a new value */

int *new_value(int value) {
  int *result;

  result = malloc(sizeof(int));
  *result = value;

  ++allocated_values;

  return result;
}

/* Callback function invoked when a value is freed */

void free_value(void *value) {
  free(value);

  --allocated_values;
}



static UnitTestFunction tests[] = {
    test_new_free_hash_table,
    test_put_get_hash_table,
    test_hash_table_remove,
    test_set_load_factor_hash_table,
    test_hashtable_entryset,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}

