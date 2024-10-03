// Created by ran on 2024/1/14.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common/framework.h"

#include "hashtable/hash_set.h"
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

Hashset *generate_hash_set(void) {
  Hashset *hash_set;
  char buf[10];
  char *value;
  int i;

  /* Allocate a new hash string_intern.  We use a hash string_intern with keys that are
   * string versions of the integer values 0..9999 to ensure that there
   * will be collisions within the hash string_intern (using integer values
   * with int_hash causes no collisions) */

  hash_set = new_hash_set(string_hash, string_compare);

  /* Insert lots of values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);

    value = strdup(buf);
    put_hash_set(hash_set, value);
  }

  /* Automatically free all the values with the hash string_intern */
  register_hashset_free_functions(hash_set, free);

  return hash_set;
}

/* Basic allocate and free */

void test_new_free_hash_set(void) {
  Hashset *hash_set;

  hash_set = new_hash_set(int_hash, int_compare);

  assert(hash_set != NULL);

  /* Add some values */

  put_hash_set(hash_set, &value1);
  put_hash_set(hash_set, &value2);
  put_hash_set(hash_set, &value3);
  put_hash_set(hash_set, &value4);

  /* Free the hash string_intern */

  free_hash_set(hash_set);

}

/* Test insert and lookup functions */

void test_put_contains_in_hash_set(void) {
  Hashset *hash_set;
  char buf[10];
  char *value;
  int i;

  /* Generate a hash string_intern */

  hash_set = generate_hash_set();

  assert(size_of_hash_set(hash_set) == NUM_TEST_VALUES);

  /* Check all values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);
    int c = contains_in_hash_set(hash_set, buf);
    assert(c == 1);
  }

  /* Lookup on invalid values returns NULL */

  sprintf(buf, "%i", -1);
  assert(contains_in_hash_set(hash_set, buf) == 0);
  sprintf(buf, "%i", NUM_TEST_VALUES);
  assert(contains_in_hash_set(hash_set, buf) == 0);

  /* Insert overwrites existing entries with the same key */

  sprintf(buf, "%i", 12345);
  value = strdup(buf);
  put_hash_set(hash_set, value);
  assert(contains_in_hash_set(hash_set, value) == 1);

  free_hash_set(hash_set);
}

unsigned int dumb_hash(void *string) {
  return 0;
}

void test_set_load_factor_hash_set(void) {
  Hashset *hash_set;
  char buf[10];
  char *value;
  int i;

  /* Allocate a new hash string_intern.  We use a hash string_intern with keys that are
   * string versions of the integer values 0..9999 to ensure that there
   * will be collisions within the hash string_intern (using integer values
   * with int_hash causes no collisions) */

  hash_set = new_hash_set(string_hash, string_compare);
  // + 64
  set_load_factor_hash_set(hash_set, 100);
  /* Insert lots of values */
  register_hashset_free_functions(hash_set, free);
  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);

    value = strdup(buf);
    // +2 = 66
    put_hash_set(hash_set, value);
  }
  // 626
  assert(size_of_hash_set(hash_set) == NUM_TEST_VALUES);
//
//  /* Automatically free all the values with the hash string_intern */
//  fflush(stdout);
//  register_hashset_free_functions(hash_set, NULL, free);
//
//  /* Check all values */
//
  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    sprintf(buf, "%i", i);
    int c = contains_in_hash_set(hash_set, buf);
    assert(c == 1);
  }

  /* Lookup on invalid values returns NULL */

  sprintf(buf, "%i", -1);
  assert(contains_in_hash_set(hash_set, buf) == 0);
  sprintf(buf, "%i", NUM_TEST_VALUES);
  assert(contains_in_hash_set(hash_set, buf) == 0);

/* Insert overwrites existing entries with the same key */

  sprintf(buf, "%i", 12345);
  value = strdup(buf);
  put_hash_set(hash_set, value);
  assert(size_of_hash_set(hash_set) == NUM_TEST_VALUES + 1);
  int c = contains_in_hash_set(hash_set, buf);
  assert(c == 1);
  free_hash_set(hash_set);
  fflush(stdout);
}

void test_hash_set_remove(void) {
  Hashset *hash_set;
  char buf[10];
  char *value;

  hash_set = generate_hash_set();

  assert(size_of_hash_set(hash_set) == NUM_TEST_VALUES);
  sprintf(buf, "%i", 5000);
  assert(contains_in_hash_set(hash_set, buf) == 1);

  /* Remove an entry */

  remove_hash_set(hash_set, buf);
  /* Check entry counter */

  assert(size_of_hash_set(hash_set) == 9999);

  /* Check that NULL is returned now */

  assert(contains_in_hash_set(hash_set, buf) == 0);

  /* Try removing a non-existent entry */

  sprintf(buf, "%i", -1);
  remove_hash_set(hash_set, buf);

  assert(size_of_hash_set(hash_set) == 9999);
  for (int i = 0; i < NUM_TEST_VALUES / 2; ++i) {
    sprintf(buf, "%i", i);
    int flag;

    remove_with_flag_hash_set(hash_set, buf, &flag);
    assert(flag == 1);
  }
  assert(size_of_hash_set(hash_set) == NUM_TEST_VALUES - (NUM_TEST_VALUES / 2) - 1);

  free_hash_set(hash_set);
}

//void test_hash_set_iterating(void)
//{
//  Hashset *hash_set;
//  HashtableIterator iterator;
//  int count;
//
//  hash_set = generate_hash_set();
//
//  /* Iterate over all values in the string_intern */
//
//  count = 0;
//
//  hash_set_iterate(hash_set, &iterator);
//
//  while (hash_set_iter_has_more(&iterator)) {
//    hash_set_iter_next(&iterator);
//
//    ++count;
//  }
//
//  assert(count == NUM_TEST_VALUES);
//
//  /* Test iter_next after iteration has completed. */
//
//  HashtablePair pair = hash_set_iter_next(&iterator);
//  assert(pair.value == hash_set_NULL);
//
//  free_hash_set(hash_set);
//
//  /* Test iterating over an empty string_intern */
//
//  hash_set = new_hash_set(int_hash, int_compare);
//
//  hash_set_iterate(hash_set, &iterator);
//
//  assert(hash_set_iter_has_more(&iterator) == 0);
//
//  free_hash_set(hash_set);
//}
//
///* Demonstrates the ability to iteratively remove objects from
// * a hash string_intern: ie. removing the current key being iterated over
// * does not break the iterator. */
//
void test_hashtable_entryset(void) {
  Hashset *hash_set;
  char buf[10];
  char *val;
  int count;
  unsigned int removed;
  int i;

  hash_set = generate_hash_set();

  /* Iterate over all values in the string_intern */

  count = 0;

  HashsetEntrySet *es = hashset_entry_set(hash_set);
  assert(size_of_hash_set(hash_set) == NUM_TEST_VALUES);
  assert(es->size == NUM_TEST_VALUES);
  KEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    KEntry *e = entries[j];
    HashsetKey key = set_entry_key(e);
    assert(contains_in_hash_set(hash_set, key) == 1);
  }

  /* Check counts */
  assert(size_of_hash_set(hash_set) == NUM_TEST_VALUES);

  free_hashset_entry_set(es);
  free_hash_set(hash_set);
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

void test_hash_set_out_of_memory(void) {
  Hashset *hash_set;
  int values[66];
  unsigned int i;

  hash_set = new_hash_set(int_hash, int_compare);

  /* Test failure when increasing string_intern size.
   * The initial string_intern size is 193 entries.  The string_intern increases in
   * size when 1/3 full, so the 66th entry should cause the insert
   * to fail.
   */

  for (i = 0; i < 65; ++i) {
    values[i] = (int) i;

    assert(put_hash_set(hash_set,
                        &values[i]) != 0);
    assert(size_of_hash_set(hash_set) == i + 1);
  }

  assert(size_of_hash_set(hash_set) == 65);

  /* Test the 66th insert */

  free_hash_set(hash_set);
}

void test_null_key() {
  Hashset *hashset = new_hash_set(string_hash, string_compare);
  put_hash_set(hashset, NULL);
  assert(size_of_hash_set(hashset) == 1);
  int c = contains_in_hash_set(hashset, NULL);
  assert(c == 1);
  remove_hash_set(hashset, NULL);
  assert(size_of_hash_set(hashset) == 0);
  assert(contains_in_hash_set(hashset, NULL) == 0);
  free_hash_set(hashset);
}

void test_null_key_tree() {
  char buf[10];
  char *value;
  Hashset *hashset = new_hash_set(dumb_hash, string_compare);
  register_hashset_free_functions(hashset, free);
  for (int i = 0; i < 8; ++i) {
    sprintf(buf, "%i", i);
    value = strdup(buf);
    put_hash_set(hashset, value);
  }
  sprintf(buf, "%i", 9);
  put_hash_set(hashset, NULL);
  assert(size_of_hash_set(hashset) == 9);
  int c = contains_in_hash_set(hashset, NULL);
  assert(c == 1);

  HashsetEntrySet *es = hashset_entry_set(hashset);
  assert(es->size == 9);
  int hasnull = 0;
  KEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    KEntry *e = entries[j];
    HashsetKey key = set_entry_key(e);
    if (key == NULL) {
      hasnull = 1;
    }
  }
  assert(hasnull == 1);
  free_hashset_entry_set(es);
  int r = remove_hash_set(hashset, NULL);
  assert(r == 1);
  assert(size_of_hash_set(hashset) == 8);
  assert(contains_in_hash_set(hashset, NULL) == 0);

  free_hash_set(hashset);
}

void test_hashset_union() {
  Hashset *set1 = new_hash_set(string_hash, string_compare);
  Hashset *set2 = new_hash_set(string_hash, string_compare);
  char buf[10];
  for (int i = 0; i < 10; i += 2) {
    sprintf(buf, "%i", i);
    char *k = strdup(buf);
    put_hash_set(set1, k);
  }
  assert(size_of_hash_set(set1) == 5);

  for (int i = 1; i < 10; i += 2) {
    sprintf(buf, "%i", i);
    char *k = strdup(buf);
    put_hash_set(set2, k);
  }
  assert(size_of_hash_set(set2) == 5);

  Hashset *set3 = union_hashset(set1, set2);
  assert(size_of_hash_set(set3) == 10);
  for (int i = 0; i < 10; ++i) {
    sprintf(buf, "%i", i);
    assert(contains_in_hash_set(set3, buf) == 1);
  }
  register_hashset_free_functions(set3, free);
  free_hash_set(set1);
  free_hash_set(set2);
  free_hash_set(set3);
}

void test_intersection_set() {
  Hashset *set1 = new_hash_set(string_hash, string_compare);
  Hashset *set2 = new_hash_set(string_hash, string_compare);
  char buf[10];
  for (int i = 0; i < 10; i++) {
    sprintf(buf, "%i", i);
    char *k = strdup(buf);
    put_hash_set(set1, k);
  }
  assert(size_of_hash_set(set1) == 10);

  for (int i = 5; i < 15; i++) {
    sprintf(buf, "%i", i);
    char *k = strdup(buf);
    put_hash_set(set2, k);
  }
  assert(size_of_hash_set(set2) == 10);

  Hashset *set3 = intersect_hashset(set1, set2);


//  HashsetEntrySet *es = hashset_entry_set(set3);
//  KEntry **entries = es->entry_set;
//  for (int j = 0; j < es->size; ++j) {
//    KEntry *e = entries[j];
//    HashsetKey key = set_entry_key(e);
//    printf("set3: %s\n", (char *) key);
//    fflush(stdout);
//  }
//  free_hashset_entry_set(es);
  assert(size_of_hash_set(set3) == 5);
  for (int i = 5; i < 10; ++i) {
    sprintf(buf, "%i", i);
    assert(contains_in_hash_set(set3, buf) == 1);
  }
  register_hashset_free_functions(set1, free);
  register_hashset_free_functions(set2, free);
  free_hash_set(set1);
  free_hash_set(set2);
  free_hash_set(set3);
}

void test_difference_set() {
  Hashset *set1 = new_hash_set(string_hash, string_compare);
  Hashset *set2 = new_hash_set(string_hash, string_compare);
  char buf[10];
  for (int i = 0; i < 10; i++) {
    sprintf(buf, "%i", i);
    char *k = strdup(buf);
    put_hash_set(set1, k);
  }
  assert(size_of_hash_set(set1) == 10);

  for (int i = 5; i < 10; i++) {
    sprintf(buf, "%i", i);
    char *k = strdup(buf);
    put_hash_set(set2, k);
  }
  assert(size_of_hash_set(set2) == 5);

  Hashset *set3 = difference_hashset(set1, set2);


//  HashsetEntrySet *es = hashset_entry_set(set3);
//  KEntry **entries = es->entry_set;
//  for (int j = 0; j < es->size; ++j) {
//    KEntry *e = entries[j];
//    HashsetKey key = set_entry_key(e);
//    printf("set3: %s\n", (char *) key);
//    fflush(stdout);
//  }
//  free_hashset_entry_set(es);
  assert(size_of_hash_set(set3) == 5);
  for (int i = 0; i < 5; ++i) {
    sprintf(buf, "%i", i);
    assert(contains_in_hash_set(set3, buf) == 1);
  }
  register_hashset_free_functions(set1, free);
  register_hashset_free_functions(set2, free);
  free_hash_set(set1);
  free_hash_set(set2);
  free_hash_set(set3);
}

static UnitTestFunction tests[] = {
    test_new_free_hash_set,
    test_put_contains_in_hash_set,
    test_hash_set_remove,
    test_set_load_factor_hash_set,
    test_hashtable_entryset,
    test_hash_set_out_of_memory,
    test_null_key,
    test_null_key_tree,
    test_hashset_union,
    test_intersection_set,
    test_difference_set,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}

