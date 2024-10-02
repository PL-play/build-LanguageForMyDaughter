//
// Created by ran on 24-4-13.
//
#ifndef ZHI_CHUNK_OBJECT_H_
#define ZHI_CHUNK_OBJECT_H_
#include <stddef.h>
#include <stdbool.h>
#include "chunk.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  Obj obj;
  size_t length;
  char *string;
  uint32_t hash;
} ObjString;

DECLARE_HASHTABLE(ObjString *, Value, Value)

DECLARE_HASHTABLE(ObjString *, int, String_)

DECLARE_ARRAY_LIST(ExceptionTable, ExceptionTable)

typedef struct {
  Obj obj;
  int arity;
  int upvalue_count;
  Chunk *chunk;
  ObjString *name;
  ExceptionTableArrayList *exception_tables;
} ObjFunction;

typedef struct ObjUpvalue {
  Obj obj;
  size_t location;
  struct ObjUpvalue *next;
  Value closed;
  bool is_closed;
} ObjUpvalue;

typedef struct {
  Obj obj;
  ObjFunction *function;
  ObjUpvalue **upvalues;
  int upvalue_count;
} ObjClosure;

typedef Value (*NativeFunction)(int arg_count, Value *args, void *);

typedef struct {
  Obj obj;
  uint8_t arity;
  NativeFunction function;
  ObjString *name;
} ObjNative;

typedef struct {
  Obj obj;
  ObjString *name;
  ValueHashtable *methods;
  ValueHashtable *static_methods;
} ObjClass;

typedef struct {
  Obj obj;
  ObjClass *klass;
  ValueHashtable *fields;
} ObjInstance;

typedef struct {
  Obj obj;
  Value receiver;
  Obj *method; // can be function or closure
} InstanceMethod;

typedef struct ObjModule ObjModule;

struct ObjModule {
  Obj obj;
  ObjString *lib;
  ObjModule *next;
  String_Hashtable *imports;
};

typedef struct {
  Obj obj;
  ValueArrayList *array;
} ObjArray;

typedef struct {
  char *name;
  int arity;
  NativeFunction function;
} NativeFunctionDecl;

typedef struct {
  char *name;
  Value value;
} NativeValueDecl;

#define IS_STRING(value) (is_obj_type(value, OBJ_STRING))
#define IS_FUNCTION(value) (is_obj_type(value, OBJ_FUNCTION))
#define IS_NATIVE(value) (is_obj_type(value, OBJ_NATIVE))
#define IS_CLOSURE(value) (is_obj_type(value,OBJ_CLOSURE))
#define IS_CLASS(value) (is_obj_type(value, OBJ_CLASS))
#define IS_INSTANCE(value) (is_obj_type(value,OBJ_INSTANCE))
#define IS_INSTANCE_METHOD(value) (is_obj_type(value,OBJ_INSTANCE_METHOD))
#define IS_MODULE(value) (is_obj_type(value,OBJ_MODULE))
#define IS_ARRAY(value) (is_obj_type(value,OBJ_ARRAY))

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*) AS_OBJ(value))->function)
#define AS_STRING_STRING(value) (((ObjString*)AS_OBJ(value))->string)
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))
#define AS_INSTANCE_METHOD(value) ((InstanceMethod*)AS_OBJ(value))
#define AS_MODULE(value) ((ObjModule*)AS_OBJ(value))
#define AS_ARRAY(value) ((ObjArray*)AS_OBJ(value))

#define OBJ_TOSTRING(OBJ) int len = 0;\
                          switch ((OBJ)->type) {\
                            case OBJ_STRING: {\
                              len = (int) ((ObjString *) (OBJ))->length;\
                              break;\
                            }\
                            case OBJ_FUNCTION: {\
                              ObjFunction *_fn = (ObjFunction *) (OBJ);\
                              if (_fn->name == NULL) {\
                                len = 8;/*<shadow>*/\
                              } else {\
                                len = (int) _fn->name->length + 8; /*<magic:%s>*/\
                              }\
                              break;\
                            }\
                            case OBJ_CLOSURE: {\
                              ObjFunction *_fn = ((ObjClosure *) (OBJ))->function;\
                              len = (int) _fn->name->length + 8; /*<charm:%s>*/\
                              break;\
                            }         \
                            case OBJ_INSTANCE_METHOD:{                  \
                              if(((InstanceMethod *) (OBJ))->method->type == OBJ_FUNCTION){   \
                                 ObjFunction *_fn = (ObjFunction *) (((InstanceMethod *) (OBJ))->method);\
                                if (_fn->name == NULL) {\
                                  len = 3; /* <g> */ \
                                } else {\
                                  len = (int) _fn->name->length + 8;/*<magic:%s>*/\
                                }\
                                break;     \
                              }else {  \
                                 ObjFunction *_fn = ((ObjClosure *) (((InstanceMethod *) (OBJ))->method))->function;\
                                 len = (int) _fn->name->length + 8;/*<charm:%s>*/\
                                 break;\
                              } \
                            }          \
                            case OBJ_NATIVE: {\
                            len = (int) ((ObjNative *) (OBJ))->name->length + 9; /*<primal:%s>*/\
                            break;\
                            }         \
                           case OBJ_UPVALUE: {  \
                             len = 10;\
                            break;\
                            }         \
                          case OBJ_CLASS: {     \
                             ObjClass* c = ((ObjClass *) (OBJ));         \
                             if(c->name==NULL){    \
                                len = 8;         \
                             }else{   \
                               len = (int) c->name->length+9;   /*<castle:%s>*/       \
                             }         \
                             break;          \
                           }          \
                           case OBJ_INSTANCE: {     \
                             ObjInstance* i = ((ObjInstance *) (OBJ));  \
                             len = (int) i->klass->name->length+9;      /* <%s:knight>*/   \
                             break;          \
                           }          \
                          case OBJ_MODULE: {     \
                             ObjModule* i = ((ObjModule *) (OBJ));  \
                             len = (int) i->lib->length+8;       /*<realm:%s>*/   \
                             break;          \
                           }          \
                           case OBJ_ARRAY: {     \
                             ObjArray* i = ((ObjArray *) (OBJ));  \
                             len = 7;          \
                             break;          \
                           }                                 \
                          }\
                          char string_[len + 1];\
                          \
                          switch ((OBJ)->type) {\
                            case OBJ_STRING: {\
                              ObjString *_os = (ObjString *) (OBJ);\
                              memcpy(string_, _os->string, _os->length);\
                              string_[len] = '\0';\
                              break;\
                            }\
                            case OBJ_FUNCTION: {\
                              if (((ObjFunction *) (OBJ))->name == NULL) {\
                                sprintf(string_, "%s", "<shadow>");\
                              } else {\
                                sprintf(string_, "<magic:%s>", ((ObjFunction *) (OBJ))->name->string);\
                              }\
                              break;\
                            }\
                            case OBJ_CLOSURE: {\
                              sprintf(string_, "<charm:%s>", ((ObjClosure *) (OBJ))->function->name->string);\
                              break;\
                            }         \
                            case OBJ_INSTANCE_METHOD:{                   \
                               if(((InstanceMethod *) (OBJ))->method->type == OBJ_FUNCTION){   \
                                 ObjFunction *_fn = (ObjFunction *) (((InstanceMethod *) (OBJ))->method);\
                                if (_fn->name == NULL) {\
                                   sprintf(string_, "%s", "<g>");\
                                } else {\
                                  sprintf(string_, "<magic:%s>", _fn->name->string);\
                                }\
                                break;     \
                              }else {  \
                                 ObjFunction *_fn = ((ObjClosure *) (((InstanceMethod *) (OBJ))->method))->function;\
                                 sprintf(string_, "<charm:%s>", _fn->name->string);\
                                 break;\
                              }           \
                            }          \
                            case OBJ_NATIVE: {\
                              sprintf(string_, "<primal:%s>", ((ObjNative *) (OBJ))->name->string);\
                              break;\
                            }         \
                            case OBJ_UPVALUE: {  \
                              sprintf(string_, "<upvalue>");\
                            break;\
                            }         \
                            case OBJ_CLASS: {   \
                              if(((ObjClass*)(OBJ))->name == NULL){   \
                                 sprintf(string_, "%s","<castle>");        \
                              } else{       \
                              sprintf(string_, "<castle:%s>",((ObjClass*)(OBJ))->name->string);           \
                              }          \
                            break;\
                            }         \
                            case OBJ_INSTANCE: {   \
                              sprintf(string_, "<%s:knight>",((ObjInstance*)(OBJ))->klass->name->string);           \
                            break;\
                            }         \
                            case OBJ_MODULE:{   \
                               sprintf(string_, "<realm:%s>",((ObjModule*)(OBJ))->lib->string);          \
                               break;          \
                            }         \
                            case OBJ_ARRAY:{   \
                               sprintf(string_, "<array>");          \
                               break;          \
                            }          \
                            default: {\
                              string_[len] = '\0';\
                            }\
                        }

void free_string(ObjString *os);

void free_string_obj(ObjString *os);

ObjString *new_string_obj(char *str, size_t length, uint32_t hash);

ObjFunction *new_function();

ObjNative *new_native(NativeFunction function, int arity);

ObjClosure *new_closure(ObjFunction *function);

ObjUpvalue *new_upvalue(size_t location);

ObjClass *new_class(ObjString *name);

ObjInstance *new_instance(ObjClass *klass);

InstanceMethod *new_instance_method(Value receiver, Obj *method);

ObjModule *new_module(ObjString *lib);

ObjArray *new_array(ValueArrayList *arr);

void free_function(ObjFunction *function);

void free_closure(ObjClosure *closure);

void free_value(Value value);

void free_class(ObjClass *class);

void free_instance(ObjInstance *instance);

void free_instance_method(InstanceMethod *method);

void free_module(ObjModule *module);

void free_array(ObjArray *array);

uint32_t obj_string_hash(ObjString *string);

int obj_string_equals(ObjString *string1, ObjString *string2);

void free_object(Obj *obj);
#ifdef __cplusplus
}
#endif

#endif //ZHI_CHUNK_OBJECT_H_
