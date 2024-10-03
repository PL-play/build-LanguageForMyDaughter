//
// Created by ran on 2024/1/4.
//
#include <stdlib.h>
#include <assert.h>

#include "common/framework.h"

#include "queue/priority_queue.h"

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

void test_new_pq(void) {
   PriorityQueue *pq;
  int i;

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    pq = create_pq(int_compare);
    free_pq(pq, NULL);
  }

}

void test_add_to_pq(void) {
  PriorityQueue *pq;
  int i;

  pq = create_pq(int_compare);

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = i;
    assert(pq_enqueue(pq, &test_array[i]) == 1);
  }

  assert(size_of_pq(pq) == NUM_TEST_VALUES);

  free_pq(pq, NULL);
}

void test_pq(void) {
  PriorityQueue *pq;
  int *val;
  int i;

  pq = create_pq(rint_compare);

  /* Push a load of values onto the heap */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = i;
    assert(pq_enqueue(pq, &test_array[i]) == 1);
  }

  /* Pop values off the heap and check they are in order */

  i = -1;
  while (size_of_pq(pq) > 0) {
    val = (int *) pq_dequeue(pq);

    assert(*val == i + 1);
    i = *val;
  }

  /* Test popping from an empty heap */

  assert(size_of_pq(pq) == 0);
  assert(pq_dequeue(pq) == NULL);

  free_pq(pq, NULL);
}

void test_max_heap(void) {
  PriorityQueue *pq;
  int *val;
  int i;

  pq = create_pq(int_compare);

  /* Push a load of values onto the heap */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = i;
    assert(pq_enqueue(pq, &test_array[i]) == 1);
  }

  /* Pop values off the heap and check they are in order */

  i = NUM_TEST_VALUES;
  while (size_of_pq(pq) > 0) {
    val = (int *) pq_dequeue(pq);

    assert(*val == i - 1);
    i = *val;
  }

  free_pq(pq, NULL);
}

/* Test out of memory scenario when adding items */

void test_out_of_memory(void) {
  PriorityQueue *pq;
  int *value;
  int values[] = {
      15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  };
  int i;

  /* Allocate a heap and fill to the default limit */

  pq = create_pq(rint_compare);

  for (i = 0; i < 16; ++i) {
    assert(pq_enqueue(pq, &values[i]) == 1);
  }

  assert(size_of_pq(pq) == 16);

  /* Check that we can read the values back out again and they
   * are in the right order. */

  for (i = 0; i < 16; ++i) {
    value = pq_dequeue(pq);
    assert(*value == i);
  }

  assert(size_of_pq(pq) == 0);

  free_pq(pq, NULL);
}

static UnitTestFunction tests[] = {
    test_new_pq,
    test_add_to_pq,
    test_pq,
    test_max_heap,
    test_out_of_memory,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}