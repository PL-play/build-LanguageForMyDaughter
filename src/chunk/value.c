//
// Created by ran on 2024-03-14.
//
#include "value.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef VALUE_ARRAY_INIT_SIZE
#define VALUE_ARRAY_INIT_SIZE 256
#endif

#ifdef ALLOC_TESTING
#include "common/alloc-testing.h"
#endif

DEFINE_ARRAY_LIST(null_value, Value)

bool is_obj_type(Value value, ObjectType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

void print_value(Value value) {
  if (IS_NUMBER(value)) {
    printf("%g", AS_NUMBER(value));
  } else if (IS_BOOL(value)) {
    printf("%s", AS_BOOL(value) ? "true" : "false");
  } else if (IS_NIL(value)) {
    printf("%s", "nil");
  } else if (IS_OBJ(value)) {
    print_object(value.obj);
  } else {
    printf("unknown value type: %d", value.type);
  }
}
