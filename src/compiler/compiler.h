//
// Created by ran on 2024-03-16.
//

#ifndef ZHI_COMPILER_COMPILER_H_
#define ZHI_COMPILER_COMPILER_H_
#include "chunk/object.h"
#include "parser.h"
#include "list/linked_list.h"

#define COMPILE_OK 0
#define COMPILE_ERROR 21

typedef char CompileType;
#define COMPILE_GLOBAL (CompileType)0
#define COMPILE_FUNCTION (CompileType) 1
#define COMPILE_METHOD (CompileType)2
#define COMPILE_INIT (CompileType)3

// local variable representation
typedef struct {
  Token name;
  int depth;
  uint32_t hash;
  int is_captured; // if the local variable is captured by a closure.
} Local;

typedef struct {
  size_t index;
  bool is_local;
} Upvalue;

DECLARE_ARRAY_LIST(Local, Local)
DECLARE_ARRAY_LIST(size_t, BreakPoint)

typedef struct {
  String_Hashtable *string_intern; // hash table for string interning
  LinkedList *objs; // objects that allocated during compiling. string objects are EXCLUDED because there are interning
  // in a hash table
  String_Hashtable *global_names; // map of global variable name to the index of value array where its value is stored
  ValueArrayList *global_values; // value of global variables
  ValueArrayList *class_objs; // value of class objects
} CompileContext;

typedef struct {
  LocalArrayList *local; // locals array
  int scope_depth; // current scope. starts from 0
} LocalContext;

typedef struct {
  int current_loop_start;      // track the start of loop
  int current_loop_scope_depth; // track the depth of loop
  BreakPointArrayList *current_loop_breaks; // track the index of break to path
  Expression *increment; // increment expression
} LoopState;

typedef struct Compiler {
  ObjFunction *function; // current compiling function
  CompileType type; // compile type, global script or function
  struct Compiler *enclosing; // enclosing compiler
  Upvalue upvalues[UINT8_MAX]; // upvalues
  CompileContext *context;
  Parser *parser;
  LocalContext *local_context;
  bool has_compile_error;
  LoopState *loop_state;
} Compiler;

extern NativeFunctionDecl *NATIVE_FUNC[];

extern NativeValueDecl *NATIVE_VAL[];

/**
 * Native function. Time used by system so far
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_func_clock(int arg_count, Value *args, void *);

/**
 * Native function. A GC indicator for garbage collection.
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_gc(int arg_count, Value *args, void *);

/**
 * Test if an instance has the field.
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_has_field(int arg_count, Value *args, void *);

/**
 * Test if an instance has the method.
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_has_method(int arg_count, Value *args, void *);

/**
 * Delete a field of instance.
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_del_field(int arg_count, Value *args, void *);

/**
 * length of a value
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_len(int arg_count, Value *args, void *);

/**
 * get the type of value
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_type(int arg_count, Value *args, void *);

/**
 * Stringify value.
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_stringify(int arg_count, Value *args, void *);

/**
 * print value and append an '\n' at the end.
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_puffln(int arg_count, Value *args, void *);

/**
 * print value.
 *
 * @param arg_count
 * @param args
 * @return
 */
Value native_puff(int arg_count, Value *args, void *);

int compile(Compiler *compiler, StatementArrayList *statement);

/**
 * Compile native functions. Native function names must start with "__" and occupy the
 * beginning slots of global value
 * array.
 *
 * Note that any user defined global variable that starts with "__" is considered as a
 * compile error.
 *
 * @param context
 * @param name
 * @param function
 * @param arity
 */
void define_native(CompileContext *context, const char *name, NativeFunction function, int arity);

/**
 * define native value
 *
 * @param context
 * @param name
 * @param value
 */
void define_native_variable(CompileContext *context, const char *name, Value value);
/**
 *
 * @param context
 */
void register_native_function(CompileContext *context);

/**
 *
 * @param context
 */
void register_native_values(CompileContext *context);
#endif //ZHI_COMPILER_COMPILER_H_
