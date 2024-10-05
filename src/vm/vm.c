//
// Created by ran on 24-3-15.
//
#include "vm.h"
#include "chunk/value.h"
#include "compiler/compiler.h"
#include "gc.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include "common/try.h"
#ifdef ALLOC_TESTING
#include "common/alloc-testing.h"
#endif
#ifdef WASM_LOG
#include <emscripten/emscripten.h>
#endif
#include "chunk/debug.h"
#include "hashtable/hash-string.h"
#include "hashtable/compare-pointer.h"
#include "hashtable/hash-pointer.h"

#define READ_BYTE(frame) (*frame->ip++)
#define READ_3BYTES(frame) ((READ_BYTE(frame)|(READ_BYTE(frame)<<8)|(READ_BYTE(frame)<<16)))
#define READ_CONSTANT(frame) (Valueget_data_arraylist(get_frame_function(frame)->chunk->constants,READ_BYTE(frame)))
#define READ_CONSTANT_LONG(frame) (Valueget_data_arraylist(get_frame_function(frame)->chunk->constants,(READ_BYTE(frame)|(READ_BYTE(frame)<<8)|(READ_BYTE(frame)<<16))))
#define READ_CLASS_LONG(vm, frame) (Valueget_data_arraylist(vm->compile_context.class_objs,(READ_BYTE(frame)|(READ_BYTE(frame)<<8)|(READ_BYTE(frame)<<16))))

#define BINARY_OP(vm, value_type, op) do{ \
                        if(IS_NUMBER(peek(vm,0)) && IS_NUMBER(peek(vm,1))){ \
                              Value b = pop(vm);    \
                              Value a = pop(vm);    \
                              push(vm, value_type(AS_NUMBER(a) op AS_NUMBER(b))); \
                        } else{           \
                            runtime_error(vm,"Operands must be numbers."); \
                            return INTERPRET_RUNTIME_ERROR;                \
                        }\
                      }while(0)
#define EXPONENTIAL(vm, value_type, op) do{ \
                         if(!IS_NUMBER(peek(vm,0))||!IS_NUMBER(peek(vm,1))){ \
                            runtime_error(vm,"Operands must be numbers."); \
                            return INTERPRET_RUNTIME_ERROR;\
                        }      \
                        Value b = pop(vm);    \
                        Value a = pop(vm);    \
                        push(vm, value_type(op(AS_NUMBER(a) , AS_NUMBER(b))) );\
                      }while(0)
#define FACTORIAL(vm, value_type, op) do{ \
                         if(!IS_NUMBER(peek(vm,0))){ \
                            runtime_error(vm,"Operands must be number."); \
                            return INTERPRET_RUNTIME_ERROR;\
                        }      \
                        Value a = pop(vm); \
                        int v = (int) AS_NUMBER(a);      \
                        push(vm, value_type(op(v)));\
                      }while(0)

DEFINE_HASHTABLE(size_t, 0, RTObj)
DEFINE_HASHTABLE(ObjModule *, NULL, Module)

static InterpretResult run(VM *vm);

static void push(VM *vm, Value value);

static Value pop(VM *vm);

static void set(VM *vm, size_t index, Value value);

static Value peek_last(VM *vm);

static Value peek(VM *vm, size_t distance);

static void set_last(VM *vm, Value value);

static void debug_print_stack(ValueArrayList *value_array);

static void debug_print_string_table(VM *v);

static long factorial(int n);

static bool is_false(Value value);

static ObjString *string_concatenate(VM *vm, ObjString *s1, ObjString *s2);

static bool call_value(VM *vm, Value callee, int arg_count);

static bool call(VM *vm, Obj *obj, ObjFunction *callee, int arg_count);

static bool call_function(VM *vm, ObjFunction *function, int arg_count);

static bool call_closure(VM *vm, ObjClosure *closure, int arg_count);

static inline ObjFunction *get_frame_function(CallFrame *frame);

static ObjUpvalue *capture_upvalue(VM *vm, size_t location);

static void close_upvalue_from(VM *vm, size_t from_location);

static void resolve_module_abs_path(VM *vm, char *module_path, char *abs_path);

static ObjModule *register_module(VM *vm, ObjString *lib);

static void new_sub_compile_context(CompileContext *main, CompileContext *sub);

static void merge_rt_obj(VM *vm, VM *sub_vm);

static void free_vm_shallow(VM *sub_vm);

static ObjArray *peek_create(VM *vm, size_t size);

static void pop_batch(VM *vm, size_t size);

static void set_class_method(VM *vm, Value klass, Value method, bool is_static);

static ObjString *value_to_string(VM *vm, Value value);

static ObjString *path_to_string(const char *path);

static int is_double_an_int(double value);

static ObjString *get_or_create_string(VM *vm, char *string, uint32_t hash, size_t length);

typedef struct {
    int actual_start;
    int actual_end;
    bool reverse;
} SliceResult;

static SliceResult get_slice_indices(bool has_start, int start_index, bool has_end, int end_index, int arr_size);

static unsigned int next_power_of_2(unsigned int n);

static ValueArrayList *sublist(ValueArrayList *list, bool has_start, int start_index, bool has_end, int end_index);

static void runtime_error(VM *vm, const char *format, ...);

static void print_error(VM *vm, const char *format, ...);

static void reset_stack(VM *vm);

static bool should_trigger_gc(VM *vm);

static int const init_len = strlen(INIT_METHOD_NAME);
static uint32_t init_hash = 0;

static ModuleHashtable *lib_path_visited = NULL;
static String_Hashtable *on_path = NULL;

// TODO add operations on value
typedef Value (*value_add_operation)(Value a, Value b, VM *v);

static Value string_add_string(Value a, Value b, VM *v) {
}

static value_add_operation ADD_OPERATION_RULE[][13] = {
    [VAL_BOOL] = {
        [VAL_BOOL] = string_add_string,
    },
};

struct GCInfo {
    GC *gc;
    size_t bytes_allocated; // all bytes allocated during runtime.
    size_t next_GC; // next allocated gc size.
    size_t gc_call; // number of explicit GC call to execute from user.
};

long factorial(int n) {
    long f = 1;
    for (int i = 1; i <= n; ++i)
        f *= i;
    return f;
}

// *********** macro methods *****************
#define COPY_METHODS(FROM_, TO_) { \
                                    ValueHashtableIterator *i = Valuehashtable_iterator(FROM_);\
                                    while (Valuehashtable_iter_has_next(i)) {\
                                      ValueKVEntry *e = Valuehashtable_next_entry(i);\
                                      ObjString *key = Valuetable_entry_key(e);                \
                                      if(key->hash==init_hash) continue;\
                                      Value value = Valuetable_entry_value(e);\
                                      Valueput_hash_table(TO_, key, value);\
                                    }\
                                    Valuefree_hashtable_iter(i);                                   \
                                }

#define DEFINE_GLOBAL(OP_)  Valueset_arraylist_data(vm->compile_context.global_values, OP_, peek_last(vm));\
                            pop(vm);

#define GET_GLOBAL(OP_) Value value = Valueget_data_arraylist(vm->compile_context.global_values, OP_);\
                        if (value.type == VAL_UNDEFINED) {\
                          runtime_error(vm, "Undefined global variable.");\
                          return INTERPRET_RUNTIME_ERROR;\
                        }\
                        push(vm, value);

#define SET_GLOBAL(OP_) size_t index = OP_;\
                        Value value = Valueget_data_arraylist(vm->compile_context.global_values, index);\
                        if (value.type == VAL_UNDEFINED) {\
                          runtime_error(vm, "Undefined global variable.");\
                          return INTERPRET_RUNTIME_ERROR;\
                        }                  \
                        if (IS_MODULE(value)) { /* TODO is set value of module variable allowed? forbid it for now*/\
                          runtime_error(vm, "Can't set module variable.");\
                          return INTERPRET_RUNTIME_ERROR;\
                        }                                    \
                        Value v = peek_last(vm);\
                        Valueset_arraylist_data(vm->compile_context.global_values, index, v);

#define GET_PROP(OP_) Value v = peek_last(vm); \
                      ObjString *prop_name = AS_STRING(OP_); \
                      ObjInstance*instance=NULL;    \
                      ObjClass *klass=NULL;    \
                      ObjModule* module = NULL;                         \
                      if (IS_INSTANCE(v)){     /*get instance prop*/\
                        instance = AS_INSTANCE(v);           \
                        klass = instance->klass;                         \
                      } else if(IS_CLASS(v)) { /*get class static method*/\
                        klass = AS_CLASS(v);\
                      } else if(IS_MODULE(v)){ /*get module*/            \
                        module = AS_MODULE(v);                        \
                      }  else {                  \
                        runtime_error(vm, "Can't get property of non-instance or class or module.");\
                        return INTERPRET_RUNTIME_ERROR;\
                      }                       \
                      if(instance!=NULL) {                         \
                        Value p = Valueget_default_hash_table(instance->fields, prop_name, undefined_value);\
                        if (p.type != VAL_UNDEFINED) {\
                          set_last(vm, p);\
                          continue;\
                        }\
                        Value m = Valueget_default_hash_table(instance->klass->methods, prop_name, undefined_value);\
                        if (m.type != VAL_UNDEFINED) {\
                          InstanceMethod *method = new_instance_method(v, m.obj);\
                          RTObjput_hash_table(vm->objects, (RTObjHashtableKey) method,sizeof(InstanceMethod));\
                          vm->gc_info->bytes_allocated += sizeof(InstanceMethod);\
                          set_last(vm, OBJ_VAL(method));\
                          continue;\
                        }                        \
                      }                        \
                      if(klass!=NULL) { \
                         /*try static method of instance class*/                      \
                         Value sm = Valueget_default_hash_table(klass->static_methods, prop_name, undefined_value); \
                         if (sm.type != VAL_UNDEFINED) {       \
                            set_last(vm, sm);                   \
                            continue;\
                         }                         \
                      }                        \
                      if(module!=NULL){        \
                         int i = String_get_default_hash_table(module->imports,prop_name,-1);  \
                         if(i!=-1){                 \
                            set_last(vm, Valueget_data_arraylist(vm->compile_context.global_values, i));                   \
                            continue;                   \
                         }\
                      }                         \
                      runtime_error(vm, "Undefined property '%s'.", prop_name->string);\
                      return INTERPRET_RUNTIME_ERROR;

#define GET_SUPER_PROP(OP_)   Value i = pop(vm); /* 'this', which is a subclass instance*/ \
                                if (!IS_INSTANCE(i)) {\
                                  runtime_error(vm, "Get super class method receiver must be instance.");\
                                  return INTERPRET_RUNTIME_ERROR;\
                                }\
                            \
                                Value v = peek_last(vm);\
                                if (!IS_CLASS(v)) {\
                                  runtime_error(vm, "Can't get super class method of non-super class.");\
                                  return INTERPRET_RUNTIME_ERROR;\
                                }\
                                ObjClass *klass = AS_CLASS(v);\
                                ObjString *prop_name = AS_STRING(OP_);\
                            \
                                Value m = Valueget_default_hash_table(klass->methods, prop_name, undefined_value);\
                                if (m.type != VAL_UNDEFINED) {\
                                  InstanceMethod *method = new_instance_method(i, m.obj);\
                                  RTObjput_hash_table(vm->objects, (RTObjHashtableKey) method, sizeof(InstanceMethod));\
                                  vm->gc_info->bytes_allocated += sizeof(InstanceMethod);\
                                  set_last(vm, OBJ_VAL(method));\
                                  continue;\
                                }\
                                Value sm = Valueget_default_hash_table(klass->static_methods, prop_name, undefined_value);   \
                                 if (sm.type != VAL_UNDEFINED) {\
                                  set_last(vm, sm);\
                                  continue;\
                                }                                                          \
                                runtime_error(vm, "Undefined super class method '%s'.", prop_name->string);\
                                return INTERPRET_RUNTIME_ERROR;

#define SET_PROP(OP_) /**
                       * [set object][set value]
                       *         stack here ^
                       *
                       * [OP_SET_PROP][prop index]
                       *  ip here ^
                       */ \
                        Value set_value = pop(vm);/* value to be set*/\
                        Value v = peek_last(vm);/*instance*/          \
                        ObjString *prop_name = AS_STRING(OP_);  \
                        if (IS_MODULE(v)) {                         \
                          /*TODO Is set property of module allowed? In order to keep lib features consistent, I will forbid it.*/ \
                          runtime_error(vm, "Can't change property [%s] of module [%s].",prop_name->string,AS_MODULE(v)->lib->string);\
                          return INTERPRET_RUNTIME_ERROR;\
                        }    \
                        if (!IS_INSTANCE(v)) {\
                          runtime_error(vm, "Can't set property of non-instance.");\
                          return INTERPRET_RUNTIME_ERROR;\
                        } \
                          \
                        ObjInstance *instance = AS_INSTANCE(v);\
                        Valueput_hash_table(instance->fields, prop_name, set_value);\
                        set_last(vm, set_value);/*set expression results in the assigned value, leave the value on the stack.*/



#define METHOD_LOCAL(OP_) size_t slot = OP_;\
                          Value method = peek_last(vm);\
                          Value klass = peek(vm, slot);\
                          set_class_method(vm, klass, method, READ_BYTE(frame));

#define METHOD_GLOBAL(OP_)  Value method = peek_last(vm);\
                            Value klass = Valueget_data_arraylist(vm->compile_context.global_values, OP_);\
                            set_class_method(vm, klass, method, READ_BYTE(frame));

#define INVOKE(OP_) /**
                     * stack； [instance/class][arg1]..[argn]
                     * op: [OP_INVOKE][name][args count]
                     */ \
                      ObjString *prop_name = AS_STRING(OP_); /*get invoke name*/ \
                      size_t arg_count = READ_BYTE(frame); /*arg count*/                        \
                      Value v = peek(vm, arg_count); /*receiver*/\
                      ObjInstance*instance=NULL;    \
                      ObjClass *klass=NULL;                                      \
                      ObjModule* module=NULL;  \
                      if (IS_INSTANCE(v)){     /*get instance prop*/\
                        instance = AS_INSTANCE(v);           \
                        klass = instance->klass;                         \
                      } else if(IS_CLASS(v)) { /*get class static method*/\
                        klass = AS_CLASS(v);\
                      } else if(IS_MODULE(v)){  /*get class static method*/    \
                        module = AS_MODULE(v);  \
                      }\
                      else {                  \
                        runtime_error(vm, "Can't get property of non-instance or class or module.");\
                        return INTERPRET_RUNTIME_ERROR;\
                      } \
                      if(instance!=NULL){                                        \
                        Value p = Valueget_default_hash_table(instance->fields, prop_name, undefined_value);/*loop up field first*/\
                        if (p.type != VAL_UNDEFINED) {\
                          if (!call_value(vm, p, arg_count)) {\
                            return INTERPRET_RUNTIME_ERROR;\
                          }\
                          frame = &vm->frames[vm->frame_count - 1];\
                          continue;\
                        }\
                        Value m = Valueget_default_hash_table(instance->klass->methods, prop_name, undefined_value);/*field not found, find method*/\
                        if (m.type != VAL_UNDEFINED) {\
                          bool ret = IS_FUNCTION(m) ? call_function(vm, AS_FUNCTION(m), arg_count)/*"this" local is already on the stack*/\
                                                    : call_closure(vm, AS_CLOSURE(m), arg_count);\
                          if (!ret) {\
                            return INTERPRET_RUNTIME_ERROR;\
                          }\
                          frame = &vm->frames[vm->frame_count - 1];\
                          continue;\
                        }\
                      } \
                      if(klass!=NULL){                                           \
                         /*try static method of instance class*/                      \
                         Value sm = Valueget_default_hash_table(klass->static_methods, prop_name, undefined_value); \
                         if (sm.type != VAL_UNDEFINED) {       \
                            if (!call_value(vm, sm, arg_count)) {\
                              return INTERPRET_RUNTIME_ERROR;\
                            }\
                           frame = &vm->frames[vm->frame_count - 1];\
                          continue;\
                         }\
                      } \
                      if(module!=NULL){                    \
                         int i = String_get_default_hash_table(module->imports,prop_name,-1);                         \
                         if(i==-1){                              \
                           runtime_error(vm, "Undefined property '%s' in '%s'.", prop_name->string,module->lib->string);\
                           return INTERPRET_RUNTIME_ERROR;   \
                         }                                                       \
                         Value c = Valueget_data_arraylist(vm->compile_context.global_values, i);\
                         if (!call_value(vm, c, arg_count)) {                    \
                              return INTERPRET_RUNTIME_ERROR;\
                            }                                                    \
                          frame = &vm->frames[vm->frame_count - 1];             \
                          continue;\
                         \
                      }\
                      runtime_error(vm, "Undefined property '%s'.", prop_name->string);\
                      return INTERPRET_RUNTIME_ERROR;
#define INVOKE_SUPER(OP_) /**
                           * stack； [subclass instance][arg1]..[argn][superclass]
                           * op: [OP_INVOKE_SUPER][name][args count]
                           */ \
                            Value super_class = pop(vm); \
                            /**
                             if(!IS_CLASS(super_class)){  \
                              runtime_error(vm, "Can't invoke super of non-class.");\
                              return INTERPRET_RUNTIME_ERROR;  \
                            } */ \
                            ObjClass *klass = AS_CLASS(super_class);\
                            ObjString *prop_name = AS_STRING(OP_);/*get invoke name*/\
                            uint8_t arg_count = READ_BYTE(frame); /*arg count*/ \
                            Value v = peek(vm, arg_count);/*receiver*/\
                            ObjInstance *instance = AS_INSTANCE(v);\
                            Value m = Valueget_default_hash_table(klass->methods, prop_name, undefined_value);/*field not found, find method*/ \
                            if (m.type == VAL_UNDEFINED)  \
                               m = Valueget_default_hash_table(klass->static_methods, prop_name, undefined_value);/*find static method*/          \
                            if (m.type != VAL_UNDEFINED) {\
                              bool ret = IS_FUNCTION(m) ? call_function(vm, AS_FUNCTION(m), arg_count)\
                                                        : call_closure(vm, AS_CLOSURE(m), arg_count);/*"this" local is already on the stack*/\
                              if (!ret) {\
                                return INTERPRET_RUNTIME_ERROR;\
                              }\
                              frame = &vm->frames[vm->frame_count - 1];\
                              continue;\
                            } \
                            runtime_error(vm, "Undefined super method '%s'.", prop_name->string);\
                            return INTERPRET_RUNTIME_ERROR;

#define CLOSURE(OP_)  ObjFunction *function = AS_FUNCTION(OP_);\
                      ObjClosure *closure = new_closure(function);\
                      push(vm, OBJ_VAL(closure));\
                      RTObjput_hash_table(vm->objects, (RTObjHashtableKey) closure, sizeof(ObjClosure));\
                      vm->gc_info->bytes_allocated +=  sizeof(ObjClosure);\
                      for (int i = 0; i < closure->upvalue_count; ++i) {\
                        uint8_t is_local = READ_BYTE(frame);\
                        uint8_t index = READ_BYTE(frame);\
                        if (is_local) {\
                          ObjUpvalue *upvalue = capture_upvalue(vm, frame->frame_stack + index);\
                          closure->upvalues[i] = upvalue;\
                        } else {\
                          closure->upvalues[i] = ((ObjClosure *) frame->function)->upvalues[index];\
                        }\
                      }

#define CHECK_INDEX(INDEX_) if (!IS_NUMBER(INDEX_)) {\
                        runtime_error(vm, "index must be number.");\
                        return INTERPRET_RUNTIME_ERROR;\
                      }\
                      if (!is_double_an_int(AS_NUMBER(INDEX_))) {\
                        runtime_error(vm, "index must be integer.");\
                        return INTERPRET_RUNTIME_ERROR;\
                      }
#define CHECK_INDEX_RANGE(INDEX_, ARR_SIZE_) if (INDEX_ < 0 || INDEX_ > ARR_SIZE_ - 1) {\
                                runtime_error(vm, "index out of range.");\
                                return INTERPRET_RUNTIME_ERROR;\
                              }

#if defined(_WIN32) || defined(_WIN64)
#include <limits.h>
#define GET_ABSOLUTE_PATH(relative_path, absolute_path) \
        _fullpath(absolute_path, relative_path, _MAX_PATH)
#else
#include <limits.h>
#define GET_ABSOLUTE_PATH(relative_path, absolute_path) \
        realpath(relative_path, absolute_path)
#endif
// *********** macro methods *****************

static bool is_false(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value)) || (IS_NUMBER(value) && AS_NUMBER(value) == 0);
}

static ObjString *get_or_create_string(VM *vm, char *string, uint32_t hash, size_t length) {
    ObjString obj_string = {.obj = (Obj){.type = OBJ_STRING}, .length = length, .string = string, .hash = hash};
    String_KVEntry *e = String_get_entry_hash_table(vm->compile_context.string_intern, &obj_string);

    if (e != NULL) {
        return (ObjString *) String_table_entry_key(e);
    } else {
        char *str = malloc(length + 1);
        memcpy(str, string, length);
        str[length] = '\0';
        ObjString *os = new_string_obj(str, length, hash);
        String_put_hash_table(vm->compile_context.string_intern, os, 1);
        // runtime obj
        RTObjput_hash_table(vm->objects, (RTObjHashtableKey) os, sizeof(ObjString));
        vm->gc_info->bytes_allocated += sizeof(ObjString);
        return os;
    }
}

static ObjString *string_concatenate(VM *vm, ObjString *s1, ObjString *s2) {
    size_t length = s1->length + s2->length;
    char new_str[length + 1];
    memcpy(new_str, s1->string, s1->length);
    memcpy(new_str + s1->length, s2->string, s2->length);
    new_str[length] = '\0';
    uint32_t hash = fnv1a_hash(new_str, length);
    return get_or_create_string(vm, new_str, hash, length);
}

static bool call_value(VM *vm, Value callee, int arg_count) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_FUNCTION: {
                return call_function(vm, AS_FUNCTION(callee), arg_count);
            }
            case OBJ_CLOSURE: {
                return call_closure(vm, AS_CLOSURE(callee), arg_count);
            }
            case OBJ_NATIVE: {
                if (arg_count != ((ObjNative *) AS_OBJ(callee))->arity) {
                    runtime_error(vm, "Expect %d arguments but got %d", ((ObjNative *) AS_OBJ(callee))->arity,
                                  arg_count);
                    return false;
                }
                ObjNative *native = ((ObjNative *) AS_OBJ(callee));

                NativeFunction native_function = native->function;
                Value *args = arg_count == 0 ? NULL : vm->stack->data + vm->stack->size - arg_count;
                // the 3rd parameter of native functions is set to the current VM pointer
                Value result = native_function(arg_count, args, vm);
                vm->stack->size -= (arg_count + 1);
                push(vm, result);
                return true;
            }
            case OBJ_CLASS: {
                ObjClass *klass = AS_CLASS(callee);
                // add to run time object
                ObjInstance *instance = new_instance(klass);
                RTObjput_hash_table(vm->objects, (RTObjHashtableKey) instance, sizeof(ObjInstance));
                vm->gc_info->bytes_allocated += sizeof(ObjInstance);
                // find init method
                ObjString
                        obj_string = {
                            .obj = (Obj){.type = OBJ_STRING}, .length = init_len, .string = INIT_METHOD_NAME,
                            .hash = init_hash
                        };
                Value init =
                        Valueget_default_hash_table(klass->methods, (ValueHashtableKey) &obj_string, undefined_value);
                if (init.type == VAL_UNDEFINED) {
                    if (arg_count != 0) {
                        runtime_error(vm, "Expect 0 arguments but got %d", arg_count);
                        return false;
                    }
                    // pop class object
                    pop(vm);
                    push(vm, OBJ_VAL(instance));
                    return true;
                } else {
                    bool is_func = IS_FUNCTION(init);

                    //InstanceMethod *method = new_instance_method(OBJ_VAL(instance), init.obj);
                    //RTObjput_hash_table(vm->objects, (RTObjHashtableKey) method, OBJ_SIZE[OBJ_INSTANCE_METHOD]);
                    //vm->gc_info->bytes_allocated += OBJ_SIZE[OBJ_INSTANCE_METHOD];

                    // set "this" local on the stack
                    vm->stack->data[vm->stack->size - arg_count - 1] = OBJ_VAL(instance);
                    // call init method
                    return is_func
                               ? call_function(vm, (ObjFunction *) init.obj, arg_count)
                               : call_closure(vm, (ObjClosure *) init.obj, arg_count);
                }
            }
            case OBJ_INSTANCE_METHOD: {
                InstanceMethod *method = AS_INSTANCE_METHOD(callee);
                // set "this" local on the stack
                vm->stack->data[vm->stack->size - arg_count - 1] = method->receiver;
                return method->method->type == OBJ_FUNCTION
                           ? call_function(vm, (ObjFunction *) method->method, arg_count)
                           : call_closure(vm, (ObjClosure *) method->method, arg_count);
            }
            default: break;
        }
    }
    runtime_error(vm, "Can only call functions, methods and classes.");
    return false;
}

static bool call(VM *vm, Obj *obj, ObjFunction *callee, int arg_count) {
    if (arg_count != callee->arity) {
        runtime_error(vm, "Expect %d arguments but got %d", callee->arity, arg_count);
        return false;
    }
    if (vm->frame_count == MAX_CALL_STACK) {
        runtime_error(vm, "Stack overflow.");
        return false;
    }

    CallFrame *frame = &vm->frames[vm->frame_count++];
    frame->function = obj;
    frame->ip = callee->chunk->code->data;
    //
    //               (-1)    {--arg count--} stack size
    //                       |             |    ↓
    //  0       1   2     3     4        ...
    // [script][v1][func][arg1][arg2]...[argn]....  <- stack
    //              ^
    //              |
    //      frame->window_stack


    frame->frame_stack = vm->stack->size - arg_count - 1;
    return true;
}

static inline ObjFunction *get_frame_function(CallFrame *frame) {
    if (frame->function->type == OBJ_FUNCTION) {
        return (ObjFunction *) frame->function;
    } else {
        return ((ObjClosure *) frame->function)->function;
    }
}

static ObjUpvalue *capture_upvalue(VM *vm, size_t location) {
    // look for existing upvalue first
    ObjUpvalue *prev = NULL;
    ObjUpvalue *value = vm->open_upvalues;
    while (value != NULL && value->location > location) {
        prev = value;
        value = value->next;
    }
    if (value != NULL && value->location == location) {
        // found existing value
        return value;
    }
    // not found, create a new upvalue
    ObjUpvalue *upvalue = new_upvalue(location);
    RTObjput_hash_table(vm->objects, (RTObjHashtableKey) upvalue, sizeof(ObjUpvalue));
    vm->gc_info->bytes_allocated += sizeof(ObjUpvalue);

    // insert upvalue into list
    upvalue->next = value;
    if (prev != NULL) {
        prev->next = upvalue;
    } else {
        vm->open_upvalues = upvalue;
    }
    return upvalue;
}

static void close_upvalue_from(VM *vm, size_t from_location) {
    while (vm->open_upvalues != NULL && vm->open_upvalues->location >= from_location) {
        ObjUpvalue *upvalue = vm->open_upvalues;
        upvalue->closed = vm->stack->data[upvalue->location];
        upvalue->is_closed = true;
        vm->open_upvalues = upvalue->next;
    }
}

static void get_parent_directory(const char *path, char *parent_dir) {
    if (path == NULL) return;
    // Find the last occurrence of '/' or '\'
    const char *last_slash = strrchr(path, '/');
    const char *last_backslash = strrchr(path, '\\');

    // Determine the position of the last path separator
    const char *last_separator = last_slash > last_backslash ? last_slash : last_backslash;

    if (last_separator != NULL) {
        // Calculate the length of the parent directory path
        size_t parent_length = last_separator - path;

        // Copy the parent directory path to the output buffer
        strncpy(parent_dir, path, parent_length);
        parent_dir[parent_length] = '\0'; // Null-terminate the string
    } else {
        // If no separator is found, the path is the root or invalid
        strcpy(parent_dir, ""); // Return an empty string
    }
}

/**
 * Resolve the module path with given module path. save the absolute path in abs_path
 *
 * @param vm
 * @param module_path
 * @param abs_path
 */
static void resolve_module_abs_path(VM *vm, char *module_path, char *abs_path) {
    // TODO resolve path from default lib path if not found
    char dir[PATH_MAX];
    // if source_path of current vm is NULL, the get default working directory
    if (vm->source_path == NULL) {
        if (getcwd(dir, sizeof(dir)) == NULL) {
            print_error(vm, "Can't get current working directory.");
            exit(70);
        }
    } else {
        get_parent_directory(vm->source_path, dir);
    }

    char fullPath[PATH_MAX + 2];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, module_path);

    // Get the absolute path
    if (GET_ABSOLUTE_PATH(fullPath, abs_path) == NULL) {
        abs_path[0] = '\0';
    }
#ifdef DEBUG_TRACE_EXECUTION
  printf("module [%s] abs path: %s\n", module_path, abs_path);
#endif
}

static void set_class_method(VM *vm, Value klass, Value method, bool is_static) {
    ObjClass *k = AS_CLASS(klass);
    if (IS_FUNCTION(method)) {
        Valueput_hash_table(is_static ? k->static_methods : k->methods, AS_FUNCTION(method)->name, method);
    } else {
        Valueput_hash_table(is_static ? k->static_methods : k->methods, AS_CLOSURE(method)->function->name, method);
    }
}

static bool call_function(VM *vm, ObjFunction *function, int arg_count) {
    return call(vm, (Obj *) function, function, arg_count);
}

static bool call_closure(VM *vm, ObjClosure *closure, int arg_count) {
    return call(vm, (Obj *) closure, closure->function, arg_count);
}

VM *init_VM(VM *v, VM *enclosing, CompileContext *previous_context, bool register_native) {
    v->stack = Valuenew_arraylist(16);
    v->frame_count = 0;
    // upvalue initial is NULL
    v->open_upvalues = NULL;
    v->enclosing = enclosing;
    // runtime created objects
    v->objects = RTObjnew_hash_table((RTObjHashtableHashFunc) hash_pointer, (RTObjHashtableEqualsFunc) pointer_compare);
    GCInfo *gc_info = malloc(sizeof(GCInfo));
    gc_info->bytes_allocated = 0;
    gc_info->next_GC = INITIAL_GC_SIZE;
    gc_info->gc_call = 0;
    GC *gc = malloc(sizeof(GC));
    init_GC(gc);
    gc_info->gc = gc;

    v->gc_info = gc_info;
    ObjFunction *function = new_function();
    CompileContext context;
    if (previous_context == NULL) {
        context = (CompileContext){
            .string_intern = String_new_hash_table(obj_string_hash, obj_string_equals),
            .objs = new_linked_list(),
            .global_names = String_new_hash_table(obj_string_hash, obj_string_equals),
            .global_values = Valuenew_arraylist(16),
            .class_objs = Valuenew_arraylist(16)
        };
        v->compile_context = context;
    } else {
        context = *previous_context;
        v->compile_context = context;
    }

    if (register_native) {
        // native functions
        register_native_function(&context);
        // native global values
        register_native_values(&context);
    }
    // the "base pointer" of function
    push(v, OBJ_VAL(function));
    call(v, (Obj *) function, function, 0);
    // init method name
    if (init_hash == 0) {
        init_hash = fnv1a_hash(INIT_METHOD_NAME, init_len);
    }
    // init lib path visited map
    if (lib_path_visited == NULL) {
        lib_path_visited = Modulenew_hash_table(obj_string_hash, obj_string_equals);
    }
    // init on path
    if (on_path == NULL) {
        on_path = String_new_hash_table(obj_string_hash, obj_string_equals);
    }
    return v;
}

/**
 * So many good memories to free :(
 * GC makes things even more sophisticated :(
 *
 * @param v
 */
void free_VM(VM *v) {
#ifdef DEBUG_TRACE_EXECUTION
  debug_print_stack(v->stack);
#endif
#ifdef DEBUG_TRACE_EXECUTION
  debug_print_string_table(v);
#endif

#ifdef DEBUG_GC_LOG
  printf("+++++ free vm starts ++++++\n");
#endif
    //  // trigger GC for the last time, this is supposed to free any objects during runtime.
    //  Valueclear_arraylist(v->stack);
    //  collect_garbage(v->gc_info->gc, v);

    // free the "base frame"
#ifdef DEBUG_GC_LOG
  printf("++free base frame\n");
#endif
    free_function((ObjFunction *) v->frames[0].function);
#ifdef DEBUG_GC_LOG
  printf("--free base frame\n");
#endif
    // free VM's stack array
#ifdef DEBUG_GC_LOG
  printf("++free stack array\n");
#endif
    Valuefree_arraylist(v->stack);
#ifdef DEBUG_GC_LOG
  printf("--free stack array\n");
  printf("++free global values\n");
#endif
    // free global values
    //  for (size_t i = 0; i < v->compile_context.global_values->size; ++i) {
    //    Value value = Valueget_data_arraylist(v->compile_context.global_values, i);
    //    if (IS_STRING(value)) {
    //#ifdef DEBUG_GC_LOG
    //      // if value is string, remove it from string interning hashmap to avoid double free
    //      printf("    gv remove [%p]:[%s] from string intern map. (%zu)\n",
    //             AS_STRING(value),
    //             AS_STRING(value)->string,
    //             String_size_of_hash_table(v->compile_context.string_intern));
    //#endif
    //      String_remove_hash_table(v->compile_context.string_intern, AS_STRING(value));
    //#ifdef DEBUG_GC_LOG
    //      printf("    gv removed. (%zu)\n", String_size_of_hash_table(v->compile_context.string_intern));
    //#endif
    //    } else if (IS_FUNCTION(value) || IS_CLOSURE(value) || IS_NATIVE(value) || IS_CLASS(value) || IS_INSTANCE(value)
    //        || IS_INSTANCE_METHOD(value)) {
    //      // do not free functions because they are in compiling objects or runtime objects
    //      continue;
    //    }
    //#ifdef DEBUG_GC_LOG
    //    printf("    free global value: ");
    //#endif
    //    free_value(value);
    //#ifdef DEBUG_GC_LOG
    //    printf("\n");
    //#endif
    //  }
#ifdef DEBUG_GC_LOG
  printf("--free global values\n");

  printf("++free runtime objects\n");
#endif
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-ZHI free runtime objects.");
#endif
    // free runtime objects
    RTObjHashtableIterator *it = RTObjhashtable_iterator(v->objects);
    while (RTObjhashtable_iter_has_next(it)) {
        RTObjKVEntry *entry = RTObjhashtable_next_entry(it);
        Obj *obj = RTObjtable_entry_key(entry);
        if (obj->type == OBJ_STRING) {
            //#ifdef DEBUG_GC_LOG
            //      printf("    rt remove [%p]:[%s] from string intern map. (%zu)\n", obj, ((ObjString *) obj)->string,
            //             String_size_of_hash_table(v->compile_context.string_intern));
            //#endif
            //      String_remove_hash_table(v->compile_context.string_intern, (String_HashtableKey) obj);
            //#ifdef DEBUG_GC_LOG
            //      printf("    rt removed. (%zu)\n", String_size_of_hash_table(v->compile_context.string_intern));
            //#endif
            continue;
        }
        if (obj->type == OBJ_FUNCTION) {
            // do not free functions because they are in compiling objects
            continue;
        }
        free_object(obj);
        printf("\n");
    }
    RTObjfree_hashtable_iter(it);
    RTObjfree_hash_table(v->objects);
#ifdef DEBUG_GC_LOG
  printf("--free runtime objects\n");

  printf("++free string interning\n");
#endif
    // free string interning
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-ZHI free string interning.");
#endif
    String_register_hashtable_free_functions(v->compile_context.string_intern, free_string_obj, NULL);
    //  String_HashtableIterator *iter = String_hashtable_iterator(v->compile_context.string_intern);
    //  while (String_hashtable_iter_has_next(iter)) {
    //    String_KVEntry *entry = String_hashtable_next_entry(iter);
    //    ObjString *obj = String_table_entry_key(entry);
    //    printf("   free string intern: [%p] - [%s] - (%lu)\n", obj, obj->string, strlen(obj->string));
    //    free_string_obj(obj);
    //  }
    //  String_free_hashtable_iter(iter);

    String_free_hash_table(v->compile_context.string_intern);
#ifdef DEBUG_GC_LOG
  printf("--free string interning\n");
  // free objects during compiling
  printf("++free objects during compiling\n");
#endif
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-ZHI free objects during compiling.");
#endif
    free_linked_list(v->compile_context.objs, (void (*)(void *)) free_object);
#ifdef DEBUG_GC_LOG
  printf("--free objects during compiling\n");

  printf("\n++free global value array.\n");
#endif
    // free global value array
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-ZHI free global value.");
#endif
    Valuefree_arraylist(v->compile_context.global_values);

#ifdef DEBUG_GC_LOG
  printf("\n--free global value array.\n");
  // free global names
  printf("++free class objects array.\n");
#endif
    // free class names array
    for (size_t i = 0; i < v->compile_context.class_objs->size; ++i) {
        Value klass = Valueget_data_arraylist(v->compile_context.class_objs, i);
        free_value(klass);
    }
    Valuefree_arraylist(v->compile_context.class_objs);
#ifdef DEBUG_GC_LOG
  printf("\n--free class objects array.\n");
  // free global names
  printf("++free global names map.\n");
#endif
    String_free_hash_table(v->compile_context.global_names);

#ifdef DEBUG_GC_LOG
  printf("--free global names map.\n");
  printf("++free gc info.\n");
#endif
    free_GC(v->gc_info->gc);
    free(v->gc_info);
#ifdef DEBUG_GC_LOG
  printf("--free gc info.\n");
  printf("+++++ free vm ends ++++++\n");
#endif

    if (lib_path_visited != NULL) {
        Moduleregister_hashtable_free_functions(lib_path_visited, (ModuleHashtableKeyFreeFunc) free_string_obj, NULL);
        Modulefree_hash_table(lib_path_visited);
        lib_path_visited = NULL;
    }
    if (on_path != NULL) {
        String_free_hash_table(on_path);
        on_path = NULL;
    }
}

InterpretResult interpret_file(VM *v, const char *file_path) {
    // TODO
}

InterpretResult interpret(VM *v, const char *source_path, const char *source) {
    v->source_path = source_path;

    ObjString *path_string = NULL;
    if (source_path != NULL) {
        path_string = path_to_string(source_path);
        // add to visited
        Moduleput_hash_table(lib_path_visited, path_string, NULL);
        // add to path
        String_put_hash_table(on_path, path_string, 1);
    }

    StatementArrayList *stmts = NULL;
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-Initialize scanner and parser.");
#endif
    Parser parser;

#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-Parsing.");
#endif
    int parse_result = parse(&parser, source, &stmts);
    if (parse_result != PARSE_OK) {
        free_statements(&parser, stmts);
        Statementfree_arraylist(stmts);
        return PARSE_ERROR;
    }
#ifdef WASM_LOG
    char* ast = print_statements(&parser, stmts);
    char buffer[strlen(ast)+7];
    sprintf(buffer,"[ast]-%s",ast);
    EM_ASM_({console.warn(UTF8ToString($0));}, buffer);
    free(ast);
#endif

    // init compiler
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-Initialize compiler.");
#endif
    Compiler compiler;
    compiler.function = (ObjFunction *) v->frames[0].function;
    compiler.type = COMPILE_GLOBAL;
    compiler.enclosing = NULL;
    compiler.context = &v->compile_context;
    compiler.parser = &parser;
    LocalContext local_context;
    compiler.local_context = &local_context;

    size_t prev_op = get_code_size(get_frame_function(&v->frames[0])->chunk);
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-Compiling...");
    EM_ASM_({console.warn(UTF8ToString($0));}, "[bytecode]---- bytecodes of [main script] -----[nl]");
    EM_ASM_({console.warn(UTF8ToString($0));
         }, "[bytecode]-offset |  line | code  |   constant index  |  value[nl]");
#endif
    if (compile(&compiler, stmts) != COMPILE_OK) {
        free_statements(&parser, stmts);
        Statementfree_arraylist(stmts);
        get_frame_function(&v->frames[0])->chunk->code->size = prev_op;
        return INTERPRET_COMPILE_ERROR;
    }
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-Preparing byte codes....");
#endif

    v->frames[0].ip = get_frame_function(&v->frames[0])->chunk->code->data + prev_op;

    //v->ip = v->chunk->code + v->prev_top;
#ifdef WASM_LOG
    EM_ASM_({console.warn(UTF8ToString($0));}, "[status]-ZHI VM interpreting...");
#endif
    InterpretResult result = run(v);
    free_statements(&parser, stmts);
    Statementfree_arraylist(stmts);
#ifdef DEBUG_TRACE_EXECUTION
  //debug_print_stack(v->stack);
  printf("********* uncollected object list ************\n");
  if (list_size(v->compile_context.objs) > 0) {
    LinkedListNode *list_node = head_of_list(v->compile_context.objs);
    LinkedListNode *head = list_node;
    do {
      Obj *o = data_of_node_linked_list(list_node);
      print_object(o);
      printf("\n");
      list_node = next_node_linked_list(list_node);
    } while (list_node != head);
  }
  printf("********* uncollected object list ************\n");
#endif
    if (path_string != NULL) {
        // remove path
        String_remove_hash_table(on_path, path_string);
    }
    return result;
}

static InterpretResult run(VM *vm) {
#ifdef __GNUC__
    // Direct Threaded-Code works in GCC
    // index of array must correspond with OpCode defined in chunk.h
    void *instruction_pointers[] = {
        [OP_RETURN] = &&INS_RETURN,
        [OP_CONSTANT] = &&INS_CONSTANT,
        [OP_CONSTANT_LONG] = &&INS_CONSTANT_LONG,
        [OP_NEGATE] = &&INS_NEGATE,
        [OP_ADD] = &&INS_ADD,
        [OP_SUBTRACT] = &&INS_SUBTRACT,
        [OP_MULTIPLY] = &&INS_MULTIPLY,
        [OP_DIVIDE] = &&INS_DIVIDE,
        [OP_EXPONENT] = &&INS_EXPONENT,
        [OP_FACTORIAL] = &&INS_FACTORIAL,
        [OP_NOT] = &&INS_NOT,
        [OP_NIL] = &&INS_NIL,
        [OP_TRUE] = &&INS_TRUE,
        [OP_FALSE] = &&INS_FALSE,
        [OP_EQUAL] = &&INS_EQUAL,
        [OP_GREATER] = &&INS_GREATER,
        [OP_LESS] = &&INS_LESS,
        [OP_PRINT] = &&INS_PRINT,
        [OP_POP] = &&INS_POP,
        [OP_DEFINE_GLOBAL] = &&INS_DEFINE_GLOBAL,
        [OP_DEFINE_GLOBAL_LONG] = &&INS_DEFINE_GLOBAL_LONG,
        [OP_GET_GLOBAL] = &&INS_GET_GLOBAL,
        [OP_GET_GLOBAL_LONG] = &&INS_GET_GLOBAL_LONG,
        [OP_SET_GLOBAL] = &&INS_SET_GLOBAL,
        [OP_SET_GLOBAL_LONG] = &&INS_SET_GLOBAL_LONG,
        [OP_SET_LOCAL] = &&INS_SET_LOCAL,
        [OP_SET_LOCAL_LONG] = &&INS_SET_LOCAL_LONG,
        [OP_GET_LOCAL] = &&INS_GET_LOCAL,
        [OP_GET_LOCAL_LONG] = &&INS_GET_LOCAL_LONG,
        [OP_JUMP_IF_FALSE] = &&INS_JUMP_IF_FALSE,
        [OP_JUMP] = &&INS_JUMP,
        [OP_JUMP_BACK] = &&INS_JUMP_BACK,
        [OP_MOD] = &&INS_MOD,
        [OP_CALL] = &&INS_CALL,
        [OP_CLOSURE] = &&INS_CLOSURE,
        [OP_CLOSURE_LONG] = &&INS_CLOSURE_LONG,
        [OP_GET_UPVALUE] = &&INS_GET_UPVALUE,
        [OP_SET_UPVALUE] = &&INS_SET_UPVALUE,
        [OP_CLOSE_UPVALUE] = &&INS_CLOSE_UPVALUE,
        [OP_CLASS] = &&INS_CLASS,
        [OP_CLASS_LONG] = &&INS_CLASS_LONG,
        [OP_GET_PROP] = &&INS_GET_PROP,
        [OP_GET_PROP_LONG] = &&INS_GET_PROP_LONG,
        [OP_SET_PROP] = &&INS_SET_PROP,
        [OP_SET_PROP_LONG] = &&INS_SET_PROP_LONG,
        [OP_METHOD_LOCAL] = &&INS_METHOD_LOCAL,
        [OP_METHOD_LOCAL_LONG] = &&INS_METHOD_LOCAL_LONG,
        [OP_METHOD_GLOBAL] = &&INS_METHOD_GLOBAL,
        [OP_METHOD_GLOBAL_LONG] = &&INS_METHOD_GLOBAL_LONG,
        [OP_INVOKE] = &&INS_INVOKE,
        [OP_INVOKE_LONG] = &&INS_INVOKE_LONG,
        [OP_GET_SUPER_PROP] = &&INS_GET_SUPER_PROP,
        [OP_GET_SUPER_PROP_LONG] = &&INS_GET_SUPER_PROP_LONG,
        [OP_INVOKE_SUPER] = &&INS_INVOKE_SUPER,
        [OP_INVOKE_SUPER_LONG] = &&INS_INVOKE_SUPER_LONG,
        [OP_INHERIT] = &&INS_INHERIT,
        [OP_IMPORT] = &&INS_IMPORT,
        [OP_ARRAY] = &&INS_ARRAY,
        [OP_INDEXING] = &&INS_INDEXING,
        [OP_ELEMENT_ASSIGN] = &&INS_ELEMENT_ASSIGN,
        [OP_SLICING] = &&INS_SLICING,
    };
#endif
    bool volatile return_error = false;
    CallFrame *frame = &vm->frames[vm->frame_count - 1];
    for (;;) {
        return_error = false;
#ifdef DEBUG_TRACE_EXECUTION
    debug_print_stack(vm->stack);
    disassemble_instruction(get_frame_function(frame)->chunk,
                            (size_t) (frame->ip - get_frame_function(frame)->chunk->code->data));
#endif

#ifdef __GNUC__
        goto
        *instruction_pointers[READ_BYTE(frame)];

    INS_CONSTANT: {
            Value constant = READ_CONSTANT(frame);
            push(vm, constant);
            continue;
        }

    INS_CONSTANT_LONG: {
            Value constant = READ_CONSTANT_LONG(frame);
            push(vm, constant);
            continue;
        }

    INS_CLASS: {
            Value klass = READ_CONSTANT(frame);
            push(vm, klass);
            continue;
        }

    INS_CLASS_LONG: {
            Value klass = READ_CONSTANT_LONG(frame);
            push(vm, klass);
            continue;
        }

    INS_INHERIT: {
            // [class][superclass]
            Value v = pop(vm);
            if (!IS_CLASS(v)) {
                runtime_error(vm, "Can only inherit from class.");
                return INTERPRET_RUNTIME_ERROR;
            }
            ObjClass *superclass = AS_CLASS(v);
            ObjClass *subclass = AS_CLASS(peek_last(vm));
            // copy method from superclass
            COPY_METHODS(superclass->methods, subclass->methods)
            // copy static methods
            COPY_METHODS(superclass->static_methods, subclass->static_methods)
            continue;
        }

    INS_NEGATE: {
            Value v = peek_last(vm);
            if (IS_NUMBER(v)) {
                set_last(vm, NUMBER_VAL(-AS_NUMBER(v)));
            } else {
                // runtime error
                runtime_error(vm, "Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            continue;
        }

    INS_ADD: {
            if (IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
                Value b = pop(vm);
                Value a = pop(vm);
                ObjString *new_str = string_concatenate(vm, AS_STRING(a), AS_STRING(b));
                push(vm, OBJ_VAL(new_str));
            } else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, NUMBER_VAL(AS_NUMBER(a) + AS_NUMBER(b)));
            } else {
                //TODO
                Value b = pop(vm);
                Value a = pop(vm);
                ObjString *sa = value_to_string(vm, a);
                if (sa == NULL) return INTERPRET_RUNTIME_ERROR;
                ObjString *sb = value_to_string(vm, b);
                if (sb == NULL) return INTERPRET_RUNTIME_ERROR;
                ObjString *new_str = string_concatenate(vm, sa, sb);
                push(vm, OBJ_VAL(new_str));
            }

            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in add\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_SUBTRACT: {
            BINARY_OP(vm, NUMBER_VAL, -);

            continue;
        }

    INS_MULTIPLY: {
            BINARY_OP(vm, NUMBER_VAL, *);
            continue;
        }

    INS_DIVIDE: {
            BINARY_OP(vm, NUMBER_VAL, /);
            continue;
        }

    INS_EXPONENT: {
            EXPONENTIAL(vm, NUMBER_VAL, pow);
            continue;
        }

    INS_FACTORIAL: {
            FACTORIAL(vm, NUMBER_VAL, factorial);
            continue;
        }

    INS_NOT: {
            Value v = peek_last(vm);
            set_last(vm, BOOL_VAL(is_false(v)));
            continue;
        }

    INS_NIL: {
            push(vm, NIL_VAL);
            continue;
        }

    INS_TRUE: {
            push(vm, BOOL_VAL(true));
            continue;
        }

    INS_FALSE: {
            push(vm, BOOL_VAL(false));
            continue;
        }

    INS_EQUAL: {
            Value b = pop(vm);
            Value a = pop(vm);
            push(vm, BOOL_VAL(value_equals(a, b)));
            continue;
        }

    INS_GREATER: {
            BINARY_OP(vm, BOOL_VAL, >);
            continue;
        }

    INS_LESS: {
            BINARY_OP(vm, BOOL_VAL, <);
            continue;
        }

    INS_PRINT: {
            Value v = pop(vm);
            ObjString *sa = value_to_string(vm, v);
            if (sa == NULL) return INTERPRET_RUNTIME_ERROR;
            printf("%s", sa->string);

            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("****gc in print\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_POP: {
            pop(vm);

            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in pop\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_DEFINE_GLOBAL: {
            DEFINE_GLOBAL(READ_BYTE(frame))
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in define global\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_DEFINE_GLOBAL_LONG: {
            DEFINE_GLOBAL(READ_3BYTES(frame))

            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in define global long\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_GET_GLOBAL: {
            GET_GLOBAL(READ_BYTE(frame))
            continue;
        }

    INS_GET_GLOBAL_LONG: {
            GET_GLOBAL(READ_3BYTES(frame))
            continue;
        }

    INS_SET_GLOBAL: {
            SET_GLOBAL(READ_BYTE(frame))
            continue;
        }

    INS_SET_GLOBAL_LONG: {
            SET_GLOBAL(READ_3BYTES(frame))
            continue;
        }

    INS_SET_LOCAL: {
            uint8_t slot = READ_BYTE(frame);
            vm->stack->data[frame->frame_stack + slot] = peek_last(vm);
            // frame->window_stack[slot] = peek_last(vm);
            continue;
        }

    INS_SET_LOCAL_LONG: {
            size_t slot = READ_3BYTES(frame);
            vm->stack->data[frame->frame_stack + slot] = peek_last(vm);
            // frame->window_stack[slot] = peek_last(vm);
            continue;
        }

    INS_GET_LOCAL: {
            uint8_t slot = READ_BYTE(frame);
            // accesses the current frame’s slots array by accessing the given numbered slot relative to the
            // beginning of that frame.
            push(vm, vm->stack->data[frame->frame_stack + slot]);
            //      push(vm, frame->window_stack[slot]);
            continue;
        }

    INS_GET_LOCAL_LONG: {
            size_t slot = READ_3BYTES(frame);
            push(vm, vm->stack->data[slot]);
            continue;
        }

    INS_GET_PROP: {
            GET_PROP(READ_CONSTANT(frame))
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in GET_PROP\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_GET_PROP_LONG: {
            GET_PROP(READ_CONSTANT_LONG(frame))
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in GET_PROP_LONG\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_GET_SUPER_PROP: {
            GET_SUPER_PROP(READ_CONSTANT(frame))
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in GET_SUPER_PROP\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_GET_SUPER_PROP_LONG: {
            // 'this', which is a subclass instance
            GET_SUPER_PROP(READ_CONSTANT_LONG(frame))
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in GET_SUPER_PROP_LONG\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_SET_PROP: {
            SET_PROP(READ_CONSTANT(frame))
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in set prop\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_SET_PROP_LONG: {
            SET_PROP(READ_CONSTANT_LONG(frame))
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in set prop long\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_METHOD_LOCAL: {
            METHOD_LOCAL(READ_BYTE(frame))
            continue;
        }

    INS_METHOD_LOCAL_LONG: {
            METHOD_LOCAL(READ_3BYTES(frame))
            continue;
        }

    INS_METHOD_GLOBAL: {
            METHOD_GLOBAL(READ_BYTE(frame))
            continue;
        }

    INS_METHOD_GLOBAL_LONG: {
            METHOD_GLOBAL(READ_3BYTES(frame))
            continue;
        }

    INS_JUMP_IF_FALSE: {
            size_t offset = READ_3BYTES(frame);
            size_t jump_size = is_false(peek_last(vm)) * offset;
            //vm->ip += jump_size;
            frame->ip += jump_size;
            continue;
        }

    INS_JUMP: {
            //vm->ip += READ_3BYTES(frame);
            frame->ip += READ_3BYTES(frame);
            continue;
        }

    INS_JUMP_BACK: {
            // vm->ip -= READ_3BYTES(frame);
            frame->ip -= READ_3BYTES(frame);
            continue;
        }

    INS_MOD: {
            if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
                Value b = pop(vm);
                Value a = pop(vm);
                if (!(is_double_an_int(b.number) && is_double_an_int(a.number))) {
                    runtime_error(vm, "Operands of '%' must be int.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm, NUMBER_VAL((int) a.number % (int) b.number));
            } else {
                runtime_error(vm, "Operands must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }
            continue;
        }

    INS_INVOKE: {
            INVOKE(READ_CONSTANT(frame))
            continue;
        }

    INS_INVOKE_LONG: {
            INVOKE(READ_CONSTANT_LONG(frame))
            continue;
        }

    INS_INVOKE_SUPER: {
            INVOKE_SUPER(READ_CONSTANT(frame))
            continue;
        }

    INS_INVOKE_SUPER_LONG: {
            INVOKE_SUPER(READ_CONSTANT_LONG(frame))
            continue;
        }

    INS_CALL: {
            int arg_count = READ_BYTE(frame);
            if (!call_value(vm, peek(vm, arg_count), arg_count)) {
                return INTERPRET_RUNTIME_ERROR;
            }
            frame = &vm->frames[vm->frame_count - 1];

            // GC
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in call\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_CLOSURE: {
            CLOSURE(READ_CONSTANT(frame))

            // GC
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in closure\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_CLOSURE_LONG: {
            CLOSURE(READ_CONSTANT_LONG(frame))

            // GC
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in closure_long\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

    INS_GET_UPVALUE: {
            uint8_t slot = READ_BYTE(frame);
            ObjUpvalue *upvalue = ((ObjClosure *) frame->function)->upvalues[slot];
            if (upvalue->is_closed) {
                push(vm, upvalue->closed);
            } else {
                push(vm, vm->stack->data[upvalue->location]);
            }

            continue;
        }

    INS_SET_UPVALUE: {
            uint8_t slot = READ_BYTE(frame);
            // set upvalue's location to the last index of vm's value stack
            ObjUpvalue *upvalue = ((ObjClosure *) frame->function)->upvalues[slot];
            if (upvalue->is_closed) {
                upvalue->closed = peek_last(vm);
            } else {
                vm->stack->data[upvalue->location] = peek_last(vm);
            }

            continue;
        }

    INS_CLOSE_UPVALUE: {
            close_upvalue_from(vm, vm->stack->size - 1);
            pop(vm);

            continue;
        }

    INS_RETURN: {
            // hold the returning result
            Value result = pop(vm);
            // if this is the last call frame, finish the entire program
            close_upvalue_from(vm, frame->frame_stack);
            vm->frame_count--;
            if (vm->frame_count == 0) {
                // pop the top-level script function
                // pop(vm);

                // todo keep the global frame for REPL
                vm->frame_count++;
                return INTERPRET_OK;
            }

            // discard all the frame's stack
            vm->stack->size = frame->frame_stack;
            // push the result
            push(vm, result);
            // update frame
            frame = &vm->frames[vm->frame_count - 1];

            continue;
        }

    INS_IMPORT: {
            // TODO MUST be 1 so far. This flag indicates if the import module has a "alias".
            //  I think it's necessary to avoid name collision so the parser checks the alias
            //  in the syntax level. However, the compiler just emit this flag as it appears
            //  for the future features.

            uint8_t i = READ_BYTE(frame);
            // stack: [path][alias]
            ObjString *alias = AS_STRING(pop(vm));
            ObjString *path = AS_STRING(pop(vm));
            char module_abs_path[PATH_MAX];
            resolve_module_abs_path(vm, path->string, module_abs_path);
#ifdef DEBUG_TRACE_EXECUTION
      printf("resolve module [%s] get: [%s]\n", path->string, module_abs_path);
#endif
            if (strlen(module_abs_path) == 0) {
                runtime_error(vm, "Can't want lib from '%s'. Lib path not found.", path->string);
                return INTERPRET_RUNTIME_ERROR;
            }
            FILE *file = fopen(module_abs_path, "rb");
            if (file == NULL) {
                runtime_error(vm, "Can't want lib from '%s'. Read content failed.", path->string);
                return INTERPRET_RUNTIME_ERROR;
            }

            fseek(file, 0L, SEEK_END);
            size_t file_size = ftell(file);
            rewind(file);

            char content[file_size + 1];
            size_t read_len = fread(content, sizeof(char), file_size, file);
            if (read_len < file_size) {
                runtime_error(vm, "Can't resolve lib '%s'.", path->string);
                return INTERPRET_RUNTIME_ERROR;
            }
            content[read_len] = '\0';
            fclose(file);
#ifdef DEBUG_TRACE_EXECUTION
      printf("\n---------------\n");
      printf("lib: [%s] content:\n+++++++++\n%s\n", module_abs_path, content);
      printf("---------------\n\n");
#endif
            ObjString path_string =
            {
                .obj = {.type = OBJ_STRING, .is_marked = false},
                .string = module_abs_path,
                .length = strlen(module_abs_path),
                .hash = fnv1a_hash(module_abs_path, strlen(module_abs_path))
            };

            // MUST use contains first before get from table because the value could be NULL
            // with existing path. If the lib was not correctly resolved, the value is NULL.
            bool visited = Modulecontains_in_hash_table(lib_path_visited, &path_string);

            if (visited) {
                // check if this module is on the current path.
                if (String_contains_in_hash_table(on_path, &path_string)) {
                    // if the lib path is on the current path, it must be circular dependency
                    // this will cause infinite recursion. so I think the best solution is to
                    // abort the vm.
                    // TODO print the circular path
                    print_error(vm, "Circular dependency for lib '%s.", path->string);
                    exit(70);
                }
                // get module from cache

                ObjModule *cached_module = Moduleget_hash_table(lib_path_visited, &path_string);
#ifdef DEBUG_TRACE_EXECUTION
        printf("resolve cached module:[%s]\n", cached_module->lib->string);
#endif
                push(vm, OBJ_VAL(cached_module));
                continue;
            }

            VM *sub_vm = malloc(sizeof(VM));

            CompileContext current_context = vm->compile_context;
            CompileContext sub_context;
            new_sub_compile_context(&current_context, &sub_context);

            init_VM(sub_vm, vm, &sub_context, false);
#ifdef DEBUG_TRACE_EXECUTION
      printf("interpret module: %s\n", module_abs_path);
#endif
            InterpretResult result = interpret(sub_vm, module_abs_path, content);

            if (result != INTERPRET_OK) {
                runtime_error(vm, "Want lib ['%s']. error.", path->string);
                return INTERPRET_RUNTIME_ERROR;
            }

            ObjModule *module = register_module(sub_vm, path);
            push(vm, OBJ_VAL(module));

            // cache module
            Moduleput_hash_table(lib_path_visited, &path_string, module);

            RTObjput_hash_table(vm->objects, (RTObjHashtableKey) module, sizeof(ObjModule));
            vm->gc_info->bytes_allocated += sizeof(ObjModule);

            // merge runtime objects
            merge_rt_obj(vm, sub_vm);

            // free subcontext global names
            String_free_hash_table(sub_context.global_names);
            // free sub_vm
            free_vm_shallow(sub_vm);
            free(sub_vm);
            continue;
        }

    INS_ARRAY: {
            size_t size = READ_3BYTES(frame);
            ObjArray *array = peek_create(vm, size);
            pop_batch(vm, size);
            push(vm, OBJ_VAL(array));
            continue;
        }

    INS_INDEXING: {
            Value index = pop(vm);
            Value object = pop(vm);
            if (!IS_NUMBER(index)) {
                runtime_error(vm, "index must be number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            if (!IS_ARRAY(object)) {
                runtime_error(vm, "Can't get index of non-array object.");
                return INTERPRET_RUNTIME_ERROR;
            }
            if (!is_double_an_int(AS_NUMBER(index))) {
                runtime_error(vm, "Index must be integer.");
                return INTERPRET_RUNTIME_ERROR;
            }
            int i = AS_NUMBER(index);
            ObjArray *arr = AS_ARRAY(object);

            if (i < 0) {
                i = arr->array->size + i;
            }
            if (i < 0 || i > arr->array->size - 1) {
                runtime_error(vm, "index out of range.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(vm, Valueget_data_arraylist(arr->array, i));
            continue;
        }

    INS_ELEMENT_ASSIGN: {
            Value right = pop(vm);
            Value index = pop(vm);
            Value object = pop(vm);

            CHECK_INDEX(index)

            if (!IS_ARRAY(object)) {
                runtime_error(vm, "Can't get index of non-array object.");
                return INTERPRET_RUNTIME_ERROR;
            }
            int i = AS_NUMBER(index);
            ObjArray *arr = AS_ARRAY(object);

            if (i < 0) {
                i = arr->array->size + i;
            }
            CHECK_INDEX_RANGE(i, arr->array->size)

            Valueset_arraylist_data(arr->array, i, right);
            push(vm, right);
            continue;
        }

    INS_SLICING: {
            uint8_t index0 = READ_BYTE(frame);
            uint8_t index1 = READ_BYTE(frame);
            int i0;
            int i1;
            if (index1) {
                Value index = pop(vm);
                CHECK_INDEX(index)
                i1 = AS_NUMBER(index);
            }
            if (index0) {
                Value index = pop(vm);
                CHECK_INDEX(index)
                i0 = AS_NUMBER(index);
            }
            Value object = pop(vm);
            if (!IS_ARRAY(object)) {
                runtime_error(vm, "Can't get slice of non-array object.");
                return INTERPRET_RUNTIME_ERROR;
            }
            ObjArray *arr = AS_ARRAY(object);
            ValueArrayList *sub_arr = sublist(arr->array, index0, i0, index1, i1);
            ObjArray *slice = new_array(sub_arr);
            RTObjput_hash_table(vm->objects, (RTObjHashtableKey) slice, sizeof(ObjArray));
            vm->gc_info->bytes_allocated += sizeof(ObjArray);
            push(vm, OBJ_VAL(slice));

            // GC
            if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
        printf("***gc in slice\n");
#endif
                collect_garbage(vm->gc_info->gc, vm);
            }
            continue;
        }

#else
    uint8_t op = READ_BYTE(frame);
    switch (op) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT(frame);
        push(vm, constant);
        break;
      }
      case OP_CONSTANT_LONG: {
        Value constant = READ_CONSTANT_LONG(frame);
        push(vm, constant);
        break;
      }
      case OP_CLASS: {
        Value klass = READ_CONSTANT(frame);
        push(vm, klass);
        break;
      }
      case OP_CLASS_LONG: {
        Value klass = READ_CONSTANT_LONG(frame);
        push(vm, klass);
        break;
      }
      case OP_INHERIT: {
        // [class][superclass]
        Value v = pop(vm);
        if (!IS_CLASS(v)) {
          runtime_error(vm, "Can only inherit from class.");
          return INTERPRET_RUNTIME_ERROR;
        }
        ObjClass *superclass = AS_CLASS(v);
        ObjClass *subclass = AS_CLASS(peek_last(vm));
        // copy method from superclass
        COPY_METHODS(superclass->methods, subclass->methods)
        // copy static methods
        COPY_METHODS(superclass->static_methods, subclass->static_methods)
        break;
      }
      case OP_NEGATE: {
        Value v = peek_last(vm);
        if (IS_NUMBER(v)) {
          set_last(vm, NUMBER_VAL(-AS_NUMBER(v)));
        } else {
          // runtime error
          runtime_error(vm, "Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_ADD: {
        if (IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
          Value b = pop(vm);
          Value a = pop(vm);
          ObjString *new_str = string_concatenate(vm, AS_STRING(a), AS_STRING(b));
          push(vm, OBJ_VAL(new_str));
        } else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
          Value b = pop(vm);
          Value a = pop(vm);
          push(vm, NUMBER_VAL(AS_NUMBER(a) + AS_NUMBER(b)));
        } else {
          //TODO
          Value b = pop(vm);
          Value a = pop(vm);
          ObjString *sa = value_to_string(vm, a);
          if (sa == NULL) return INTERPRET_RUNTIME_ERROR;
          ObjString *sb = value_to_string(vm, b);
          if (sb == NULL) return INTERPRET_RUNTIME_ERROR;
          ObjString *new_str = string_concatenate(vm, sa, sb);
          push(vm, OBJ_VAL(new_str));
        }
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in add\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_SUBTRACT: {
        BINARY_OP(vm, NUMBER_VAL, -);
        break;
      }
      case OP_MULTIPLY: {
        BINARY_OP(vm, NUMBER_VAL, *);
        break;
      }
      case OP_DIVIDE: {
        BINARY_OP(vm, NUMBER_VAL, /);
        break;
      }
      case OP_EXPONENT: {
        EXPONENTIAL(vm, NUMBER_VAL, pow);
        break;
      }
      case OP_FACTORIAL: {
        FACTORIAL(vm, NUMBER_VAL, factorial);
        break;
      }
      case OP_NOT: {
        Value v = peek_last(vm);
        set_last(vm, BOOL_VAL(is_false(v)));
        break;
      }
      case OP_NIL: {
        push(vm, NIL_VAL);
        break;
      }
      case OP_TRUE: {
        push(vm, BOOL_VAL(true));
        break;
      }
      case OP_FALSE: {
        push(vm, BOOL_VAL(false));
        break;
      }
      case OP_EQUAL: {
        Value b = pop(vm);
        Value a = pop(vm);
        push(vm, BOOL_VAL(value_equals(a, b)));
        break;
      }
      case OP_GREATER: {
        BINARY_OP(vm, BOOL_VAL, >);
        break;
      }
      case OP_LESS: {
        BINARY_OP(vm, BOOL_VAL, <);
        break;
      }
      case OP_PRINT: {
        ObjString *sa = value_to_string(vm, pop(vm));
        if (sa == NULL) return INTERPRET_RUNTIME_ERROR;
        printf("%s", sa->string);
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("****gc in print\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_POP: {
        pop(vm);
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in pop\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_DEFINE_GLOBAL: {
        DEFINE_GLOBAL(READ_BYTE(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in define global\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_DEFINE_GLOBAL_LONG: {
        DEFINE_GLOBAL(READ_3BYTES(frame))

        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in define global long\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_GET_GLOBAL: {
        GET_GLOBAL(READ_BYTE(frame))
        break;
      }
      case OP_GET_GLOBAL_LONG: {
        GET_GLOBAL(READ_3BYTES(frame))
        break;
      }
      case OP_SET_GLOBAL: {
        SET_GLOBAL(READ_BYTE(frame))
        break;
      }
      case OP_SET_GLOBAL_LONG: {
        SET_GLOBAL(READ_3BYTES(frame))
        break;
      }
      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE(frame);
        vm->stack->data[frame->frame_stack + slot] = peek_last(vm);
        break;
      }
      case OP_SET_LOCAL_LONG: {
        size_t slot = READ_3BYTES(frame);
        vm->stack->data[frame->frame_stack + slot] = peek_last(vm);
        break;
      }
      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE(frame);
        // accesses the current frame’s slots array by accessing the given numbered slot relative to the
        // beginning of that frame.
        push(vm, vm->stack->data[frame->frame_stack + slot]);
        break;
      }
      case OP_GET_LOCAL_LONG: {
        size_t slot = READ_3BYTES(frame);
        push(vm, vm->stack->data[slot]);
        break;
      }
      case OP_GET_PROP: {
        GET_PROP(READ_CONSTANT(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in GET_PROP\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_GET_PROP_LONG: {
        GET_PROP(READ_CONSTANT_LONG(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in GET_PROP_LONG\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_GET_SUPER_PROP: {
        GET_SUPER_PROP(READ_CONSTANT(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in GET_SUPER_PROP\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_GET_SUPER_PROP_LONG: {
        // 'this', which is a subclass instance
        GET_SUPER_PROP(READ_CONSTANT_LONG(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in GET_SUPER_PROP_LONG\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_SET_PROP: {
        SET_PROP(READ_CONSTANT(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in set prop\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_SET_PROP_LONG: {
        SET_PROP(READ_CONSTANT_LONG(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in set prop long\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_METHOD_LOCAL: {
        METHOD_LOCAL(READ_BYTE(frame))
        break;
      }
      case OP_METHOD_LOCAL_LONG: {
        METHOD_LOCAL(READ_3BYTES(frame))
        break;
      }
      case OP_METHOD_GLOBAL: {
        METHOD_GLOBAL(READ_BYTE(frame))
        break;
      }
      case OP_METHOD_GLOBAL_LONG: {
        METHOD_GLOBAL(READ_3BYTES(frame))
        break;
      }
      case OP_JUMP_IF_FALSE: {
        size_t offset = READ_3BYTES(frame);
        size_t jump_size = is_false(peek_last(vm)) * offset;
        //vm->ip += jump_size;
        frame->ip += jump_size;
        break;
      }
      case OP_JUMP: {
        frame->ip += READ_3BYTES(frame);
        break;
      }
      case OP_JUMP_BACK: {
        frame->ip -= READ_3BYTES(frame);
        break;
      }
      case OP_MOD: {
        if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
          Value b = pop(vm);
          Value a = pop(vm);
          if (!(is_double_an_int(b.number) && is_double_an_int(a.number))) {
            runtime_error(vm, "Operands of '%' must be int.");
            return INTERPRET_RUNTIME_ERROR;
          }
          push(vm, NUMBER_VAL((int) a.number % (int) b.number));
        } else {
          runtime_error(vm, "Operands must be numbers.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_INVOKE: {
        INVOKE(READ_CONSTANT(frame))
        break;
      }
      case OP_INVOKE_LONG: {
        INVOKE(READ_CONSTANT_LONG(frame))
        break;
      }
      case OP_INVOKE_SUPER: {
        INVOKE_SUPER(READ_CONSTANT(frame))
        break;
      }
      case OP_INVOKE_SUPER_LONG: {
        INVOKE_SUPER(READ_CONSTANT_LONG(frame))
        break;
      }
      case OP_CALL: {
        int arg_count = READ_BYTE(frame);
        if (!call_value(vm, peek(vm, arg_count), arg_count)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        frame = &vm->frames[vm->frame_count - 1];
        // GC
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in call\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_CLOSURE: {
        CLOSURE(READ_CONSTANT(frame))
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in closure\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_CLOSURE_LONG: {
        CLOSURE(READ_CONSTANT_LONG(frame))
        // GC
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in closure_long\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE(frame);
        ObjUpvalue *upvalue = ((ObjClosure *) frame->function)->upvalues[slot];
        if (upvalue->is_closed) {
          push(vm, upvalue->closed);
        } else {
          push(vm, vm->stack->data[upvalue->location]);
        }
        break;
      }
      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE(frame);
        // set upvalue's location to the last index of vm's value stack
        ObjUpvalue *upvalue = ((ObjClosure *) frame->function)->upvalues[slot];
        if (upvalue->is_closed) {
          upvalue->closed = peek_last(vm);
        } else {
          vm->stack->data[upvalue->location] = peek_last(vm);
        }
        break;
      }
      case OP_CLOSE_UPVALUE: {
        close_upvalue_from(vm, vm->stack->size - 1);
        pop(vm);
        break;
      }
      case OP_RETURN: {
        // hold the returning result
        Value result = pop(vm);
        // if this is the last call frame, finish the entire program
        close_upvalue_from(vm, frame->frame_stack);
        vm->frame_count--;
        if (vm->frame_count == 0) {
          // pop the top-level script function
          // pop(vm);

          // todo keep the global frame for REPL
          vm->frame_count++;
          return INTERPRET_OK;
        }

        // discard all the frame's stack
        vm->stack->size = frame->frame_stack;
        // push the result
        push(vm, result);
        // update frame
        frame = &vm->frames[vm->frame_count - 1];
        break;
      }
      case OP_IMPORT: {
        // must be 1 so far
        uint8_t i = READ_BYTE(frame);
        // stack: [path][alias]
        ObjString *alias = AS_STRING(pop(vm));
        ObjString *path = AS_STRING(pop(vm));

        char *p = resolve_import_lib_path(vm, path->string);
        FILE *file = fopen(p, "rb");

        if (file == NULL) {
          runtime_error(vm, "Can't want lib from '%s'.", path->string);
          return INTERPRET_RUNTIME_ERROR;
        }

        fseek(file, 0L, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        char content[file_size + 1];
        size_t read_len = fread(content, sizeof(char), file_size, file);
        if (read_len < file_size) {
          runtime_error(vm, "Can't resolve lib '%s'.", path->string);
          return INTERPRET_RUNTIME_ERROR;
        }
        content[read_len] = '\0';
        fclose(file);
#ifdef DEBUG_TRACE_EXECUTION
        printf("\n---------------\n");
      printf("lib: [%s] content:\n+++++++++\n%s\n", p, content);
      printf("---------------\n\n");
#endif
        VM sub_vm;
        CompileContext current_context = vm->compile_context;
        CompileContext sub_context;
        new_sub_compile_context(&current_context, &sub_context);

        init_VM(&sub_vm, vm, &sub_context, false);
        InterpretResult result = interpret(&sub_vm, content);

        if (result != INTERPRET_OK) {
          runtime_error(vm, "Want lib ['%s']. error.", path->string);
          return INTERPRET_RUNTIME_ERROR;
        }

        // TODO register module
        ObjModule *module = register_module(&sub_vm, path);
        push(vm, OBJ_VAL(module));

        RTObjput_hash_table(vm->objects, (RTObjHashtableKey) module, sizeof(ObjModule));
        vm->gc_info->bytes_allocated += sizeof(ObjModule);

        // merge runtime objects
        merge_rt_obj(vm, &sub_vm);

        // free subcontext global names
        String_free_hash_table(sub_context.global_names);
        // free sub_vm
        free_vm_shallow(&sub_vm);
        break;
      }

      case OP_ARRAY: {
        size_t size = READ_3BYTES(frame);
        ObjArray *array = peek_create(vm, size);
        pop_batch(vm, size);
        push(vm, OBJ_VAL(array));
        break;
      }

      case OP_INDEXING: {
        Value index = pop(vm);
        Value object = pop(vm);
        if (!IS_NUMBER(index)) {
          runtime_error(vm, "index must be number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        if (!IS_ARRAY(object)) {
          runtime_error(vm, "Can't get index of non-array object.");
          return INTERPRET_RUNTIME_ERROR;
        }
        if (!is_double_an_int(AS_NUMBER(index))) {
          runtime_error(vm, "Index must be integer.");
          return INTERPRET_RUNTIME_ERROR;
        }
        int i = AS_NUMBER(index);
        ObjArray *arr = AS_ARRAY(object);

        if (i < 0) {
          i = arr->array->size + i;
        }
        if (i < 0 || i > arr->array->size - 1) {
          runtime_error(vm, "index out of range.");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(vm, Valueget_data_arraylist(arr->array, i));
        break;
      }

      case OP_ELEMENT_ASSIGN: {
        Value right = pop(vm);
        Value index = pop(vm);
        Value object = pop(vm);

        CHECK_INDEX(index)

        if (!IS_ARRAY(object)) {
          runtime_error(vm, "Can't get index of non-array object.");
          return INTERPRET_RUNTIME_ERROR;
        }
        int i = AS_NUMBER(index);
        ObjArray *arr = AS_ARRAY(object);

        if (i < 0) {
          i = arr->array->size + i;
        }
        CHECK_INDEX_RANGE(i, arr->array->size)

        Valueset_arraylist_data(arr->array, i, right);
        push(vm, right);
        break;
      }

      case OP_SLICING: {
        uint8_t index0 = READ_BYTE(frame);
        uint8_t index1 = READ_BYTE(frame);
        int i0;
        int i1;
        if (index1) {
          Value index = pop(vm);
          CHECK_INDEX(index)
          i1 = AS_NUMBER(index);
        }
        if (index0) {
          Value index = pop(vm);
          CHECK_INDEX(index)
          i0 = AS_NUMBER(index);
        }
        Value object = pop(vm);
        if (!IS_ARRAY(object)) {
          runtime_error(vm, "Can't get slice of non-array object.");
          return INTERPRET_RUNTIME_ERROR;
        }
        ObjArray *arr = AS_ARRAY(object);
        ValueArrayList *sub_arr = sublist(arr->array, index0, i0, index1, i1);
        ObjArray *slice = new_array(sub_arr);
        RTObjput_hash_table(vm->objects, (RTObjHashtableKey) slice, sizeof(ObjArray));
        vm->gc_info->bytes_allocated += sizeof(ObjArray);
        push(vm, OBJ_VAL(slice));

        // GC
        if (should_trigger_gc(vm)) {
#ifdef DEBUG_GC_LOG
          printf("***gc in slice\n");
#endif
          collect_garbage(vm->gc_info->gc, vm);
        }
        break;
      }
      default: {
        runtime_error(vm, "Unsupported operation code.");
        return INTERPRET_RUNTIME_ERROR;
      }
    }
#endif
    }
}

// Function to find the next power of 2 greater than or equal to the given number
static unsigned int next_power_of_2(unsigned int n) {
    // If n is already a power of 2, return n
    if (n && !(n & (n - 1))) {
        return n;
    }

    // Initialize the result as 1
    unsigned int power = 1;

    // Left shift the result until it is greater than or equal to n
    while (power < n) {
        power <<= 1;
    }

    return power;
}

static SliceResult get_slice_indices(bool has_start, int start_index, bool has_end, int end_index, int arr_size) {
    SliceResult result;
    result.reverse = false;

    // Determine the actual start index
    if (!has_start) {
        result.actual_start = 0; // If start index is not provided, default to 0
    } else if (start_index < 0) {
        result.actual_start = arr_size + start_index; // If start index is negative
        if (result.actual_start < 0) result.actual_start = 0; // Ensure it does not go below 0
    } else {
        result.actual_start = start_index;
    }

    // Determine the actual end index
    if (!has_end) {
        result.actual_end = arr_size; // If end index is not provided, default to size of array
    } else if (end_index < 0) {
        result.actual_end = arr_size + end_index; // If end index is negative
        if (result.actual_end < 0) result.actual_end = 0; // Ensure it does not go below 0
    } else {
        result.actual_end = end_index;
    }

    // Check if we need to reverse the array
    if (start_index < 0 && !has_end) {
        result.reverse = true;
    }

    return result;
}

static ValueArrayList *sublist(ValueArrayList *list, bool has_start, int start_index, bool has_end, int end_index) {
    SliceResult slice = get_slice_indices(has_start, start_index, has_end, end_index, list->size);
    int actual_start = slice.actual_start;
    int actual_end = slice.actual_end;
    bool reverse = slice.reverse;

    // Check if the range is valid
    if (actual_start >= actual_end || actual_start >= list->size) {
        return Valuenew_arraylist(1);
    }
    if (actual_end > list->size) {
        actual_end = list->size;
    }
    int size = actual_end - actual_start;
    unsigned int cap = next_power_of_2(size);
    ValueArrayList *slice_list = Valuenew_arraylist(cap);
    if (reverse) {
        for (int i = 0; i < size; i++) {
            slice_list->data[i] = list->data[actual_end - 1 - i];
        }
    } else {
        memcpy(slice_list->data, list->data + actual_start, size * sizeof(Value));
    }
    slice_list->size = size;
    return slice_list;
}

static void free_vm_shallow(VM *sub_vm) {
    free_function((ObjFunction *) sub_vm->frames[0].function);
    Valuefree_arraylist(sub_vm->stack);
    // free runtime objects
    RTObjfree_hash_table(sub_vm->objects);
    free_GC(sub_vm->gc_info->gc);
    free(sub_vm->gc_info);
}

static ObjArray *peek_create(VM *vm, size_t size) {
    ObjArray *array = new_array(NULL);
    for (size_t i = 0; i < size; ++i) {
        Valueappend_arraylist(array->array, peek(vm, size - i - 1));
    }

    RTObjput_hash_table(vm->objects, (RTObjHashtableKey) array, sizeof(ObjArray));
    vm->gc_info->bytes_allocated += sizeof(ObjArray);
    return array;
}

static void pop_batch(VM *vm, size_t size) {
    vm->stack->size -= size;
}

static void merge_rt_obj(VM *vm, VM *sub_vm) {
    RTObjHashtableIterator *it = RTObjhashtable_iterator(sub_vm->objects);
    while (RTObjhashtable_iter_has_next(it)) {
        RTObjKVEntry *entry = RTObjhashtable_next_entry(it);
        Obj *obj = RTObjtable_entry_key(entry);
        RTObjput_hash_table(vm->objects, obj, RTObjtable_entry_value(entry));
    }
    RTObjfree_hashtable_iter(it);
}

/**
 *
 * @param from
 * @param to
 */
static void new_sub_compile_context(CompileContext *main, CompileContext *sub) {
    sub->string_intern = main->string_intern;
    sub->objs = main->objs;
    sub->global_values = main->global_values;
    sub->class_objs = main->class_objs;

    // new global names for new scope, need to keep native functions same
    sub->global_names = String_new_hash_table(obj_string_hash, obj_string_equals);
    String_HashtableIterator *s = String_hashtable_iterator(main->global_names);
    while (String_hashtable_iter_has_next(s)) {
        String_KVEntry *e = String_hashtable_next_entry(s);
        ObjString *key = String_table_entry_key(e);
        bool is_native_name = false;
        for (int i = 0; NATIVE_FUNC[i] != NULL; ++i) {
            NativeFunctionDecl *n = NATIVE_FUNC[i];
            if (strcmp(key->string, n->name) == 0) {
                is_native_name = true;
                break;
            }
        }
        if (!is_native_name) {
            for (int i = 0; NATIVE_VAL[i] != NULL; ++i) {
                NativeValueDecl *n = NATIVE_VAL[i];
                if (strcmp(key->string, n->name) == 0) {
                    is_native_name = true;
                    break;
                }
            }
        }

        if (is_native_name) {
            int value = String_table_entry_value(e);
            String_put_hash_table(sub->global_names, key, value);
        }
    }
    String_free_hashtable_iter(s);
}

/**
 * VM's runtime objects are created from:
 *   1. Call of class's initializer. -> create an ObjectInstance* object
 *   2. Capture upvalue. -> create an ObjUpvalue* object
 *   3. Get property (methods) of instance or super. -> create an InstanceMethod* object
 *   4. Closure object from function. -> create an ObjClosure* object, also capture upvalues in 2
 *   5. String object. -> create an ObjString* object and add to string intern hashtable
 * All the runtime objects are put to VM's objects hashtable where key is object and value is its size
 *
 * Runtime objects are stored in VM. The module only expose global values, some of these values are referred to
 * runtime objects. Global values are stored in VM's global value table, so we only need to collect those values.
 * There's no need to free runtime objects after compile and interpret modules because some objects are referred by
 * global values and will cause dangling pointer if freed.
 *
 *
 * @param vm
 */
static ObjModule *register_module(VM *vm, ObjString *lib) {
    ObjModule *module = new_module(lib);
#ifdef DEBUG_TRACE_EXECUTION
  printf("--- register module from [%s] ----\n", lib->string);
#endif
    // copy global values
    String_HashtableIterator *it = String_hashtable_iterator(vm->compile_context.global_names);
    while (String_hashtable_iter_has_next(it)) {
        String_KVEntry *e = String_hashtable_next_entry(it);
        ObjString *key = String_table_entry_key(e);
        int index = String_table_entry_value(e);
#ifdef DEBUG_TRACE_EXECUTION
    printf("-(%d) [%s]: ", index, key->string);
    print_value(vm->compile_context.global_values->data[index]);
    printf("\n");
#endif
        Value value = vm->compile_context.global_values->data[index];
        if (IS_NATIVE(value)) continue;
        if (IS_MODULE(value)) {
            if (module->next == NULL) {
                module->next = AS_MODULE(value);
            } else {
                ObjModule *m = AS_MODULE(value);
                m->next = module->next;
                module->next = m;
            }
        }
#ifdef DEBUG_TRACE_EXECUTION
    printf("    register [%s] : ", key->string);
    print_value(value);
    printf("\n");
#endif
        String_put_hash_table(module->imports, key, index);
    }
    String_free_hashtable_iter(it);
#ifdef DEBUG_TRACE_EXECUTION
  int i = vm->compile_context.global_values->size;
  for (int j = 0; j < i; ++j) {
    printf("global[%d] ", j);
    print_value(vm->compile_context.global_values->data[j]);
    printf("\n");
  }
#endif
    return module;
}

static void push(VM *vm, Value value) {
    //  add_value_array(&vm->stack, value);
    Valueappend_arraylist(vm->stack, value);
}

static Value pop(VM *vm) {
    Value v = Valueget_data_arraylist(vm->stack, vm->stack->size - 1);
    Valueremove_arraylist(vm->stack, vm->stack->size - 1);
    return v;
    //return vm->stack.values[vm->stack.count-- - 1];
}

static void set(VM *vm, size_t index, Value value) {
    //  vm->stack.values[index] = value;
    Valueset_arraylist_data(vm->stack, index, value);
}

static Value peek_last(VM *vm) {
    return peek(vm, 0);
}

static Value peek(VM *vm, size_t distance) {
    return Valueget_data_arraylist(vm->stack, vm->stack->size - 1 - distance);
    //  return vm->stack.values[vm->stack.count - 1 - distance];
}

static void set_last(VM *vm, Value value) {
    set(vm, vm->stack->size - 1, value);
}

static void debug_print_stack(ValueArrayList *value_array) {
    printf(" stack:        ");
    for (size_t i = 0; i < value_array->size; i++) {
        printf("[");
        print_value(value_array->data[i]);
        printf("]");
    }
    printf("\n");
}

static void debug_print_string_table(VM *v) {
    printf("          \n");
    printf("size of string string_intern: %zu\n",
           String_size_of_hash_table((String_Hashtable *) v->compile_context.string_intern));
    String_HashtableIterator *i = String_hashtable_iterator((String_Hashtable *) v->compile_context.string_intern);
    while (String_hashtable_iter_has_next(i)) {
        String_KVEntry *e = String_hashtable_next_entry(i);
        printf("    [strobj: %p, %s]\n", e->key, e->key->string);
        fflush(stdout);
    }
    String_free_hashtable_iter(i);
    printf("\n");
    fflush(stdout);
}

static void runtime_error(VM *vm, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
#ifdef WASM_LOG
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
#endif
    va_end(args);
    fputs("\n", stderr);
#ifdef WASM_LOG
    // 添加前缀到最终消息
    char final_message[550];
    snprintf(final_message, sizeof(final_message), "[status][error]-Runtime ERROR. ZHI VM: %s", buffer);
    EM_ASM_({console.warn(UTF8ToString($0));}, final_message);
#endif
    for (size_t i = vm->frame_count; i > 0; --i) {
        CallFrame *frame = &vm->frames[i - 1];
        ObjFunction *function = get_frame_function(frame);
        size_t offset = frame->ip - function->chunk->code->data - 1;
        size_t line = get_line(function->chunk, offset);

        if (vm->source_path != NULL) {
            fprintf(stderr, "[%s] ", vm->source_path);
#ifdef WASM_LOG
            char buffer2[512];
            sprintf(buffer2, "[status][error]-Runtime ERROR. ZHI VM: [%s] ", vm->source_path);
#endif
        }
        fprintf(stderr, "[line %zu] in %s()\n", line,
                function->name == NULL ? "script" : function->name->string);

#ifdef WASM_LOG
        char buffer2[512];
        sprintf(buffer2, "[status][error]-ZHI VM: [line %zu] in %s()", line,
            function->name == NULL ? "main_script" : function->name->string);
        EM_ASM_({console.warn(UTF8ToString($0));}, buffer2);
#endif
    }

    reset_stack(vm);
}

static void print_error(VM *vm, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
#ifdef WASM_LOG
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
#endif

    va_end(args);
    fputs("\n", stderr);
#ifdef WASM_LOG
    // 添加前缀到最终消息
    char final_message[550];
    snprintf(final_message, sizeof(final_message), "[status][error]-Runtime ERROR. ZHI VM: %s", buffer);
    EM_ASM_({console.warn(UTF8ToString($0));}, final_message);
#endif


    for (size_t i = vm->frame_count; i > 0; --i) {
        CallFrame *frame = &vm->frames[i - 1];
        ObjFunction *function = get_frame_function(frame);
        size_t offset = frame->ip - function->chunk->code->data - 1;
        size_t line = get_line(function->chunk, offset);
        fprintf(stderr, "[line %zu] in %s()\n", line,
                function->name == NULL ? "script" : function->name->string);
#ifdef WASM_LOG
        char buffer2[512];
        sprintf(buffer2, "[status][error]-Runtime ERROR. ZHI VM: [line %zu] in %s()", line,
            function->name == NULL ? "main_script" : function->name->string);
        EM_ASM_({console.warn(UTF8ToString($0));}, buffer2);
#endif
    }
}

static void reset_stack(VM *vm) {
    // reset count as value array is struct
    Valueclear_arraylist(vm->stack);
    //  vm->stack.count = 0;
    vm->frame_count = 0;
    vm->open_upvalues = NULL;
    collect_garbage(vm->gc_info->gc, vm);
}

static ObjString *value_to_string(VM *vm, Value value) {
    if (IS_STRING(value)) {
        return AS_STRING(value);
    }
    if (IS_OBJ(value)) {
        OBJ_TOSTRING(value.obj)
        if (len == 0) {
            runtime_error(vm, "Value can't be stringify");
            return NULL;
        }
        uint32_t hash = fnv1a_hash(string_, len);
        return get_or_create_string(vm, string_, hash, len);
    }
    char *string = NULL;
    int size;
    char str[32];
    if (IS_NUMBER(value)) {
        double v = AS_NUMBER(value);
        size = sprintf(str, "%.17g", v);
        str[size] = '\0';
        string = &str[0];
        //    size = strlen(str);
    } else if (IS_NIL(value)) {
        size = 3;
        string = "nil";
    } else if (IS_BOOL(value)) {
        bool v = AS_BOOL(value);
        string = v ? "aow" : "emm";
        size = 3;
    }

    uint32_t hash = fnv1a_hash(string, size);
    return get_or_create_string(vm, string, hash, size);
}

static ObjString *path_to_string(const char *path) {
    size_t len = strlen(path);
    uint32_t hash = fnv1a_hash(path, len);
    char *str = malloc(len + 1);
    memcpy(str, path, len);
    str[len] = '\0';
    return new_string_obj(str, len, hash);
}

static int is_double_an_int(double value) {
    return value == floor(value);
}

static bool should_trigger_gc(VM *vm) {
    if (vm->gc_info->bytes_allocated > vm->gc_info->next_GC) {
        return true;
    }
    if (vm->gc_info->gc_call >= GC_CALL_TRIGGER) {
#ifdef DEBUG_GC_LOG
    printf("trigger gc by __GC call");
#endif
        return true;
    }
    return false;
}

void deallocate_rt_obj(GCInfo *gc_info, size_t size) {
    if (gc_info->bytes_allocated < size) {
        gc_info->bytes_allocated = 0;
        return;
    }
    gc_info->bytes_allocated -= size;
}

void update_next_gc_size(GCInfo *gc_info) {
#ifdef DEBUG_GC_LOG
  printf("++update next gc size\n");
#endif
    // reset gc call
    gc_info->gc_call = 0;
    if (gc_info->bytes_allocated == 0) {
        gc_info->next_GC = INITIAL_GC_SIZE;
    } else {
        size_t next_size = gc_info->bytes_allocated * GC_GROW_FACTOR;
        if (next_size < INITIAL_GC_SIZE) {
            gc_info->next_GC = INITIAL_GC_SIZE;
        } else if (next_size > GC_MAX_SIZE) {
            gc_info->next_GC = GC_MAX_SIZE;
        } else {
            gc_info->next_GC = next_size;
        }
    }
#ifdef DEBUG_GC_LOG
  printf("   ********* next gc size: [%zu] bytes **********\n", gc_info->next_GC);
  printf("--update next gc size\n");
#endif
}

/**  +++++++++++NATIVE FUNCTIONS+++++++++++++++++ */

Value native_func_clock(int arg_count, Value *args, void *vm) {
    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

Value native_gc(int arg_count, Value *args, void *vm) {
    // always return true
    VM *v = vm;
    v->gc_info->gc_call++;
    return BOOL_VAL(true);
}

Value native_has_field(int arg_count, Value *args, void *vm) {
    Value arg0 = args[0];
    if (!IS_INSTANCE(arg0)) {
        return BOOL_VAL(false);
    }
    ObjInstance *instance = AS_INSTANCE(arg0);
    Value arg1 = args[1];
    if (!IS_STRING(arg1)) {
        return BOOL_VAL(false);
    }
    ObjString *name = AS_STRING(arg1);
    int ret = Valuecontains_in_hash_table(instance->fields, name);
    return BOOL_VAL(ret == 1);
}

Value native_has_method(int arg_count, Value *args, void *vm) {
    Value arg0 = args[0];
    ObjClass *klass = NULL;
    if (IS_INSTANCE(arg0)) {
        klass = AS_INSTANCE(arg0)->klass;
    } else if (IS_CLASS(arg0)) {
        klass = AS_CLASS(arg0);
    } else {
        return BOOL_VAL(false);
    }
    Value arg1 = args[1];
    if (!IS_STRING(arg1)) {
        return BOOL_VAL(false);
    }
    ObjString *name = AS_STRING(arg1);
    int ret = Valuecontains_in_hash_table(klass->methods, name);
    if (ret == 1) {
        return BOOL_VAL(true);
    }
    ret = Valuecontains_in_hash_table(klass->static_methods, name);
    return BOOL_VAL(ret == 1);
}

Value native_del_field(int arg_count, Value *args, void *vm) {
    Value arg0 = args[0];
    if (!IS_INSTANCE(arg0)) {
        return BOOL_VAL(false);
    }
    ObjInstance *instance = AS_INSTANCE(arg0);
    Value arg1 = args[1];
    if (!IS_STRING(arg1)) {
        return BOOL_VAL(false);
    }
    ObjString *name = AS_STRING(arg1);

    int flag;
    Valueremove_with_flag_hash_table(instance->fields, name, &flag);
    return BOOL_VAL(flag == 1);
}

Value native_len(int arg_count, Value *args, void *vm) {
    // TODO should throw exception if value is not countable
    // return 0 for now if value is not countable
    Value arg0 = args[0];
    if (!IS_ARRAY(arg0)) {
        //    char msg[200];
        //    sprintf(msg, "TypeError: Object type [%d] has no length.", IS_OBJ(arg0) ? arg0.type + VAL_OBJ : arg0.type);
        print_error(vm, "Object type [%d] has no length.", IS_OBJ(arg0) ? arg0.type + VAL_OBJ : arg0.type);
        cx_throw(100);
    }
    return NUMBER_VAL(AS_ARRAY(arg0)->array->size);
}

Value native_type(int arg_count, Value *args, void *vm) {
    Value arg0 = args[0];
    if (IS_OBJ(arg0)) {
        return NUMBER_VAL(arg0.type + AS_OBJ(arg0)->type);
    }
    return NUMBER_VAL(arg0.type);
}

Value native_stringify(int arg_count, Value *args, void *v) {
    VM *vm = v;
    Value arg0 = args[0];
    if (IS_ARRAY(arg0)) {
        ObjArray *arr = AS_ARRAY(arg0);
        if (arr->array->size == 0) {
            char *s = "[]";
            uint32_t hash = fnv1a_hash(s, 2);
            return OBJ_VAL(get_or_create_string(vm, s, hash, 2));
        }
        ObjString *strarr[arr->array->size];

        size_t arr_str_size = 0;
        for (size_t i = 0; i < arr->array->size; i++) {
            Value e = arr->array->data[i];
            ObjString *sa = value_to_string(vm, e);
            if (sa == NULL) {
                cx_throw(100);
            }
            strarr[i] = sa;
            arr_str_size += sa->length;
        }
        // + n-1 ',' and '[',']' and a \0
        char ret[arr_str_size + arr->array->size - 1 + 2 + 1];
        ret[0] = '[';
        size_t loc = 1;
        for (size_t i = 0; i < arr->array->size; i++) {
            memcpy(ret + loc, strarr[i]->string, strarr[i]->length);
            loc += strarr[i]->length;
            if (i != arr->array->size - 1) {
                ret[loc++] = ',';
            }
        }
        ret[loc++] = ']';
        ret[loc + 1] = '\0';
        uint32_t hash = fnv1a_hash(ret, loc);
        return OBJ_VAL(get_or_create_string(vm, ret, hash, loc));
    }
    ObjString *sa = value_to_string(vm, arg0);
    if (sa == NULL) {
        cx_throw(100);
    }
    return OBJ_VAL(sa);
}

Value native_puffln(int arg_count, Value *args, void *v) {
    native_puff(arg_count, args, v);
    printf("\n");
    return NIL_VAL;
}

Value native_puff(int arg_count, Value *args, void *v) {
    Value str = native_stringify(arg_count, args, v);
    printf("%s",AS_STRING(str)->string);
    return NIL_VAL;
}
