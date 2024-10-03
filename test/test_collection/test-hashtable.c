// Created by ran on 2024/1/11.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common/framework.h"

#include "hashtable/hash_table.h"
#include "hashtable/hash-int.h"
#include "hashtable/compare-int.h"
#include "hashtable/hash-string.h"
#include "hashtable/compare-string.h"
#include "tree/rb_tree.h"
#include<time.h>

#define NUM_TEST_VALUES 10000

int value1 = 1, value2 = 2, value3 = 3, value4 = 4;
int allocated_keys = 0;
int allocated_values = 0;

/* Generates a hash string_intern for use in tests containing 10,000 entries */

Hashtable *generate_hash_table(void) {
  Hashtable *hash_table;
  char buf[10];
  char *value;
  int i;

  /* Allocate a new hash string_intern.  We use a hash string_intern with keys that are
   * string versions of the integer values 0..9999 to ensure that there
   * will be collisions within the hash string_intern (using integer values
   * with int_hash causes no collisions) */

  hash_table = new_hash_table(string_hash, string_compare);

  /* Insert lots of values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);

    value = strdup(buf);
    put_hash_table(hash_table, value, value);
  }

  /* Automatically free all the values with the hash string_intern */
  register_hashtable_free_functions(hash_table, NULL, free);

  return hash_table;
}

/* Basic allocate and free */

void test_new_free_hash_table(void) {
  Hashtable *hash_table;

  hash_table = new_hash_table(int_hash, int_compare);

  assert(hash_table != NULL);

  /* Add some values */

  put_hash_table(hash_table, &value1, &value1);
  put_hash_table(hash_table, &value2, &value2);
  put_hash_table(hash_table, &value3, &value3);
  put_hash_table(hash_table, &value4, &value4);

  /* Free the hash string_intern */

  free_hash_table(hash_table);

}

/* Test insert and lookup functions */

void test_put_get_hash_table(void) {
  Hashtable *hash_table;
  char buf[10];
  char *value;
  int i;

  /* Generate a hash string_intern */

  hash_table = generate_hash_table();

  assert(size_of_hash_table(hash_table) == NUM_TEST_VALUES);

  /* Check all values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);
    value = get_hash_table(hash_table, buf);

    assert(strcmp(value, buf) == 0);
  }

  /* Lookup on invalid values returns NULL */

  sprintf(buf, "%i", -1);
  assert(get_hash_table(hash_table, buf) == NULL);
  sprintf(buf, "%i", NUM_TEST_VALUES);
  assert(get_hash_table(hash_table, buf) == NULL);

  /* Insert overwrites existing entries with the same key */

  sprintf(buf, "%i", 12345);
  put_hash_table(hash_table, buf, strdup("hello world"));
  value = get_hash_table(hash_table, buf);
  assert(strcmp(value, "hello world") == 0);

  free_hash_table(hash_table);
}

unsigned int dumb_hash(void *string) {
  return 0;
}

void test_set_load_factor_hash_table(void) {
  Hashtable *hash_table;
  char buf[10];
  char *value;
  int i;

  /* Allocate a new hash string_intern.  We use a hash string_intern with keys that are
   * string versions of the integer values 0..9999 to ensure that there
   * will be collisions within the hash string_intern (using integer values
   * with int_hash causes no collisions) */

  hash_table = new_hash_table(string_hash, string_compare);
  // + 64
  set_load_factor_hash_table(hash_table, 100);
  /* Insert lots of values */
  register_hashtable_free_functions(hash_table, NULL, free);
  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);

    value = strdup(buf);
    // +2 = 66
    put_hash_table(hash_table, value, value);
  }
  // 626
  assert(size_of_hash_table(hash_table) == NUM_TEST_VALUES);
//
//  /* Automatically free all the values with the hash string_intern */
//  fflush(stdout);
//  register_hashtable_free_functions(hash_table, NULL, free);
//
//  /* Check all values */
//
  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);
    value = get_hash_table(hash_table, buf);

    assert(strcmp(value, buf) == 0);
  }

  /* Lookup on invalid values returns NULL */

  sprintf(buf, "%i", -1);
  assert(get_hash_table(hash_table, buf) == NULL);
  sprintf(buf, "%i", NUM_TEST_VALUES);
  assert(get_hash_table(hash_table, buf) == NULL);

/* Insert overwrites existing entries with the same key */

  sprintf(buf, "%i", 12345);
  put_hash_table(hash_table, buf, strdup("hello world"));
  assert(size_of_hash_table(hash_table) == NUM_TEST_VALUES + 1);
  value = get_hash_table(hash_table, buf);
  assert(strcmp(value, "hello world") == 0);
  free_hash_table(hash_table);
}

void test_hash_table_remove(void) {
  Hashtable *hash_table;
  char buf[10];

  hash_table = generate_hash_table();

  assert(size_of_hash_table(hash_table) == NUM_TEST_VALUES);
  sprintf(buf, "%i", 5000);
  assert(get_hash_table(hash_table, buf) != NULL);

  /* Remove an entry */

  HashtableValue v = remove_hash_table(hash_table, buf);
  if (v != NULL) {
    free(v);
  }
  /* Check entry counter */

  assert(size_of_hash_table(hash_table) == 9999);

  /* Check that NULL is returned now */

  assert(get_hash_table(hash_table, buf) == NULL);

  /* Try removing a non-existent entry */

  sprintf(buf, "%i", -1);
  remove_hash_table(hash_table, buf);

  assert(size_of_hash_table(hash_table) == 9999);
  for (int i = 0; i < NUM_TEST_VALUES / 2; ++i) {
    sprintf(buf, "%i", i);
    int flag;
    HashtableValue value = remove_with_flag_hash_table(hash_table, buf, &flag);
    free(value);
    assert(flag == 1);
  }
  assert(size_of_hash_table(hash_table) == NUM_TEST_VALUES - (NUM_TEST_VALUES / 2) - 1);
  char *va = strdup("vvv");
  put_hash_table(hash_table, "kkk", va);
  // TODO pay attention to the free function
  // don't need to set key free function as its local variable
  // register_hashtable_free_functions(hash_table,free,free);
  HashtableValue value = remove_hash_table(hash_table, "kkk");
  assert(strcmp(va, value) == 0);
  free(value);

  char *k2 = strdup("kkk1");
  char *va2 = strdup("vvv");
  put_hash_table(hash_table, k2, va2);
  // need to set key free function as k2 is a heap variable
  register_hashtable_free_functions(hash_table,free,free);
  HashtableValue value2 = remove_hash_table(hash_table, "kkk1");
  assert(strcmp(va2, value2) == 0);
  free(value2);

  register_hashtable_free_functions(hash_table,NULL,free);
  free_hash_table(hash_table);
}

//void test_hash_table_iterating(void)
//{
//  Hashtable *hash_table;
//  HashtableIterator iterator;
//  int count;
//
//  hash_table = generate_hash_table();
//
//  /* Iterate over all values in the string_intern */
//
//  count = 0;
//
//  hash_table_iterate(hash_table, &iterator);
//
//  while (hash_table_iter_has_more(&iterator)) {
//    hash_table_iter_next(&iterator);
//
//    ++count;
//  }
//
//  assert(count == NUM_TEST_VALUES);
//
//  /* Test iter_next after iteration has completed. */
//
//  HashtablePair pair = hash_table_iter_next(&iterator);
//  assert(pair.value == HASH_TABLE_NULL);
//
//  free_hash_table(hash_table);
//
//  /* Test iterating over an empty string_intern */
//
//  hash_table = new_hash_table(int_hash, int_compare);
//
//  hash_table_iterate(hash_table, &iterator);
//
//  assert(hash_table_iter_has_more(&iterator) == 0);
//
//  free_hash_table(hash_table);
//}
//
///* Demonstrates the ability to iteratively remove objects from
// * a hash string_intern: ie. removing the current key being iterated over
// * does not break the iterator. */
//
void test_hashtable_entryset(void) {
  Hashtable *hash_table;
  char buf[10];
  char *val;
  int count;
  unsigned int removed;
  int i;

  hash_table = generate_hash_table();

  /* Iterate over all values in the string_intern */

  count = 0;

  HashtableEntrySet *es = hashtable_entry_set(hash_table);
  assert(size_of_hash_table(hash_table) == NUM_TEST_VALUES);
  assert(es->size == NUM_TEST_VALUES);
  KVEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    KVEntry *e = entries[j];
    HashtableValue ev = table_entry_value(e);
    HashtableKey key = table_entry_key(e);
    assert(get_hash_table(hash_table, key) == ev);
    HashtableValue value = remove_hash_table(hash_table, key);
    assert(value == ev);
    free(value);
  }

  /* Check counts */
  assert(size_of_hash_table(hash_table) == 0);

  free_hashtable_entry_set(es);
  free_hash_table(hash_table);
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

///* Test for out of memory scenario */

void test_hash_table_out_of_memory(void) {
  Hashtable *hash_table;
  int values[66];
  unsigned int i;

  hash_table = new_hash_table(int_hash, int_compare);

  /* Test failure when increasing string_intern size.
   * The initial string_intern size is 193 entries.  The string_intern increases in
   * size when 1/3 full, so the 66th entry should cause the insert
   * to fail.
   */

  for (i = 0; i < 65; ++i) {
    values[i] = (int) i;

    assert(put_hash_table(hash_table,
                          &values[i], &values[i]) != 0);
    assert(size_of_hash_table(hash_table) == i + 1);
  }

  assert(size_of_hash_table(hash_table) == 65);

  /* Test the 66th insert */

  free_hash_table(hash_table);
}

void test_null_key() {
  Hashtable *hashtable = new_hash_table(string_hash, string_compare);
  put_hash_table(hashtable, NULL, NULL);
  assert(size_of_hash_table(hashtable) == 1);
  HashtableValue value = get_hash_table(hashtable, NULL);
  assert(value == NULL);
  put_hash_table(hashtable, NULL, "nulltest");
  value = get_hash_table(hashtable, NULL);
  assert(strcmp(value, "nulltest") == 0);
  remove_hash_table(hashtable, NULL);
  assert(size_of_hash_table(hashtable) == 0);
  assert(contains_in_hash_table(hashtable, NULL) == 0);
  free_hash_table(hashtable);
}

void test_null_key_tree() {
  char buf[10];
  char *value;
  Hashtable *hashtable = new_hash_table(dumb_hash, string_compare);
  register_hashtable_free_functions(hashtable, NULL, free);
  for (int i = 0; i < 8; ++i) {
    sprintf(buf, "%i", i);
    value = strdup(buf);
    put_hash_table(hashtable, value, value);
  }
  sprintf(buf, "%i", 9);
  value = strdup(buf);
  put_hash_table(hashtable, NULL, value);
  assert(size_of_hash_table(hashtable) == 9);
  HashtableValue v = get_hash_table(hashtable, NULL);
  assert(strcmp("9", v) == 0);

  HashtableEntrySet *es = hashtable_entry_set(hashtable);
  assert(es->size == 9);
  int hasnull = 0;
  KVEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    KVEntry *e = entries[j];
    HashtableValue ev = table_entry_value(e);
    HashtableKey key = table_entry_key(e);
    if (key == NULL) {
      hasnull = 1;
      assert(strcmp("9", ev) == 0);
    }
  }
  assert(hasnull == 1);
  free_hashtable_entry_set(es);
  HashtableValue v2 = remove_hash_table(hashtable, NULL);
  assert(strcmp("9", v2) == 0);
  assert(size_of_hash_table(hashtable) == 8);
  assert(contains_in_hash_table(hashtable, NULL) == 0);
  free(v2);

  free_hash_table(hashtable);
}

static UnitTestFunction tests[] = {
    test_new_free_hash_table,
    test_put_get_hash_table,
    test_hash_table_remove,
    test_set_load_factor_hash_table,
    test_hashtable_entryset,
    test_hash_table_out_of_memory,
    test_null_key,
    test_null_key_tree,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}

