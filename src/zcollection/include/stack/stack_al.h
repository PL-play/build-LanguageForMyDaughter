//
// Created by ran on 2024/1/7.
//

#ifndef ZCOLLECTION_STACK_STACK_AL_H_
#define ZCOLLECTION_STACK_STACK_AL_H_
/**
 * stack based on array list
 */
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef void *StackALData;
typedef struct StackAL StackAL;

StackAL *new_stack_al(unsigned int init_size);

void free_stack_al(StackAL *);

void push_stack_al(StackAL *, StackALData);

StackALData pop_stack_al(StackAL *);

StackALData peek_stack_al(StackAL *);

unsigned int size_stack_al(StackAL *);

bool is_empty_stack_al(StackAL *);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_STACK_STACK_AL_H_
