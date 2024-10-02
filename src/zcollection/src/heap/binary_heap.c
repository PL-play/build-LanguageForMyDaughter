//
// Created by ran on 2024/1/4.
//
#include <stdlib.h>
#include "heap/binary_heap.h"


#define INIT_VALUE_CAPACITY 8

struct BinaryHeap {
  BinaryHeapData *values;
  unsigned int size;
  unsigned int capacity;
  BinaryHeapComparator comparator;
};

static void sift_down(BinaryHeap *heap, unsigned int index);
static void sift_up(BinaryHeap *heap, unsigned int index);
static unsigned int parent_index(unsigned int index);
static unsigned int left_child_index(unsigned int parent);
static unsigned int right_child_index(unsigned int parent);
static void grow_values(BinaryHeap *heap);
static void swap_values(BinaryHeap *heap, unsigned int a, unsigned int b);

/**
 * Create a new binary heap use the given initial data. if NULL is given, create a empty one.
 *
 * @param init_data     the initial data
 * @param data_size     size of initial data
 * @param compare_func  comparator of value
 * @return
 */
BinaryHeap *new_binary_heap(BinaryHeapComparator compare_func) {
  BinaryHeap *heap = malloc(sizeof(BinaryHeap));
  if (heap == NULL) return NULL;
  heap->values = malloc(sizeof(BinaryHeapData) * INIT_VALUE_CAPACITY);
  if (heap->values == NULL) {
    free(heap);
    return NULL;
  }
  heap->size = 0;
  heap->capacity = INIT_VALUE_CAPACITY;
  heap->comparator = compare_func;
  return heap;
}

/**
 * Free a binary heap.
 * @param heap
 */
void free_binary_heap(BinaryHeap *heap, void(*free_func)(void *)) {
  if (free_func) {
    for (int i = 0; i < heap->size; ++i) {
      free_func(heap->values[i]);
    }
  }
  free(heap->values);
  free(heap);
}

/**
 * Add a value into the binary heap.
 *
 * @param heap      pointer of the heap
 * @param value     value to add
 * @return          zero if successful
 */
int add_to_binary_heap(BinaryHeap *heap, BinaryHeapData value) {
  if (heap == NULL) return BINARY_HEAP_NULL_ERROR;
  if (heap->size == heap->capacity) {
    grow_values(heap);
    if (heap->size == heap->capacity) return BINARY_HEAP_MEMORY_ERROR;
  }
  heap->values[heap->size++] = value;
  sift_up(heap, heap->size - 1);
  return BINARY_HEAP_SUCCESS;
}

/**
 * Pop the value in the heap.
 *
 * @param heap  pointer of the heap
 * @return      first value of the heap.NULL if heap is empty
 */
BinaryHeapData pop_from_binary_heap(BinaryHeap *heap) {
  if (heap == NULL || heap->size == 0) return NULL;
  BinaryHeapData top = heap->values[0];
  swap_values(heap, 0, heap->size - 1);
  heap->size--;
  sift_down(heap, 0);
  return top;
}

/**
 * Peak the value in the heap.
 *
 * @param heap  pointer of the heap
 * @return      first value of the heap.NULL if heap is empty
 */
BinaryHeapData peak_binary_heap(BinaryHeap *heap) {
  if (heap == NULL || heap->size == 0) return NULL;
  return heap->values[0];
}

/**
 * Size of values stored in the heap
 *
 * @param heap  pointer of the heap
 * @return      the number of values
 */
unsigned int size_of_binary_heap(BinaryHeap *heap) {
  return heap->size;
}

static void sift_down(BinaryHeap *heap, unsigned int index) {
  if (index >= heap->size) return;
  while (left_child_index(index) < heap->size) {
    // will swap (index ,j)
    unsigned int j = left_child_index(index);
    if ((j + 1) < heap->size && heap->comparator(heap->values[j + 1], heap->values[j]) > 0) {
      // find the j that holds max value.
      j++;
    }
    if (heap->comparator(heap->values[index], heap->values[j]) >= 0) {
      // if already in right place
      break;
    }
    swap_values(heap, index, j);
    index = j;
  }
}
static void sift_up(BinaryHeap *heap, unsigned int index) {
  while (index > 0 && heap->comparator(heap->values[parent_index(index)], heap->values[index]) < 0) {
    // parent is lower than index, need sift up
    swap_values(heap, index, parent_index(index));
    index = parent_index(index);
  }
}
static unsigned int parent_index(unsigned int index) {
  return (index - 1) / 2;
}
static unsigned int left_child_index(unsigned int parent) {
  return parent * 2 + 1;
}

static unsigned int right_child_index(unsigned int parent) {
  return parent * 2 + 2;
}

static void grow_values(BinaryHeap *heap) {
  BinaryHeapData *new_values = realloc(heap->values, sizeof(BinaryHeapData) * heap->capacity * 2);
  if (new_values == NULL) return;
  heap->values = new_values;
  heap->capacity = 2 * heap->capacity;
}

static void swap_values(BinaryHeap *heap, unsigned int a, unsigned int b) {
  BinaryHeapData temp = heap->values[a];
  heap->values[a] = heap->values[b];
  heap->values[b] = temp;
}