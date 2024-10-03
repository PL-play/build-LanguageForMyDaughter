#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common/framework.h"

#include "tree/trie.h"

#define NUM_TEST_VALUES 10000

int test_array[NUM_TEST_VALUES];
char test_strings[NUM_TEST_VALUES][10];
unsigned char bin_key[] = {'a', 'b', 'c', 0, 1, 2, 0xff};
unsigned char bin_key2[] = {'a', 'b', 'c', 0, 1, 2, 0xff, 0};
unsigned char bin_key3[] = {'a', 'b', 'c'};
unsigned char bin_key4[] = {'z', 0, 'z', 'z'};

Trie *generate_trie(void) {
  Trie *trie;
  int i;
  unsigned int entries;

  /* Create a trie and fill it with a large number of values */

  trie = new_trie();
  entries = 0;

  for (i = 0; i < NUM_TEST_VALUES; ++i) {

    /* Create a string containing a text version of i, and use
     * it as a key for the value */

    test_array[i] = i;
    sprintf(test_strings[i], "%i", i);

    assert(insert_trie(trie, test_strings[i],
                       &test_array[i]) != 0);

    ++entries;

    assert(size_of_trie(trie) == entries);
  }

  return trie;
}

void test_new_free_trie(void) {
  Trie *trie;

  /* Allocate and free an empty trie */

  trie = new_trie();

  assert(trie != NULL);

  free_trie(trie);

  /* Add some values before freeing */

  trie = new_trie();

  assert(insert_trie(trie, "hello", "there") != 0);
  assert(insert_trie(trie, "helle", "there") != 0);
  assert(insert_trie(trie, "hell", "testing") != 0);
  assert(insert_trie(trie, "testing", "testing") != 0);
  assert(insert_trie(trie, "", "asfasf") != 0);

  free_trie(trie);

  /* Add a value, remove it and then free */

  trie = new_trie();

  assert(insert_trie(trie, "hello", "there") != 0);
  assert(remove_key_trie(trie, "hello") != 0);

  free_trie(trie);

}

void test_insert_trie(void) {
  Trie *trie;
  unsigned int entries;
  size_t allocated;

  trie = generate_trie();
  free_trie(trie);
  /* Test insert of NULL value has no effect */

  trie = generate_trie();
  entries = size_of_trie(trie);
  /* Test out of memory scenario */


  /* Test rollback */

  free_trie(trie);
}

void test_get_value_trie(void) {
  Trie *trie;
  char buf[10];
  int *val;
  int i;

  trie = generate_trie();

  /* Test lookup for non-existent values */

  assert(get_value_trie(trie, "000000000000000") == NULL);
  assert(get_value_trie(trie, "") == NULL);

  /* Look up all values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {

    sprintf(buf, "%i", i);

    val = (int *) get_value_trie(trie, buf);

    assert(*val == i);
  }

  free_trie(trie);
}

void test_remove_key_trie(void) {
  Trie *trie;
  char buf[10];
  int i;
  unsigned int entries;

  trie = generate_trie();

  /* Test remove on non-existent values. */

  assert(remove_key_trie(trie, "000000000000000") == 0);
  assert(remove_key_trie(trie, "") == 0);

  entries = size_of_trie(trie);

  assert(entries == NUM_TEST_VALUES);

  /* Remove all values */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {

    sprintf(buf, "%i", i);

    /* Remove value and check counter */

    assert(remove_key_trie(trie, buf) != 0);
    --entries;
    assert(size_of_trie(trie) == entries);
  }

  free_trie(trie);
}

void test_trie_replace(void) {
  Trie *trie;
  int *val;

  trie = generate_trie();

  /* Test replacing values */

  val = malloc(sizeof(int));
  *val = 999;
  assert(insert_trie(trie, "999", val) != 0);
  assert(size_of_trie(trie) == NUM_TEST_VALUES);

  assert(get_value_trie(trie, "999") == val);
  free(val);
  free_trie(trie);
}

void test_insert_trie_empty(void) {
  Trie *trie;
  char buf[10];

  trie = new_trie();

  /* Test insert on empty string */

  assert(insert_trie(trie, "", buf) != 0);
  assert(size_of_trie(trie) != 0);
  assert(get_value_trie(trie, "") == buf);
  assert(remove_key_trie(trie, "") != 0);

  assert(size_of_trie(trie) == 0);

  free_trie(trie);
}

#define LONG_STRING_LEN 4096
static void test_free_trie_long(void) {
  char *long_string;
  Trie *trie;

  /* Generate a long string */

  long_string = malloc(LONG_STRING_LEN);
  memset(long_string, 'A', LONG_STRING_LEN);
  long_string[LONG_STRING_LEN - 1] = '\0';

  /* Create a trie and add the string */

  trie = new_trie();
  insert_trie(trie, long_string, long_string);

  free_trie(trie);

  free(long_string);
}

/* Test the use of the trie when characters in the keys used are negative
 * (top bit set in the character; alternative, c >= 128). */

static void test_trie_negative_keys(void) {
  char my_key[] = {'a', 'b', 'c', -50, -20, '\0'};
  Trie *trie;
  void *value;

  trie = new_trie();

  assert(insert_trie(trie, my_key, "hello world") != 0);

  value = get_value_trie(trie, my_key);

  assert(!strcmp(value, "hello world"));

  assert(remove_key_trie(trie, my_key) != 0);
  assert(remove_key_trie(trie, my_key) == 0);
  assert(get_value_trie(trie, my_key) == NULL);

  free_trie(trie);
}

static UnitTestFunction tests[] = {
    test_new_free_trie,
    test_insert_trie,
    test_get_value_trie,
    test_remove_key_trie,
    test_trie_replace,
    test_insert_trie_empty,
	test_free_trie_long,
	test_trie_negative_keys,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}

