//
// Created by ran on 2024-03-14.
//
#include "value.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WASM_LOG
#include <emscripten/emscripten.h>
#endif
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
#ifdef WASM_LOG
    char buff[32];
    sprintf(buff,"[bytecode]-%g", AS_NUMBER(value));
    EM_ASM_({console.warn(UTF8ToString($0));}, buff);
#else
    printf("%g", AS_NUMBER(value));
#endif
  } else if (IS_BOOL(value)) {
#ifdef WASM_LOG
    char buff[16];
    sprintf(buff,"[bytecode]-%s", AS_BOOL(value) ? "aow" : "emm");
    EM_ASM_({console.warn(UTF8ToString($0));}, buff);
#else
    printf("%s", AS_BOOL(value) ? "aow" : "emm");
#endif
  } else if (IS_NIL(value)) {
#ifdef WASM_LOG
    char buff[16];
    sprintf(buff,"[bytecode]-%s",  "nil");
    EM_ASM_({console.warn(UTF8ToString($0));}, buff);
#else
      printf("%s", "nil");
#endif
  } else if (IS_OBJ(value)) {
    print_object(value.obj);
  } else {
#ifdef WASM_LOG
      char buff[48];
      sprintf(buff,"[bytecode]-unknown value type: %d", value.type);
      EM_ASM_({console.warn(UTF8ToString($0));}, buff);
#else
      printf("unknown value type: %d", value.type);
#endif
  }
}
