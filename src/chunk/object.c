//
// Created by ran on 24-4-13.
//
#include "object.h"
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
#ifdef WASM_LOG
#include <emscripten/emscripten.h>
#endif
DEFINE_HASHTABLE(int, -1, String_)
DEFINE_HASHTABLE(Value, undefined_value, Value)

const ExceptionTable
    null_exception = {.start_pc=0, .end_pc=0, .handle_pc=0, .catch_type=0};// won't match any exception.

DEFINE_ARRAY_LIST(null_exception, ExceptionTable)

void free_string(ObjString *os) {
  free(os->string);
}

void free_string_obj(ObjString *os) {
#ifdef DEBUG_GC_LOG
  printf("   free string obj[%p]: %s\n", os, os->string);
#endif
  free(os->string);
  free(os);
}

static void print_function(ObjFunction *function) {
  if (function->name == NULL) {
    printf("<g>");
    return;
  }
  printf("<func: %s>", function->name->string);
}

void print_object(Obj *obj) {
  OBJ_TOSTRING(obj)
#ifdef WASM_LOG
  char buff[len+32];
  sprintf(buff,"[bytecode]-type:%d. %s ", obj->type, string_);
  EM_ASM_({console.warn(UTF8ToString($0));}, buff);
#else
  printf("type:%d. %s ", obj->type, string_);
#endif

}

ObjString *new_string_obj(char *str, size_t length, uint32_t hash) {
  ObjString *nos = malloc(sizeof(ObjString));
  nos->obj = (Obj) {.type=OBJ_STRING, .is_marked=false};
  nos->length = length;
  nos->string = str;
  nos->hash = hash;
  return nos;
}

ObjFunction *new_function() {
  ObjFunction *function = malloc(sizeof(ObjFunction));
  function->arity = 0;
  function->upvalue_count = 0;
  function->obj = (Obj) {.type=OBJ_FUNCTION, .is_marked=false};
  function->name = NULL;
  function->chunk = init_chunk();
  function->exception_tables = ExceptionTablenew_arraylist(8);
  return function;
}

ObjNative *new_native(NativeFunction function, int arity) {
  ObjNative *native = malloc(sizeof(ObjNative));
  native->obj = (Obj) {.type=OBJ_NATIVE, .is_marked=false};
  native->function = function;
  native->arity = arity;
  return native;
}

ObjClosure *new_closure(ObjFunction *function) {
  ObjClosure *closure = malloc(sizeof(ObjClosure));
  closure->obj = (Obj) {.type=OBJ_CLOSURE, .is_marked=false};
  closure->function = function;
  ObjUpvalue **upvalues = malloc(sizeof(ObjUpvalue *) * function->upvalue_count);
  for (int i = 0; i < function->upvalue_count; ++i) {
    upvalues[i] = NULL;
  }
  closure->upvalues = upvalues;
  closure->upvalue_count = function->upvalue_count;
  return closure;
}

ObjUpvalue *new_upvalue(size_t location) {
  ObjUpvalue *upvalue = malloc(sizeof(ObjUpvalue));
  upvalue->obj = (Obj) {.type=OBJ_UPVALUE, .is_marked=false};
  upvalue->location = location;
  upvalue->next = NULL;
  upvalue->is_closed = false;
  return upvalue;
}

ObjClass *new_class(ObjString *name) {
  ObjClass *klass = malloc(sizeof(ObjClass));
  klass->obj = (Obj) {.type=OBJ_CLASS, .is_marked=false};
  klass->name = name;
  klass->methods = Valuenew_hash_table(obj_string_hash, obj_string_equals);
  klass->static_methods = Valuenew_hash_table(obj_string_hash, obj_string_equals);
  return klass;
}

ObjInstance *new_instance(ObjClass *klass) {
  ObjInstance *instance = malloc(sizeof(ObjInstance));
  instance->obj = (Obj) {.type=OBJ_INSTANCE, .is_marked=false};
  instance->klass = klass;
  instance->fields = Valuenew_hash_table(obj_string_hash, obj_string_equals);
  return instance;
}

InstanceMethod *new_instance_method(Value receiver, Obj *method) {
  InstanceMethod *m = malloc(sizeof(InstanceMethod));
  m->obj = (Obj) {.type=OBJ_INSTANCE_METHOD, .is_marked=false};
  m->method = method;
  m->receiver = receiver;
  return m;
}

ObjModule *new_module(ObjString *lib) {
  ObjModule *m = malloc(sizeof(ObjModule));
  m->obj = (Obj) {.type=OBJ_MODULE, .is_marked=false};
  m->lib = lib;
  m->next = NULL;
  m->imports = String_new_hash_table(obj_string_hash, obj_string_equals);

  return m;
}

ObjArray *new_array(ValueArrayList *arr) {
  ObjArray *a = malloc(sizeof(ObjArray));
  a->obj = (Obj) {.type=OBJ_ARRAY, .is_marked=false};
  a->array = arr == NULL ? Valuenew_arraylist(ARRAY_INIT_SIZE) : arr;
  return a;
}

uint32_t obj_string_hash(ObjString *string) {
  return string->hash;
}

int obj_string_equals(ObjString *string1, ObjString *string2) {
  if (string1->length != string2->length) {
    return -1;
  }
  return memcmp(string1->string, string2->string, string1->length);
}

void free_function(ObjFunction *function) {
  if (function == NULL) return;
  free_chunk(function->chunk);
  ExceptionTablefree_arraylist(function->exception_tables);
  free(function);
}

void free_closure(ObjClosure *closure) {
  if (closure == NULL) return;
  free(closure->upvalues);
  free(closure);
}

void free_class(ObjClass *klass) {
  if (klass == NULL) return;
  Valuefree_hash_table(klass->methods);
  Valuefree_hash_table(klass->static_methods);
  free(klass);
}

void free_instance(ObjInstance *instance) {
  if (instance == NULL) return;
  Valuefree_hash_table(instance->fields);
  free(instance);
}

void free_instance_method(InstanceMethod *method) {
  if (method == NULL) return;
  free(method);
}

void free_module(ObjModule *module) {
  if (module == NULL)return;
  // TODO free module values
  String_free_hash_table(module->imports);
  free(module);
}

void free_array(ObjArray *array) {
  if (array == NULL) return;
  Valuefree_arraylist(array->array);
  free(array);
}

void free_value(Value value) {
  if (value.type == VAL_OBJ) {
    free_object(value.obj);
  }
}

void free_object(Obj *obj) {
  if (obj == NULL) return;
  switch (obj->type) {
    case OBJ_NATIVE: {
#ifdef DEBUG_GC_LOG
      printf("free native:[%p] ", obj);
#endif
      free(obj);
      break;
    }
    case OBJ_FUNCTION: {
#ifdef DEBUG_GC_LOG
      printf("free function:[%p] ", obj);
#endif
      free_function((ObjFunction *) obj);
      break;
    }
    case OBJ_CLOSURE: {
#ifdef DEBUG_GC_LOG
      printf("free closure:[%p] ", obj);
#endif
      free_closure((ObjClosure *) obj);
      break;
    }
    case OBJ_UPVALUE: {
#ifdef DEBUG_GC_LOG
      printf("free upvalue:[%p] ", obj);
#endif
      free(obj);
      break;
    }
    case OBJ_STRING: {
#ifdef DEBUG_GC_LOG
      printf("free string:[%p] - [%s] - (%lu)", obj, ((ObjString *) obj)->string, strlen(((ObjString *) obj)->string));
#endif
      free_string_obj((ObjString *) obj);
      break;
    }
    case OBJ_CLASS: {
#ifdef DEBUG_GC_LOG
      printf("free class:[%p] ", obj);
#endif
      free_class((ObjClass *) obj);
      break;
    }
    case OBJ_INSTANCE: {
#ifdef DEBUG_GC_LOG
      printf("free instance:[%p] ", obj);
#endif
      free_instance((ObjInstance *) obj);
      break;
    }
    case OBJ_INSTANCE_METHOD: {
#ifdef DEBUG_GC_LOG
      printf("free instance method:[%p] ", obj);
#endif
      free_instance_method((InstanceMethod *) obj);
      break;
    }
    case OBJ_MODULE: {
#ifdef DEBUG_GC_LOG
      printf("free module:[%p] ", ((ObjModule *) obj)->lib->string);
#endif
      free_module((ObjModule *) obj);
      break;
    }
    case OBJ_ARRAY: {
#ifdef DEBUG_GC_LOG
      printf("free array:[%p] ", ((ObjArray *) obj));
#endif
      free_array((ObjArray *) obj);
      break;
    }
  }
}

bool value_equals(Value v1, Value v2) {
  if (v1.type != v2.type) {
    return false;
  }
  switch (v1.type) {
    case VAL_BOOL: return AS_BOOL(v1) == AS_BOOL(v2);
    case VAL_NIL: return true;
    case VAL_NUMBER: return AS_NUMBER(v1) == AS_NUMBER(v2);
    case VAL_OBJ: {
//      if (IS_STRING(v1)) {
//        ObjString *s1 = AS_STRING(v1);
//        ObjString *s2 = AS_STRING(v2);
//        return obj_string_equals(s1, s2) == 0;
//      }
      return AS_OBJ(v1) == AS_OBJ(v2);
    }
    default:return false;
  }
}