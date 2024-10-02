//
// Created by ran on 24-3-15.
//

#ifndef ZHI_VM_VM_H_
#define ZHI_VM_VM_H_

#include "common/common.h"
#include "compiler/compiler.h"
#include "list/linked_list.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef char InterpretResult;
#define INTERPRET_OK (InterpretResult)0
#define INTERPRET_COMPILE_ERROR (InterpretResult)1
#define INTERPRET_RUNTIME_ERROR (InterpretResult)2

DECLARE_HASHTABLE(Obj*, size_t, RTObj)

DECLARE_HASHTABLE(ObjString*, ObjModule *, Module)

typedef struct CallFrame {
    //ObjFunction *function; // function being called.
    Obj *function; // may be either a ObjClosure or ObjFunction
    uint8_t *ip; // caller's ip where function will return.
    // Value *window_stack;
    size_t frame_stack; // first slot index that function can use in VM's value stack.
} CallFrame;

typedef struct GCInfo GCInfo;
typedef struct VM VM;

struct VM {
    CallFrame frames[MAX_CALL_STACK]; // call frame stack
    size_t frame_count; // count of frame stack
    CompileContext compile_context;
    ValueArrayList *stack; // value stack
    ObjUpvalue *open_upvalues; // head pointer of open upvalue linked list
    RTObjHashtable *objects; // any object that allocated during runtime. used for gc.
    GCInfo *gc_info; // gc info
    VM *enclosing; // used for sub vm (when importing module) to save parent vm
    const char *source_path; // current file path
};

VM *init_VM(VM *vm, VM *enclosing, CompileContext *previous_context, bool register_native);

void free_VM(VM *v);

InterpretResult interpret(VM *v, const char *path, const char *source);

InterpretResult interpret_file(VM *v, const char *file_path);

/**
 * update allocated bytes size
 *
 * @param gc_info
 * @param size
 */
void deallocate_rt_obj(GCInfo *gc_info, size_t size);

/**
 * update next gc size after sweep
 *
 * @param gc_info
 */
void update_next_gc_size(GCInfo *gc_info);
#ifdef __cplusplus
}
#endif
#endif //ZHI_VM_VM_H_
