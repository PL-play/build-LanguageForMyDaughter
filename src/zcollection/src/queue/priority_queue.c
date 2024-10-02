//
// Created by ran on 2024/2/5.
//
#include "queue/priority_queue.h"
#include "heap/binary_heap.h"
#include <stdlib.h>


struct PriorityQueue {
  BinaryHeap *heap;
};

PriorityQueue *create_pq(cmp_func func) {
  PriorityQueue *pq = malloc(sizeof(PriorityQueue));
  if (!pq) return NULL;
  BinaryHeap *heap = new_binary_heap(func);
  if (!heap) {
    free(pq);
    return NULL;
  }
  pq->heap = heap;
  return pq;
}

void free_pq(PriorityQueue *priority_queue, void (*free_func)(void *)) {
  if (priority_queue) {
    free_binary_heap(priority_queue->heap, free_func);
    free(priority_queue);
  }
}

int size_of_pq(PriorityQueue *priority_queue) {
  return size_of_binary_heap(priority_queue->heap);
}

int is_pq_empty(PriorityQueue *priority_queue) {
  return size_of_pq(priority_queue) == 0;
}

PriorityQueueData pq_dequeue(PriorityQueue *priority_queue) {
  return pop_from_binary_heap(priority_queue->heap);
}

int pq_enqueue(PriorityQueue *priority_queue, PriorityQueueData data) {
  int ret = add_to_binary_heap(priority_queue->heap, data);
  return ret == BINARY_HEAP_SUCCESS ? 1 : 0;
}

PriorityQueueData pq_peek(PriorityQueue *priority_queue) {
  return peak_binary_heap(priority_queue->heap);
}
