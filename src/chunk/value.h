//
// Created by ran on 2024-03-14.
//

#ifndef ZHI_CHUNK_VALUE_H_
#define ZHI_CHUNK_VALUE_H_
#include <stddef.h>
#include <stdbool.h>
#include "common/common.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef char ValueType;
#define VAL_UNDEFINED ((ValueType) -1)
#define VAL_BOOL ((ValueType) 0)
#define VAL_NIL ((ValueType) 1)
#define VAL_NUMBER ((ValueType) 2)
#define VAL_OBJ ((ValueType) 3)

typedef char ObjectType;
#define OBJ_STRING ((ObjectType)0)
#define OBJ_FUNCTION ((ObjectType)1)
#define OBJ_NATIVE ((ObjectType)2)
#define OBJ_CLOSURE ((ObjectType)3)
#define OBJ_UPVALUE ((ObjectType)4)
#define OBJ_CLASS ((ObjectType)5)
#define OBJ_INSTANCE ((ObjectType)6)
#define OBJ_INSTANCE_METHOD ((ObjectType)7)
#define OBJ_MODULE ((ObjectType)8)
#define OBJ_ARRAY ((ObjectType)9)

typedef struct Value Value;
DECLARE_ARRAY_LIST(Value, Value)

typedef struct {
  ObjectType type;
  bool is_marked; // for garbage collector marking root
} Obj;

struct Value {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj *obj;
  };
};

#define BOOL_VAL(value) ((Value){.type=VAL_BOOL,.boolean=value})
#define NIL_VAL ((Value){.type=VAL_NIL,.number=0})
#define NUMBER_VAL(value) ((Value){.type=VAL_NUMBER,.number=value})
#define OBJ_VAL(object) ((Value){.type=VAL_OBJ,.obj =(Obj*) object})

#define AS_BOOL(value) ((value).boolean)
#define AS_NUMBER(value) ((value).number)
#define AS_OBJ(value) ((value).obj)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)
#define OBJ_TYPE(value) (AS_OBJ(value)->type)
static const Value null_value = {.type=VAL_NIL, .number=0xBABAD0D0};
static const Value undefined_value = {.type=VAL_UNDEFINED, .number=0xD0D00101};

void print_value(Value value);
void print_object(Obj *obj);

bool value_equals(Value v1, Value v2);
bool is_obj_type(Value value, ObjectType type);
#ifdef __cplusplus
}
#endif

#endif //ZHI_CHUNK_VALUE_H_
