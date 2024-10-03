//
// Created by ran on 2024/1/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/framework.h"

#include "queue/dqueue_ll.h"

int variable1, variable2, variable3, variable4;

Dqueue *generate_queue(void)
{
  Dqueue *queue;
  int i;

  queue = new_dqueue();

  /* Add some values */

  for (i=0; i<1000; ++i) {
    dqueue_push_head(queue, &variable1);
    dqueue_push_head(queue, &variable2);
    dqueue_push_head(queue, &variable3);
    dqueue_push_head(queue, &variable4);
  }

  return queue;
}

/* Test cases for the queue */

void test_new_free_queue(void)
{
  int i;
  Dqueue *queue;

  /* Create and destroy a queue */

  queue = new_dqueue();

  free_dqueue(queue);

  /* Add lots of values and then destroy */

  queue = new_dqueue();

  for (i=0; i<1000; ++i) {
    dqueue_push_head(queue, &variable1);
  }

  free_dqueue(queue);

}

void test_queue_push_head(void)
{
  Dqueue *queue;
  int i;

  queue = new_dqueue();

  /* Add some values */

  for (i=0; i<1000; ++i) {
    dqueue_push_head(queue, &variable1);
    dqueue_push_head(queue, &variable2);
    dqueue_push_head(queue, &variable3);
    dqueue_push_head(queue, &variable4);
  }

  assert(!dqueue_is_empty(queue));

  /* Check values come out of the tail properly */

  assert(dqueue_pop_tail(queue) == &variable1);
  assert(dqueue_pop_tail(queue) == &variable2);
  assert(dqueue_pop_tail(queue) == &variable3);
  assert(dqueue_pop_tail(queue) == &variable4);

  /* Check values come back out of the head properly */

  assert(dqueue_pop_head(queue) == &variable4);
  assert(dqueue_pop_head(queue) == &variable3);
  assert(dqueue_pop_head(queue) == &variable2);
  assert(dqueue_pop_head(queue) == &variable1);

  free_dqueue(queue);

}

void test_queue_pop_head(void)
{
  Dqueue *queue;

  /* Check popping off an empty queue */

  queue = new_dqueue();

  assert(dqueue_pop_head(queue) == NULL);

  free_dqueue(queue);

  /* Pop off all the values from the queue */

  queue = generate_queue();

  while (!dqueue_is_empty(queue)) {
    assert(dqueue_pop_head(queue) == &variable4);
    assert(dqueue_pop_head(queue) == &variable3);
    assert(dqueue_pop_head(queue) == &variable2);
    assert(dqueue_pop_head(queue) == &variable1);
  }

  assert(dqueue_pop_head(queue) == NULL);

  free_dqueue(queue);
}

void test_queue_peek_head(void)
{
  Dqueue *queue;

  /* Check peeking into an empty queue */

  queue = new_dqueue();

  assert(dqueue_peek_head(queue) == NULL);

  free_dqueue(queue);

  /* Pop off all the values from the queue, making sure that peek
   * has the correct value beforehand */

  queue = generate_queue();

  while (!dqueue_is_empty(queue)) {
    assert(dqueue_peek_head(queue) == &variable4);
    assert(dqueue_pop_head(queue) == &variable4);
    assert(dqueue_peek_head(queue) == &variable3);
    assert(dqueue_pop_head(queue) == &variable3);
    assert(dqueue_peek_head(queue) == &variable2);
    assert(dqueue_pop_head(queue) == &variable2);
    assert(dqueue_peek_head(queue) == &variable1);
    assert(dqueue_pop_head(queue) == &variable1);
  }

  assert(dqueue_peek_head(queue) == NULL);

  free_dqueue(queue);
}

void test_queue_push_tail(void)
{
  Dqueue *queue;
  int i;

  queue = new_dqueue();

  /* Add some values */

  for (i=0; i<1000; ++i) {
    dqueue_push_tail(queue, &variable1);
    dqueue_push_tail(queue, &variable2);
    dqueue_push_tail(queue, &variable3);
    dqueue_push_tail(queue, &variable4);
  }

  assert(!dqueue_is_empty(queue));

  /* Check values come out of the head properly */

  assert(dqueue_pop_head(queue) == &variable1);
  assert(dqueue_pop_head(queue) == &variable2);
  assert(dqueue_pop_head(queue) == &variable3);
  assert(dqueue_pop_head(queue) == &variable4);

  /* Check values come back out of the tail properly */

  assert(dqueue_pop_tail(queue) == &variable4);
  assert(dqueue_pop_tail(queue) == &variable3);
  assert(dqueue_pop_tail(queue) == &variable2);
  assert(dqueue_pop_tail(queue) == &variable1);

  free_dqueue(queue);

}

void test_queue_pop_tail(void)
{
  Dqueue *queue;

  /* Check popping off an empty queue */

  queue = new_dqueue();

  assert(dqueue_pop_tail(queue) == NULL);

  free_dqueue(queue);

  /* Pop off all the values from the queue */

  queue = generate_queue();

  while (!dqueue_is_empty(queue)) {
    assert(dqueue_pop_tail(queue) == &variable1);
    assert(dqueue_pop_tail(queue) == &variable2);
    assert(dqueue_pop_tail(queue) == &variable3);
    assert(dqueue_pop_tail(queue) == &variable4);
  }

  assert(dqueue_pop_tail(queue) == NULL);

  free_dqueue(queue);
}

void test_queue_peek_tail(void)
{
  Dqueue *queue;

  /* Check peeking into an empty queue */

  queue = new_dqueue();

  assert(dqueue_peek_tail(queue) == NULL);

  free_dqueue(queue);

  /* Pop off all the values from the queue, making sure that peek
   * has the correct value beforehand */

  queue = generate_queue();

  while (!dqueue_is_empty(queue)) {
    assert(dqueue_peek_tail(queue) == &variable1);
    assert(dqueue_pop_tail(queue) == &variable1);
    assert(dqueue_peek_tail(queue) == &variable2);
    assert(dqueue_pop_tail(queue) == &variable2);
    assert(dqueue_peek_tail(queue) == &variable3);
    assert(dqueue_pop_tail(queue) == &variable3);
    assert(dqueue_peek_tail(queue) == &variable4);
    assert(dqueue_pop_tail(queue) == &variable4);
  }

  assert(dqueue_peek_tail(queue) == NULL);

  free_dqueue(queue);
}

void test_queue_is_empty(void)
{
  Dqueue *queue;

  queue = new_dqueue();

  assert(dqueue_is_empty(queue));

  dqueue_push_head(queue, &variable1);

  assert(!dqueue_is_empty(queue));

  dqueue_pop_head(queue);

  assert(dqueue_is_empty(queue));

  dqueue_push_tail(queue, &variable1);

  assert(!dqueue_is_empty(queue));

  dqueue_pop_tail(queue);

  assert(dqueue_is_empty(queue));

  free_dqueue(queue);
}

static UnitTestFunction tests[] = {
    test_new_free_queue,
    test_queue_push_head,
    test_queue_pop_head,
    test_queue_peek_head,
    test_queue_push_tail,
    test_queue_pop_tail,
    test_queue_peek_tail,
    test_queue_is_empty,
    NULL
};

int main(int argc, char *argv[])
{
  run_tests(tests);

  return 0;
}

