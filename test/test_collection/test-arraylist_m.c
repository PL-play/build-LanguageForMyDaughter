/* M_ArrayList test cases */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/framework.h"

#include "list/array_list_m.h"
#include <string.h>
DECLARE_ARRAY_LIST(void*,M_)
DEFINE_ARRAY_LIST(NULL,M_)

int variable1, variable2, variable3, variable4;

int int_equal(void *vlocation1, void *vlocation2) {
  int *location1;
  int *location2;

  location1 = (int *) vlocation1;
  location2 = (int *) vlocation2;
  if( *location1 == *location2){
    return 0;
  } else if( *location1 >*location2){
    return 1;
  } else{
    return -1;
  }

}
int int_compare(void *vlocation1, void *vlocation2) {
  int *location1;
  int *location2;

  location1 = (int *) vlocation1;
  location2 = (int *) vlocation2;

  if (*location1 < *location2) {
    return -1;
  } else if (*location1 > *location2) {
    return 1;
  } else {
    return 0;
  }
}

M_ArrayList *generate_arraylist(void) {
  M_ArrayList *arraylist;
  int i;

  arraylist = M_new_arraylist(0);

  for (i = 0; i < 4; ++i) {
    M_append_arraylist(arraylist, &variable1);
    M_append_arraylist(arraylist, &variable2);
    M_append_arraylist(arraylist, &variable3);
    M_append_arraylist(arraylist, &variable4);
  }

  return arraylist;
}

void test_new_free_arraylist(void) {
  M_ArrayList *arraylist;

  /* Use a default size when given zero */

  arraylist = M_new_arraylist(0);
  assert(arraylist != NULL);
  M_free_arraylist(arraylist);

  /* Normal allocated */

  arraylist = M_new_arraylist(10);
  assert(arraylist != NULL);
  M_free_arraylist(arraylist);

  /* Freeing a null arraylist works */

  M_free_arraylist(NULL);

}

void test_append_arraylist(void) {
  M_ArrayList *arraylist;
  int i;

  arraylist = M_new_arraylist(0);

  assert(arraylist->size == 0);

  /* Append some entries */

  assert(M_append_arraylist(arraylist, &variable1) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 1);

  assert(M_append_arraylist(arraylist, &variable2) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 2);

  assert(M_append_arraylist(arraylist, &variable3) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 3);

  assert(M_append_arraylist(arraylist, &variable4) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 4);

  assert(arraylist->data[0] == &variable1);
  assert(arraylist->data[1] == &variable2);
  assert(arraylist->data[2] == &variable3);
  assert(arraylist->data[3] == &variable4);

  /* Test appending many entries */

  for (i = 0; i < 10000; ++i) {
    assert(M_append_arraylist(arraylist, NULL) == M_ARRAYLIST_SUCCESS);
  }

  M_free_arraylist(arraylist);

}

void test_prepend_arraylist(void) {
  M_ArrayList *arraylist;
  int i;

  arraylist = M_new_arraylist(0);

  assert(arraylist->size == 0);

  /* Append some entries */

  assert(M_prepend_arraylist(arraylist, &variable1) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 1);

  assert(M_prepend_arraylist(arraylist, &variable2) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 2);

  assert(M_prepend_arraylist(arraylist, &variable3) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 3);

  assert(M_prepend_arraylist(arraylist, &variable4) == M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 4);

  assert(arraylist->data[0] == &variable4);
  assert(arraylist->data[1] == &variable3);
  assert(arraylist->data[2] == &variable2);
  assert(arraylist->data[3] == &variable1);

  /* Test prepending many entries */

  for (i = 0; i < 10000; ++i) {
    assert(M_prepend_arraylist(arraylist, NULL) == M_ARRAYLIST_SUCCESS);
  }

  M_free_arraylist(arraylist);

}

void test_insert_arraylist(void) {
  M_ArrayList *arraylist;
  int i;

  arraylist = generate_arraylist();

  /* Check for out of range insert */

  assert(arraylist->size == 16);
  assert(M_insert_arraylist(arraylist, 17, &variable1) != M_ARRAYLIST_SUCCESS);
  assert(arraylist->size == 16);

  /* Insert a new entry at index 5 */

  assert(arraylist->size == 16);
  assert(arraylist->data[4] == &variable1);
  assert(arraylist->data[5] == &variable2);
  assert(arraylist->data[6] == &variable3);

  assert(M_insert_arraylist(arraylist, 5, &variable4) == M_ARRAYLIST_SUCCESS);

  assert(arraylist->size == 17);
  assert(arraylist->data[4] == &variable1);
  assert(arraylist->data[5] == &variable4);
  assert(arraylist->data[6] == &variable2);
  assert(arraylist->data[7] == &variable3);

  /* Inserting at the start */

  assert(arraylist->data[0] == &variable1);
  assert(arraylist->data[1] == &variable2);
  assert(arraylist->data[2] == &variable3);

  assert(M_insert_arraylist(arraylist, 0, &variable4) == M_ARRAYLIST_SUCCESS);

  assert(arraylist->size == 18);
  assert(arraylist->data[0] == &variable4);
  assert(arraylist->data[1] == &variable1);
  assert(arraylist->data[2] == &variable2);
  assert(arraylist->data[3] == &variable3);

  /* Inserting at the end */

  assert(arraylist->data[15] == &variable2);
  assert(arraylist->data[16] == &variable3);
  assert(arraylist->data[17] == &variable4);

  assert(M_insert_arraylist(arraylist, 18, &variable1) == M_ARRAYLIST_SUCCESS);

  assert(arraylist->size == 19);
  assert(arraylist->data[15] == &variable2);
  assert(arraylist->data[16] == &variable3);
  assert(arraylist->data[17] == &variable4);
  assert(arraylist->data[18] == &variable1);

  /* Test inserting many entries */

  for (i = 0; i < 10000; ++i) {
    M_insert_arraylist(arraylist, 10, &variable1);
  }

  M_free_arraylist(arraylist);
}

void test_remove_arraylist(void) {
  M_ArrayList *arraylist;

  arraylist = generate_arraylist();

  assert(arraylist->size == 16);
  assert(arraylist->data[3] == &variable4);
  assert(arraylist->data[4] == &variable1);
  assert(arraylist->data[5] == &variable2);
  assert(arraylist->data[6] == &variable3);

  M_remove_arraylist(arraylist, 4);

  assert(arraylist->size == 15);
  assert(arraylist->data[3] == &variable4);
  assert(arraylist->data[4] == &variable2);
  assert(arraylist->data[5] == &variable3);
  assert(arraylist->data[6] == &variable4);

  /* Try some invalid removes */

  M_remove_arraylist(arraylist, 15);

  assert(arraylist->size == 15);

  M_free_arraylist(arraylist);
}

void test_index_of_arraylist(void) {
  int entries[] = {89, 4, 23, 42, 16, 15, 8, 99, 50, 30};
  int num_entries;
  M_ArrayList *arraylist;
  int i;
  int index;
  int val;

  /* Generate an arraylist containing the entries in the array */

  num_entries = sizeof(entries) / sizeof(int);
  arraylist = M_new_arraylist(0);

  for (i = 0; i < num_entries; ++i) {
    M_append_arraylist(arraylist, &entries[i]);
  }
  /* Check all values get found correctly */
  printf("\n");
  for (i = 0; i < num_entries; ++i) {

    val = entries[i];
    index = M_index_of_arraylist(arraylist, &val, int_equal);
    assert(index == i);
  }

  /* Check invalid values */

  val = 0;
  assert(M_index_of_arraylist(arraylist, &val, int_equal) < 0);
  val = 57;
  assert(M_index_of_arraylist(arraylist, &val, int_equal) < 0);

  M_free_arraylist(arraylist);
}

void test_clear_arraylist(void) {
  M_ArrayList *arraylist;

  arraylist = M_new_arraylist(0);

  /* Emptying an already-empty arraylist */

  M_clear_arraylist(arraylist);
  assert(arraylist->size == 0);

  /* Add some items and then empty it */

  M_append_arraylist(arraylist, &variable1);
  M_append_arraylist(arraylist, &variable2);
  M_append_arraylist(arraylist, &variable3);
  M_append_arraylist(arraylist, &variable4);

  M_clear_arraylist(arraylist);

  assert(arraylist->size == 0);

  M_free_arraylist(arraylist);
}



void test_set_arraylist(void) {
  M_ArrayList *arraylist;
  arraylist = generate_arraylist();

  assert(M_get_data_arraylist(arraylist, 0) == &variable1);
  assert(M_get_data_arraylist(arraylist, 4) == &variable1);
  assert(M_get_data_arraylist(arraylist, 8) == &variable1);
  assert(M_get_data_arraylist(arraylist, 12) == &variable1);
  M_free_arraylist(arraylist);
}

static UnitTestFunction tests[] = {
    test_new_free_arraylist,
    test_append_arraylist,
    test_prepend_arraylist,
    test_insert_arraylist,
    test_remove_arraylist,
    test_index_of_arraylist,
    test_clear_arraylist,
    test_set_arraylist,
    NULL
};

int main(int argc, char *argv[]) {

  run_tests(tests);

  return 0;
}

