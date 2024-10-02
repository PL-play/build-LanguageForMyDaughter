//
// Created by ran on 2024/1/17.
//
#include "queue/dqueue_ll.h"
#include "list/linked_list.h"
#include <stdlib.h>


struct Dqueue {
  LinkedList *ll;
};

Dqueue *new_dqueue() {
  Dqueue *queue = malloc(sizeof(Dqueue));
  if (queue == NULL) return NULL;
  queue->ll = new_linked_list();
  return queue;
}

void free_dqueue(Dqueue *queue) {
  if (queue != NULL) {
    free_linked_list(queue->ll, NULL);
    free(queue);
  }
}

int dqueue_push_head(Dqueue *queue, DqueueValue value) {
  return prepend_list(queue->ll, value);
}

DqueueValue dqueue_pop_head(Dqueue *queue) {
  DqueueValue value = peek_list_first(queue->ll);
  remove_list_first(queue->ll);
  return value;
}

DqueueValue dqueue_peek_head(Dqueue *queue) {
  return peek_list_first(queue->ll);
}

int dqueue_push_tail(Dqueue *queue, DqueueValue value) {
  return append_list(queue->ll, value);
}

DqueueValue dqueue_pop_tail(Dqueue *queue) {
  DqueueValue value = peek_list_last(queue->ll);
  remove_list_last(queue->ll);
  return value;
}

DqueueValue dqueue_peek_tail(Dqueue *queue) {
  return peek_list_last(queue->ll);
}

int dqueue_is_empty(Dqueue *queue) {
  return list_size(queue->ll) == 0;
}