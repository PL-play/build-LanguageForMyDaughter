
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/framework.h"

#include "list/linked_list_m.h"

DECLARE_LINKED_LIST(int ,Int_)

DEFINE_LINKED_LIST(int,-1,Int_)

void display_list(Int_LinkedListNode *head) {
  printf("\nContents of your linked list: \n");
  Int_LinkedListNode *temp;
  temp = head;
  if (head != NULL) {
    while (Int_next_node_linked_list(temp) != head) {
      printf("%d\n",  (Int_data_of_node_linked_list(temp)));
      temp = Int_next_node_linked_list(temp);
    }
    if (Int_next_node_linked_list(temp) == head) {
      printf("%d\n",  (Int_data_of_node_linked_list(temp)));
    }
  } else {
    printf("The list is empty");
  }
  printf("\n");
}
int int_equal(int a, int b) {

  if( a == b){
    return 0;
  } else if( a >b){
    return 1;
  } else{
    return -1;
  }

}
int int_compare(int a, int b) {
  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

int variable1 = 50, variable2=43, variable3=23, variable4=37;

/* Returns a list containing four entries */

Int_LinkedList *generate_list(void) {
  Int_LinkedList *list = Int_new_linked_list();

  assert(Int_append_list(list, variable1) == 1);
  assert(Int_append_list(list, variable2) == 1);
  assert(Int_append_list(list, variable3) == 1);
  assert(Int_append_list(list, variable4) == 1);

  return list;
}

void test_append_linked_list(void) {
  Int_LinkedList *list = Int_new_linked_list();

  assert(Int_append_list(list, variable1));
  assert(Int_append_list(list, variable2));
  assert(Int_append_list(list, variable3));
  assert(Int_append_list(list, variable4));

  assert(Int_list_size(list) == 4);

  assert(Int_get_list_data(list, 0) == variable1);
  assert(Int_get_list_data(list, 1) == variable2);
  assert(Int_get_list_data(list, 2) == variable3);
  assert(Int_get_list_data(list, 3) == variable4);

  Int_free_linked_list(list, NULL);
}

void test_prepend_linked_list(void) {
  Int_LinkedList *list = Int_new_linked_list();
  printf("add v1:%d\n", variable1);
  fflush(stdout);
  assert(Int_prepend_list(list, variable1));
  printf("add v2:%d\n", variable2);
  fflush(stdout);
  assert(Int_prepend_list(list, variable2));
  printf("add v3:%d\n", variable3);
  fflush(stdout);
  assert(Int_prepend_list(list, variable3));
  printf("add v4:%d\n", variable4);
  fflush(stdout);
  assert(Int_prepend_list(list, variable4));
  display_list(Int_head_of_list(list));
  fflush(stdout);
  assert(Int_get_list_data(list, 0) == variable4);
  assert(Int_get_list_data(list, 1) == variable3);
  assert(Int_get_list_data(list, 2) == variable2);
  assert(Int_get_list_data(list, 3) == variable1);

  /* Test out of memory scenario */

  Int_free_linked_list(list, NULL);
}

void test_free_linked_list(void) {
  Int_LinkedList *list = generate_list();

  /* Create a list and free it */

  Int_free_linked_list(list, NULL);

  /* Check the empty list frees correctly */
}

void test_node_of_linked_list(void) {
  Int_LinkedList *list = generate_list();

  /* Check all values in the list */

  assert(Int_data_of_node_linked_list(Int_node_of_list(list, 0)) == variable1);
  assert(Int_data_of_node_linked_list(Int_node_of_list(list, 1)) == variable2);
  assert(Int_data_of_node_linked_list(Int_node_of_list(list, 2)) == variable3);
  assert(Int_data_of_node_linked_list(Int_node_of_list(list, 3)) == variable4);

  /* Check out of range values */

  assert(Int_node_of_list(list, 4) == NULL);
  assert(Int_node_of_list(list, 400) == NULL);

  Int_free_linked_list(list, NULL);
}

void test_size_of_linked_list(void) {
  Int_LinkedList *list = generate_list();

  /* Generate a list and check that it is four entries long */

  assert(Int_list_size(list) == 4);

  /* Add an entry and check that it still works properly */

  assert(Int_prepend_list(list, variable1));

  assert(Int_list_size(list) == 5);

  Int_free_linked_list(list, NULL);

}

void test_remove_node_linked_list(void) {
  Int_LinkedListNode *empty_list = NULL;
  Int_LinkedListNode *entry;

  Int_LinkedList *list = generate_list();

  /* Remove the third entry */

  entry = Int_node_of_list(list, 2);
  Int_ListValue data = Int_data_of_node_linked_list(entry);
  assert(Int_remove_list(list, 2) == data);
  assert(Int_list_size(list) == 3);
  /* Remove the first entry */

  entry = Int_node_of_list(list, 0);
  data = Int_data_of_node_linked_list(entry);
  assert(Int_remove_list(list, 0) == data);
  assert(Int_list_size(list) == 2);

  /* Try some invalid removes */

  /* Removing NULL from an empty list */

  Int_free_linked_list(list, NULL);

  /* Test removing the last entry */

  list = generate_list();
  entry = Int_node_of_list(list, 3);
  data = Int_data_of_node_linked_list(entry);
  assert(Int_remove_list(list, 3) == data);
  Int_free_linked_list(list, NULL);
}

void test_remove_data_linked_list(void) {
  int entries[] = {89, 4, 23, 42, 4, 16, 15, 4, 8, 99, 50, 30, 4};
  unsigned int num_entries = sizeof(entries) / sizeof(int);
  int val;
  Int_LinkedList *list = Int_new_linked_list();
  unsigned int i;

  /* Generate a list containing all the entries in the array */

  for (i = 0; i < num_entries; ++i) {
    assert(Int_prepend_list(list, entries[i]));
  }

  /* Test removing invalid data */

  val = 0;
  assert(Int_remove_list_equals_to(list, val, int_equal) == 0);
  val = 56;
  assert(Int_remove_list_equals_to(list, val, int_equal) == 0);

  /* Remove the number 8 from the list */

  val = 8;
  assert(Int_remove_list_equals_to(list, val, int_equal) == 1);
  assert(Int_list_size(list) == num_entries - 1);

  /* Remove the number 4 from the list (occurs multiple times) */

  val = 4;
  display_list(Int_head_of_list(list));
  fflush(stdout);
  int c = Int_remove_list_equals_to(list, val, int_equal);
  assert(c == 4);
  assert(Int_list_size(list) == num_entries - 5);

  /* Remove the number 89 from the list (first entry) */

  val = 89;
  assert(Int_remove_list_equals_to(list, val, int_equal) == 1);
  assert(Int_list_size(list) == num_entries - 6);

  Int_free_linked_list(list, NULL);
}

void test_linked_list_to_array(void) {
  Int_LinkedList *list;
  int *array;

  list = generate_list();

  array = Int_value_array(list);

  assert(array[0] == variable1);
  assert(array[1] == variable2);
  assert(array[2] == variable3);
  assert(array[3] == variable4);

  free(array);

  /* Test out of memory scenario */

  Int_free_linked_list(list, NULL);
}

void test_remove_head_tail_linked_list(void) {
  Int_LinkedList *list = generate_list();
  assert(Int_list_size(list) == 4);
  assert(Int_data_of_node_linked_list(Int_head_of_list(list)) == variable1);
  assert(Int_remove_list_first(list) == 1);

  assert(Int_list_size(list) == 3);
  assert(Int_data_of_node_linked_list(Int_head_of_list(list)) == variable2);

  assert(Int_remove_list_last(list) == 1);
  assert(Int_list_size(list) == 2);
  assert(Int_data_of_node_linked_list(Int_prev_node_linked_list(Int_head_of_list(list))) == variable3);

  assert(Int_remove_list_last(list) == 1);
  assert(Int_list_size(list) == 1);
  assert(Int_data_of_node_linked_list(Int_prev_node_linked_list(Int_head_of_list(list))) == variable2);

  assert(Int_remove_list_first(list) == 1);
  assert(Int_list_size(list) == 0);
  assert(Int_data_of_node_linked_list(Int_prev_node_linked_list(Int_head_of_list(list))) == -1);

  Int_free_linked_list(list, NULL);

}

/* Test that the iterator functions can survive removal of the current
 * value using the normal remove functions. */


static UnitTestFunction tests[] = {
    test_append_linked_list,
    test_prepend_linked_list,
    test_free_linked_list,
    test_node_of_linked_list,
    test_node_of_linked_list,
    test_size_of_linked_list,
    test_remove_node_linked_list,
    test_remove_data_linked_list,
    test_linked_list_to_array,
    test_remove_head_tail_linked_list,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}

