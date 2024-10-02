//
// Created by ran on 2024/1/7.
//

#ifndef ZCOLLECTION_STACK_STACK_ll_H_
#define ZCOLLECTION_STACK_STACK_ll_H_
/**
 * stack based on linked list
 */
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef void *StackLLData;
typedef struct StackLL StackLL;

StackLL *new_stack_ll();

void free_stack_ll(StackLL *);

void push_stack_ll(StackLL *, StackLLData);

StackLLData pop_stack_ll(StackLL *);

StackLLData peek_stack_ll(StackLL *);

unsigned int size_stack_ll(StackLL *);

bool is_empty_stack_ll(StackLL *);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_STACK_STACK_ll_H_
