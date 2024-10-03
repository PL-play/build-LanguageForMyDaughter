//
// Created by ran on 2024/1/4.
//
#include <stdlib.h>
#include <assert.h>

#include "common/framework.h"

#include "heap/binary_heap.h"

#define NUM_TEST_VALUES 10000

int int_compare(void *vlocation1, void *vlocation2) {
  int *location1;
  int *location2;

  location1 = (int *) vlocation1;
  location2 = (int *) vlocation2;

  //printf("data1: %d, data2: %d\n", *location1, *location2);

  if (*location1 < *location2) {
    //printf("   <\n");
    return -1;
  } else if (*location1 > *location2) {
    //printf("   >\n");
    return 1;
  } else {
    // printf("   ==\n");
    return 0;
  }
}

int rint_compare(void *vlocation1, void *vlocation2) {
  int *location1;
  int *location2;

  location1 = (int *) vlocation1;
  location2 = (int *) vlocation2;

  //printf("data1: %d, data2: %d\n", *location1, *location2);

  if (*location1 < *location2) {
    //printf("   <\n");
    return 1;
  } else if (*location1 > *location2) {
    //printf("   >\n");
    return -1;
  } else {
    // printf("   ==\n");
    return 0;
  }
}

int test_array[NUM_TEST_VALUES];

void test_new_free_binary_heap(void) {
  BinaryHeap *heap;
  int i;

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    heap = new_binary_heap(int_compare);
    free_binary_heap(heap, NULL);
  }

}

void test_add_to_binary_heap(void) {
  BinaryHeap *heap;
  int i;

  heap = new_binary_heap(int_compare);

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = i;
    assert(add_to_binary_heap(heap, &test_array[i]) == 0);
  }

  assert(size_of_binary_heap(heap) == NUM_TEST_VALUES);

  free_binary_heap(heap, NULL);
}

void test_min_heap(void) {
  BinaryHeap *heap;
  int *val;
  int i;

  heap = new_binary_heap(rint_compare);

  /* Push a load of values onto the heap */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = i;
    assert(add_to_binary_heap(heap, &test_array[i]) == 0);
  }

  /* Pop values off the heap and check they are in order */

  i = -1;
  while (size_of_binary_heap(heap) > 0) {
    val = (int *) pop_from_binary_heap(heap);

    assert(*val == i + 1);
    i = *val;
  }

  /* Test popping from an empty heap */

  assert(size_of_binary_heap(heap) == 0);
  assert(pop_from_binary_heap(heap) == NULL);

  free_binary_heap(heap, NULL);
}

void test_max_heap(void) {
  BinaryHeap *heap;
  int *val;
  int i;

  heap = new_binary_heap(int_compare);

  /* Push a load of values onto the heap */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = i;
    assert(add_to_binary_heap(heap, &test_array[i]) == 0);
  }

  /* Pop values off the heap and check they are in order */

  i = NUM_TEST_VALUES;
  while (size_of_binary_heap(heap) > 0) {
    val = (int *) pop_from_binary_heap(heap);

    assert(*val == i - 1);
    i = *val;
  }

  free_binary_heap(heap, NULL);
}

/* Test out of memory scenario when adding items */

void test_out_of_memory(void) {
  BinaryHeap *heap;
  int *value;
  int values[] = {
      15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  };
  int i;

  /* Allocate a heap and fill to the default limit */

  heap = new_binary_heap(rint_compare);

  for (i = 0; i < 16; ++i) {
    assert(add_to_binary_heap(heap, &values[i]) == 0);
  }

  assert(size_of_binary_heap(heap) == 16);
}

static UnitTestFunction tests[] = {
    test_new_free_binary_heap,
    test_add_to_binary_heap,
    test_min_heap,
    test_max_heap,
    test_out_of_memory,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}