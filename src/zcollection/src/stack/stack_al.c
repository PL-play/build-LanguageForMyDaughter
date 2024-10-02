//
// Created by ran on 2024/1/7.
//
#include "stack/stack_al.h"
#include <stdlib.h>
#include "list/array_list.h"

struct StackAL {
  ArrayList *array_list;
  int top;
  int size;
};

StackAL *new_stack_al(unsigned int init_size) {
  StackAL *stack_al = malloc(sizeof(StackAL));
  if (stack_al == NULL) return NULL;
  ArrayList *array_list = new_arraylist(init_size);
  if (array_list == NULL) {
    free(stack_al);
    return NULL;
  }
  stack_al->array_list = array_list;
  stack_al->top = 0;
  stack_al->size = 0;
  return stack_al;
}

void free_stack_al(StackAL *stack_al) {
  if (stack_al != NULL) {
    free_arraylist(stack_al->array_list);
    free(stack_al);
  }
}

void push_stack_al(StackAL *stack_al, StackALData stack_al_data) {
  insert_arraylist(stack_al->array_list, stack_al->top++, stack_al_data);
  stack_al->size++;
}

StackALData pop_stack_al(StackAL *stack_al) {
  if (is_empty_stack_al(stack_al)) return NULL;
  StackALData ret = get_data_arraylist(stack_al->array_list, stack_al->top - 1);
  remove_arraylist(stack_al->array_list, stack_al->top - 1);
  stack_al->size--;
  stack_al->top--;
  return ret;
}

StackALData peek_stack_al(StackAL *stack_al) {
  if (is_empty_stack_al(stack_al)) return NULL;
  return get_data_arraylist(stack_al->array_list, stack_al->top - 1);
}

unsigned int size_stack_al(StackAL *stack_al) {
  return stack_al->size;
}

bool is_empty_stack_al(StackAL *stack_al) {
  return stack_al->size == 0;
}
