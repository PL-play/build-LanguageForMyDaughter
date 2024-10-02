//
// Created by ran on 2024/1/7.
//
#include "stack/stack_ll.h"
#include <stdlib.h>
#include "list/linked_list.h"

struct StackLL {
  LinkedList *list;
};

StackLL *new_stack_ll() {
  StackLL *stack_ll = malloc(sizeof(StackLL));
  if (stack_ll == NULL) return NULL;
  stack_ll->list = new_linked_list();
  return stack_ll;
}

void free_stack_ll(StackLL *stack_ll) {
  if (stack_ll != NULL) {
    free_linked_list(stack_ll->list, NULL);
    free(stack_ll);
  }
}

void push_stack_ll(StackLL *stack_ll, StackLLData stack_ll_data) {
  prepend_list(stack_ll->list, stack_ll_data);
}

StackLLData pop_stack_ll(StackLL *stack_ll) {
  StackLLData data = peek_list_first(stack_ll->list);
  remove_list_first(stack_ll->list);
  return data;
}

StackLLData peek_stack_ll(StackLL *stack_ll) {
  return peek_list_first(stack_ll->list);
}

unsigned int size_stack_ll(StackLL *stack_ll) {
  return list_size(stack_ll->list);
}

bool is_empty_stack_ll(StackLL *stack_ll) {
  return size_stack_ll(stack_ll) == 0;
}
