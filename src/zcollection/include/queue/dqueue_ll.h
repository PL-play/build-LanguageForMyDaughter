//
// Created by ran on 2024/1/17.
//
#ifndef ZCOLLECTION_DQUEUE_LL_H_
#define ZCOLLECTION_DQUEUE_LL_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef struct Dqueue Dqueue;

typedef void* DqueueValue;

/**
 *
 * @return
 */
Dqueue * new_dqueue();

void free_dqueue(Dqueue* Dqueue);

int dqueue_push_head(Dqueue* Dqueue,DqueueValue value);

DqueueValue dqueue_pop_head(Dqueue* Dqueue);

DqueueValue dqueue_peek_head(Dqueue* Dqueue);

int dqueue_push_tail(Dqueue* Dqueue,DqueueValue value);

DqueueValue dqueue_pop_tail(Dqueue* Dqueue);

DqueueValue dqueue_peek_tail(Dqueue* Dqueue);

int dqueue_is_empty(Dqueue* Dqueue);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_DQUEUE_LL_H_
