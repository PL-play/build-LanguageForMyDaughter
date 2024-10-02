//
// Created by ran on 2024/2/5.
//

#ifndef ZCOLLECTION_QUEUE_PRIORITY_QUEUE_H_
#define ZCOLLECTION_QUEUE_PRIORITY_QUEUE_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef void *PriorityQueueData;
typedef struct PriorityQueue PriorityQueue;
typedef int (*cmp_func)(PriorityQueueData data1, PriorityQueueData data2);

PriorityQueue *create_pq(cmp_func);
void free_pq(PriorityQueue *priority_queue, void (*free_func)(void *));

int size_of_pq(PriorityQueue *priority_queue);
int is_pq_empty(PriorityQueue *priority_queue);
PriorityQueueData pq_dequeue(PriorityQueue *priority_queue);
int pq_enqueue(PriorityQueue *priority_queue,PriorityQueueData data);
PriorityQueueData pq_peek(PriorityQueue *priority_queue);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_QUEUE_PRIORITY_QUEUE_H_
