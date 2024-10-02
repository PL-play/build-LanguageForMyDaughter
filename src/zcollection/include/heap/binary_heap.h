//
// Created by ran on 2024/1/4.
//

#ifndef ZCOLLECTION_HEAP_BINARY_HEAP_H_
#define ZCOLLECTION_HEAP_BINARY_HEAP_H_
#ifdef __cplusplus
extern "C" {
#endif
#define BINARY_HEAP_NULL_ERROR (-1)
#define BINARY_HEAP_SUCCESS 0
#define BINARY_HEAP_MEMORY_ERROR (-2)

/**
 * Value stored in a @ref BinaryHeap.
 */
typedef void *BinaryHeapData;

/**
 * Comparator of BinaryHeapData
 */
typedef int (*BinaryHeapComparator)(BinaryHeapData data1, BinaryHeapData data2);

/**
 * The binary heap data structure.
 */
typedef struct BinaryHeap BinaryHeap;

/**
 * Create a new binary heap.
 *
 * @param data_size     size of initial data
 * @param compare_func  comparator of value
 * @return
 */
BinaryHeap *new_binary_heap(BinaryHeapComparator compare_func);

/**
 * Free a binary heap.
 * @param heap
 */
void free_binary_heap(BinaryHeap *heap, void(*free_func)(void *));

/**
 * Add a value into the binary heap.
 *
 * @param heap      pointer of the heap
 * @param value     value to add
 * @return          zero if successful
 */
int add_to_binary_heap(BinaryHeap *heap, BinaryHeapData value);

/**
 * Pop the value in the heap.
 *
 * @param heap  pointer of the heap
 * @return      first value of the heap.NULL if heap is empty
 */
BinaryHeapData pop_from_binary_heap(BinaryHeap *heap);

/**
 * Peak the value in the heap.
 *
 * @param heap  pointer of the heap
 * @return      first value of the heap.NULL if heap is empty
 */
BinaryHeapData peak_binary_heap(BinaryHeap *heap);

/**
 * Size of values stored in the heap
 *
 * @param heap  pointer of the heap
 * @return      the number of values
 */
unsigned int size_of_binary_heap(BinaryHeap *heap);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_HEAP_BINARY_HEAP_H_
