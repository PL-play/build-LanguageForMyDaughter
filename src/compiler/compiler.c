//
// Created by ran on 2024-03-16.
//
#include "compiler/compiler.h"
#include "compiler/scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "chunk/debug.h"
#include "hashtable/hash-string.h"
#ifdef ALLOC_TESTING
#include "common/alloc-testing.h"
#endif
#ifdef WASM_LOG
#include <emscripten/emscripten.h>
#endif
#define VARIABLE_OPERATION(compiler, var_index, line_, op) do{ \
                                                  if((var_index)<256){ \
                                                    emit_byte((compiler),(op), (line_));\
                                                    emit_byte((compiler),(uint8_t) (var_index), (line_));  \
                                                  }\
                                                  else{              \
                                                    emit_byte((compiler), ((op)+1), (line_));\
                                                    emit_byte((compiler),(uint8_t) ((var_index) & 0xff), (line_));\
                                                    emit_byte((compiler),(uint8_t) (((var_index) >> 8) & 0xff), (line_));\
                                                    emit_byte((compiler),(uint8_t) (((var_index) >> 16) & 0xff), (line_));\
                                                  }\
                                              }while(0)

static Local null_local = {.depth = -1};

DEFINE_ARRAY_LIST(null_local, Local)
// Since the "break" must be in the loop and won't be at index 0, so set 0 to represent the null data is fine.
DEFINE_ARRAY_LIST(0, BreakPoint)

static void init_compiler(Compiler *compiler);

static Chunk *current_chunk(Compiler *compiler);

static void end_compiler(Compiler *compiler, size_t line);

static void emit_byte(Compiler *compiler, uint8_t byte, size_t line);

static void emit_return(Compiler *compiler, size_t line);

static void emit_constant(Compiler *compiler, Value value, size_t line);

static size_t emit_jump(Compiler *compiler, OpCode op_code, size_t line);

static void patch_jump(Compiler *compiler, size_t offset);

static void emit_int_3bytes(Compiler *compiler, size_t i, size_t line_);

static void emit_loop(Compiler *compiler, size_t location, size_t line);

static void emit_class(Compiler *compiler, Value value, size_t line);

static void *expression_emit_bytecode(Compiler *compiler, Expression *expression);

static void statement_emit_bytecode(Compiler *compiler, Statement *statement);

static void literal(Compiler *compiler, Expression *e);

static void number(Compiler *compiler, Expression *e);

static void int_number(Compiler *compiler, Expression *e);

static void prefix(Compiler *compiler, Expression *e);

static void postfix(Compiler *compiler, Expression *e);

static void infix(Compiler *compiler, Expression *e);

static int global_variable_decl(Compiler *compiler, Token name, Expression *expression, bool emit_nil);

static int global_variable_assign(Compiler *compiler, Expression *expression);

static int global_variable_get(Compiler *compiler, Expression *expression);

static int declare_variable(Compiler *compiler, Token name);

static int add_local(Compiler *compiler, Token name);

static int resolve_local(Compiler *compiler, Token name);

static ObjString *get_string_intern(CompileContext *context, Token v);

static int resolve_upvalue(Compiler *compiler, Token name);

static int add_upvalue(Compiler *compiler, size_t index, bool is_local, bool *overflow);

static ObjFunction *function(Compiler *compiler, Expression *expression, CompileType type);

static void emit_synthetic_this_expression(Compiler *compiler, size_t line);

static void emit_synthetic_super_expression(Compiler *compiler, size_t line);

static void begin_scope(Compiler *compiler);

static void end_scope(Compiler *compiler);

static void error_at(Compiler *compiler, Token *token, const char *msg);

static void warn_at(Compiler *compiler, Token *token, const char *msg);

static bool is_same_name(Token name1, Token name2);

static NativeFunctionDecl f1 = (NativeFunctionDecl){.name = "__clock", .arity = 0, .function = native_func_clock};
static NativeFunctionDecl f2 = (NativeFunctionDecl){.name = "__GC", .arity = 0, .function = native_gc};
static NativeFunctionDecl f3 = (NativeFunctionDecl){.name = "__has_field", .arity = 2, .function = native_has_field};
static NativeFunctionDecl f4 = (NativeFunctionDecl){.name = "__del_field", .arity = 2, .function = native_del_field};
static NativeFunctionDecl f5 = (NativeFunctionDecl){.name = "__has_method", .arity = 2, .function = native_has_method};
static NativeFunctionDecl f6 = (NativeFunctionDecl){.name = "__len", .arity = 1, .function = native_len};
static NativeFunctionDecl f7 = (NativeFunctionDecl){.name = "__type", .arity = 1, .function = native_type};
static NativeFunctionDecl f8 = (NativeFunctionDecl){.name = "__str", .arity = 1, .function = native_stringify};
static NativeFunctionDecl f9 = (NativeFunctionDecl){.name = "puffln", .arity = 1, .function = native_puffln};
static NativeFunctionDecl f10 = (NativeFunctionDecl){.name = "puff", .arity = 1, .function = native_puff};

static NativeValueDecl v1 = {.name = "__bool_type", .value = NUMBER_VAL(VAL_BOOL)};
static NativeValueDecl v2 = {.name = "__nil_type", .value = NUMBER_VAL(VAL_NIL)};
static NativeValueDecl v3 = {.name = "__number_type", .value = NUMBER_VAL(VAL_NUMBER)};
static NativeValueDecl v4 = {.name = "__string_type", .value = NUMBER_VAL(OBJ_STRING + VAL_OBJ)};
static NativeValueDecl v5 = {.name = "__magic_type", .value = NUMBER_VAL(OBJ_FUNCTION + VAL_OBJ)};
static NativeValueDecl v6 = {.name = "__charm_type", .value = NUMBER_VAL(OBJ_CLOSURE + VAL_OBJ)};
static NativeValueDecl v7 = {.name = "__up_value_type", .value = NUMBER_VAL(OBJ_UPVALUE + VAL_OBJ)};
static NativeValueDecl v8 = {.name = "__castle_type", .value = NUMBER_VAL(OBJ_CLASS + VAL_OBJ)};
static NativeValueDecl v9 = {.name = "__knight_type", .value = NUMBER_VAL(OBJ_INSTANCE + VAL_OBJ)};
static NativeValueDecl v10 = {.name = "__method_type", .value = NUMBER_VAL(OBJ_INSTANCE_METHOD + VAL_OBJ)};
static NativeValueDecl v11 = {.name = "__realm_type", .value = NUMBER_VAL(OBJ_MODULE + VAL_OBJ)};
static NativeValueDecl v12 = {.name = "__array_type", .value = NUMBER_VAL(OBJ_ARRAY + VAL_OBJ)};

NativeFunctionDecl *NATIVE_FUNC[] = {
    &f1, &f2, &f3, &f4, &f5, &f6, &f7, &f8, &f9, &f10,
    NULL,
};

NativeValueDecl *NATIVE_VAL[] = {
    &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10, &v11, &v12,
    NULL
};

int compile(Compiler *compiler, StatementArrayList *statements) {
    init_compiler(compiler);
    for (size_t i = 0; i < statements->size; ++i) {
        statement_emit_bytecode(compiler, Statementget_data_arraylist(statements, i));
    }
    end_compiler(compiler, eof_line(compiler->parser));
#ifdef DEBUG_TRACE_EXECUTION
  if (!compiler->has_compile_error) {
    //disassemble_chunk(compiler->function->chunk, "=====compiler end. bytecodes: ======");
    // printf("======\n");
  }
#endif

    return compiler->has_compile_error ? COMPILE_ERROR : COMPILE_OK;
}

uint32_t local_hash(Local local) {
    return local.hash;
}

int local_equals(Local l1, Local l2) {
    if (l1.name.length != l2.name.length) return -1;
    return memcmp(l1.name.start, l2.name.start, l1.name.length);
}

static void init_compiler(Compiler *compiler) {
    compiler->has_compile_error = false;
    compiler->local_context->scope_depth = 0;
    compiler->local_context->local = Localnew_arraylist(32);

    // compiler implicitly claims stack slot zero for the VM’s own internal use.
    Token token = {.start = NULL, .length = 0};
    // add 'this' to local
    if (compiler->type != COMPILE_FUNCTION && compiler->type != COMPILE_GLOBAL) {
        token.start = THIS_LITERAL;
        token.length = strlen(THIS_LITERAL);
    }
    Local local = {.name = token, .depth = 0, .hash = 0, .is_captured = 0};
    Localappend_arraylist(compiler->local_context->local, local);

    LoopState *loop_state = malloc(sizeof(LoopState));
    loop_state->current_loop_start = -1;
    loop_state->current_loop_scope_depth = 0;
    loop_state->increment = NULL;
    loop_state->current_loop_breaks = BreakPointnew_arraylist(10);
    compiler->loop_state = loop_state;

    compiler->enclosing = NULL;
}

static void statement_emit_bytecode(Compiler *compiler, Statement *statement) {
    if (statement == NULL)return;
    ParseType type = statement->type;
    switch (type) {
        case PRINT_STMT: {
            expression_emit_bytecode(compiler, statement->print_expr);
            emit_byte(compiler, OP_PRINT, statement->line);
            return;
        }
        case EXPRESSION_STMT: {
            expression_emit_bytecode(compiler, statement->expression);
            emit_byte(compiler, OP_POP, statement->line);
            return;
        }
        case VAR_DECL_STMT: {
            int local_index = declare_variable(compiler, statement->var_decl.name);
            if (local_index != -1) {
                // local
                if (statement->var_decl.initializer != NULL) {
                    expression_emit_bytecode(compiler, statement->var_decl.initializer);
                } else {
                    emit_byte(compiler, OP_NIL, statement->line);
                }
                //int index = resolve_local(statement->var_decl.name);
                VARIABLE_OPERATION(compiler, local_index, statement->line, OP_SET_LOCAL);

                // mark the local as initialized.
                (&(compiler->local_context->local->data[local_index]))->depth = compiler->local_context->scope_depth;
            } else {
                // global
                int var_index = global_variable_decl(compiler, statement->var_decl.name,
                                                     statement->var_decl.initializer, true);
                VARIABLE_OPERATION(compiler, var_index, statement->line, OP_DEFINE_GLOBAL);
            }
            return;
        }
        case BLOCK_STMT: {
            begin_scope(compiler);
            for (size_t i = 0; i < statement->block.stmt_nums; ++i) {
                statement_emit_bytecode(compiler, statement->block.statements[i]);
            }
            end_scope(compiler);
            return;
        }
        case IF_STMT: {
            /**
             *        {IF CONDITION}                     |    if condition expression
             *    --- [OP_JUMP_IF_FALSE] [X][X][X]       |    jump if condition is false.  3 8-bits place holder of offset
             *   |    [OP_POP]                           |     <- pop condition value
             *   |    {THEN BRANCH}                      |    then statements
             *   |    [OP_JUMP] [X][X][X] ---            |    jump with no condition to skip else branch
             *   |--> [OP_POP]              |            |    <- pop condition value if the condition is false
             *        {ELSE BRANCH}         |            |    else statements
             *        [XXX]      <--------- |            |    next codes
             */
            // condition
            expression_emit_bytecode(compiler, statement->if_stmt.condition);
            // jump if false
            size_t then_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, statement->if_stmt.condition->line);
            // pop if skip then branch
            emit_byte(compiler, OP_POP, statement->if_stmt.condition->line);
            // then branch
            statement_emit_bytecode(compiler, statement->if_stmt.then_stmt);

            size_t else_jump = emit_jump(compiler, OP_JUMP, statement->if_stmt.condition->line);
            // patch
            patch_jump(compiler, then_jump);
            // pop after then branch
            emit_byte(compiler, OP_POP, statement->if_stmt.condition->line);

            if (statement->if_stmt.else_stmt != NULL) {
                statement_emit_bytecode(compiler, statement->if_stmt.else_stmt);
            }
            // patch else jump
            patch_jump(compiler, else_jump);
            return;
        }
        case WHILE_STMT: {
            /**
              *        {CONDITION}    <--------------|     |    condition expression
              *    --- [OP_JUMP_IF_FALSE] [X][X][X]  |     |    jump if condition is false.  3 8-bits place holder of offset
              *   |    [OP_POP]                      |     |     <- pop condition value
              *   |    {WHILE BODY}                  |     |    then statements
              *   |    [OP_JUMP_BACK]   --------------     |    LOOP
              *   |--> [OP_POP]                            |    <- pop condition value if the condition is false
              *        [XXX]                               |    next codes
              */

            // store surrounding loop state
            int pre_current_loop_start = compiler->loop_state->current_loop_start;
            int pre_current_loop_scope_depth = compiler->loop_state->current_loop_scope_depth;
            Expression *pre_increment = compiler->loop_state->increment;
            BreakPointArrayList *pre_breaks = compiler->loop_state->current_loop_breaks;

            compiler->loop_state->current_loop_start = (int) get_code_size(current_chunk(compiler));
            compiler->loop_state->current_loop_scope_depth = compiler->local_context->scope_depth;
            compiler->loop_state->increment = NULL;
            compiler->loop_state->current_loop_breaks = BreakPointnew_arraylist(10);

            expression_emit_bytecode(compiler, statement->while_stmt.condition);
            size_t exit_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, statement->while_stmt.condition->line);
            emit_byte(compiler, OP_POP, statement->while_stmt.condition->line);
            statement_emit_bytecode(compiler, statement->while_stmt.body);
            emit_loop(compiler, compiler->loop_state->current_loop_start, statement->while_stmt.condition->line);
            patch_jump(compiler, exit_jump);
            emit_byte(compiler, OP_POP, statement->while_stmt.condition->line);

            // patch break jumps
            for (int i = 0; i < compiler->loop_state->current_loop_breaks->size; ++i) {
                size_t break_point = BreakPointget_data_arraylist(compiler->loop_state->current_loop_breaks, i);
                patch_jump(compiler, break_point);
                // no need to pop condition value. it must be popped as condition is true to get to this break statement.
            }

            BreakPointfree_arraylist(compiler->loop_state->current_loop_breaks);
            // restore surrounding loop state
            compiler->loop_state->current_loop_start = pre_current_loop_start;
            compiler->loop_state->current_loop_scope_depth = pre_current_loop_scope_depth;
            compiler->loop_state->increment = pre_increment;
            compiler->loop_state->current_loop_breaks = pre_breaks;
            return;
        }
        case FOR_STMT: {
            /**
             *        {INITIALIZER}
             *        {CONDITION}    <--------------|     |    condition expression
             *    --- [OP_JUMP_IF_FALSE] [X][X][X]  |     |    jump if condition is false.  3 8-bits place holder of offset
             *   |    [OP_POP]                      |     |     <- pop condition value
             *   |    {FOR BODY}                    |     |    FOR BODY
             *   |    {INCREMENT}                   |     |    INCREMENT EXPRESSION
             *   |    [OP_JUMP_BACK]   --------------     |    LOOP
             *   |--> [OP_POP]                            |    <- pop condition value if the condition is false
             *        [XXX]                               |    next codes
             */

            begin_scope(compiler);
            // store surrounding loop state
            int pre_current_loop_start = compiler->loop_state->current_loop_start;
            int pre_current_loop_scope_depth = compiler->loop_state->current_loop_scope_depth;
            Expression *pre_increment = compiler->loop_state->increment;
            BreakPointArrayList *pre_breaks = compiler->loop_state->current_loop_breaks;

            // initializer
            statement_emit_bytecode(compiler, statement->for_stmt.initializer);

            compiler->loop_state->current_loop_start = (int) get_code_size(current_chunk(compiler));
            compiler->loop_state->current_loop_scope_depth = compiler->local_context->scope_depth;
            compiler->loop_state->increment = statement->for_stmt.increment;
            compiler->loop_state->current_loop_breaks = BreakPointnew_arraylist(10);

            size_t exit_jump = -1;
            // if there is a condition
            if (statement->for_stmt.condition != NULL) {
                expression_emit_bytecode(compiler, statement->for_stmt.condition);
                exit_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, statement->for_stmt.condition->line);
                emit_byte(compiler, OP_POP, statement->for_stmt.condition->line);
            }
            statement_emit_bytecode(compiler, statement->for_stmt.body);

            // increment
            if (statement->for_stmt.increment != NULL) {
                expression_emit_bytecode(compiler, statement->for_stmt.increment);
                emit_byte(compiler, OP_POP, statement->for_stmt.increment->line);
            }
            // TODO what's the line number if there is no condition? so far set to 1
            emit_loop(compiler,
                      compiler->loop_state->current_loop_start,
                      exit_jump == -1 ? 1 : statement->for_stmt.condition->line);

            if (exit_jump != -1) {
                patch_jump(compiler, exit_jump);
                emit_byte(compiler, OP_POP, exit_jump == -1 ? 1 : statement->for_stmt.condition->line);
            }

            // patch break jumps
            for (int i = 0; i < compiler->loop_state->current_loop_breaks->size; ++i) {
                size_t break_point = BreakPointget_data_arraylist(compiler->loop_state->current_loop_breaks, i);
                patch_jump(compiler, break_point);
                // no need to pop condition value. it must be popped as condition is true to get to this break statement.
            }

            BreakPointfree_arraylist(compiler->loop_state->current_loop_breaks);
            // restore surrounding loop state
            compiler->loop_state->current_loop_start = pre_current_loop_start;
            compiler->loop_state->current_loop_scope_depth = pre_current_loop_scope_depth;
            compiler->loop_state->increment = pre_increment;
            compiler->loop_state->current_loop_breaks = pre_breaks;
            end_scope(compiler);
            return;
        }
        case CONTINUE_STMT: {
            if (compiler->loop_state->current_loop_start == -1) {
                error_at(compiler, &statement->continue_stmt.token, "Can't use 'skip' outside of a loop.");
            }
            // discard locals create inside the loop
            for (int i = compiler->local_context->local->size - 1; i >= 0; --i) {
                if (compiler->local_context->local->data[i].depth <= compiler->loop_state->current_loop_scope_depth) {
                    break;
                }
                emit_byte(compiler, OP_POP, statement->continue_stmt.token.line);
            }
            // TODO duplicate increment expression in continue. is it a good way for continue in for statement?
            if (compiler->loop_state->increment != NULL) {
                expression_emit_bytecode(compiler, compiler->loop_state->increment);
                emit_byte(compiler, OP_POP, compiler->loop_state->increment->line);
            }
            // jump to the current loop start
            emit_loop(compiler, compiler->loop_state->current_loop_start, statement->continue_stmt.token.line);
            return;
        }
        case BREAK_STMT: {
            if (compiler->loop_state->current_loop_start == -1) {
                error_at(compiler, &statement->continue_stmt.token, "Can't use 'break' outside of a loop.");
            }
            // discard locals create inside the loop
            for (int i = compiler->local_context->local->size - 1; i >= 0; --i) {
                if (compiler->local_context->local->data[i].depth <= compiler->loop_state->current_loop_scope_depth) {
                    break;
                }
                emit_byte(compiler, OP_POP, statement->continue_stmt.token.line);
            }
            // jump
            size_t break_jump = emit_jump(compiler, OP_JUMP, statement->break_stmt.token.line);
            BreakPointappend_arraylist(compiler->loop_state->current_loop_breaks, break_jump);
            return;
        }
        case FUNC_STMT: {
            // functions are first-class values so a function name is compiled just like any other variable declaration.
            // A top level declared function will bind to a global variable. Inside a block or other function will create
            // a local variable.
            int local_index = declare_variable(compiler, statement->function_stmt.name);
#ifdef WASM_LOG
            char buff_name[statement->function_stmt.name.length+1];
            snprintf(buff_name,statement->function_stmt.name.length+1,"%s",statement->function_stmt.name.start);
            char n[statement->function_stmt.name.length+64];
            sprintf(n,"[bytecode]---- bytecodes of [%s] -----[nl]",buff_name);
            EM_ASM_({console.warn(UTF8ToString($0));}, n);
#endif
            if (local_index != -1) {
                // local
                // mark the local as initialized.
                (&(compiler->local_context->local->data[local_index]))->depth = compiler->local_context->scope_depth;
                ObjFunction *func = expression_emit_bytecode(compiler, statement->function_stmt.function);
                func->name = get_string_intern(compiler->context, statement->function_stmt.name);
                VARIABLE_OPERATION(compiler, local_index, statement->line, OP_SET_LOCAL);
            } else {
                // global
                int var_index =
                        global_variable_decl(compiler, statement->function_stmt.name, statement->function_stmt.function,
                                             true);
                VARIABLE_OPERATION(compiler, var_index, statement->line, OP_DEFINE_GLOBAL);
            }
#ifdef WASM_LOG
            char n2[statement->function_stmt.name.length+64];
            sprintf(n2,"[bytecode]---- end bytecodes of [%s] -----[nl]",buff_name);
            EM_ASM_({console.warn(UTF8ToString($0));}, n2);
#endif
            return;
        }
        case RETURN_STMT: {
            if (compiler->type == COMPILE_INIT) {
                error_at(compiler, &statement->return_stmt.keyword, "Can't return from initializer.");
            }
            if (statement->return_stmt.value != NULL) {
                expression_emit_bytecode(compiler, statement->return_stmt.value);
            } else {
                emit_byte(compiler, OP_NIL, statement->return_stmt.keyword.line);
            }
            emit_return(compiler, statement->return_stmt.keyword.line);
            return;
        }
        case IMPORT_STMT: {
            Token lib = statement->import_stmt.lib;
            size_t lib_str_len = lib.escaped_string->size;
            char lib_str[lib_str_len];
            memcpy(lib_str, lib.escaped_string->data, lib_str_len);
            lib_str[lib_str_len] = '\0';
            Stringfree_arraylist(lib.escaped_string);

            if (lib_str_len == 0) {
                error_at(compiler, &lib, "Can't want by empty name.");
                return;
            }
            // TODO lib path location, should I refine lib path in compiler or refine it in runtime?
            Token v = {.length = lib_str_len, .start = lib_str};
            ObjString *path = get_string_intern(compiler->context, v);
            emit_constant(compiler, OBJ_VAL(path), lib.line);

            ObjString *as = get_string_intern(compiler->context, statement->import_stmt.as->variable.name);
            emit_constant(compiler, OBJ_VAL(as), lib.line);

            emit_byte(compiler, OP_IMPORT, lib.line);
            emit_byte(compiler, statement->import_stmt.as != NULL, lib.line);
            // opcode: [OP_IMPORT][1]
            // stack: [path][alias]
            // declare module as a variable. => "var alias = Module;"
            int local_index = declare_variable(compiler, statement->import_stmt.as->variable.name);
            // TODO new opcode for define module object
            if (local_index != -1) {
                // local
                VARIABLE_OPERATION(compiler, local_index, statement->line, OP_SET_LOCAL);
                // mark the local as initialized.
                (&(compiler->local_context->local->data[local_index]))->depth = compiler->local_context->scope_depth;
            } else {
                // global
                int var_index = global_variable_decl(compiler, statement->import_stmt.as->variable.name, NULL, false);
                VARIABLE_OPERATION(compiler, var_index, statement->line, OP_DEFINE_GLOBAL);
            }
            return;
        }
        case CLASS_STMT: {
            int local_index = declare_variable(compiler, statement->class_stmt.name);
            // compile class
            ObjString *class_name = get_string_intern(compiler->context, statement->class_stmt.name);
            ObjClass *klass = new_class(class_name);
            Value value = OBJ_VAL(klass);
            int global_index = -1;
            if (local_index != -1) {
                // mark the local as initialized.
                (&(compiler->local_context->local->data[local_index]))->depth = compiler->local_context->scope_depth;
                // compile class
                emit_class(compiler, value, statement->class_stmt.name.line);
                if (statement->class_stmt.super_class != NULL) {
                    // inherit from superclass
                    expression_emit_bytecode(compiler, statement->class_stmt.super_class);
                    add_chunk(current_chunk(compiler), OP_INHERIT, statement->class_stmt.super_class->line);
                }
                // local
                VARIABLE_OPERATION(compiler, local_index, statement->line, OP_SET_LOCAL);
                // set local operation leaves the value on the stack, so no need to push the value again
            } else {
                // global
                global_index = global_variable_decl(compiler, statement->var_decl.name, NULL, false);
                emit_class(compiler, value, statement->class_stmt.name.line);
                if (statement->class_stmt.super_class != NULL) {
                    // inherit from superclass
                    expression_emit_bytecode(compiler, statement->class_stmt.super_class);
                    add_chunk(current_chunk(compiler), OP_INHERIT, statement->class_stmt.super_class->line);
                }
                VARIABLE_OPERATION(compiler, global_index, statement->line, OP_DEFINE_GLOBAL);
            }

            // the class object is now at the top of VM's stack
            begin_scope(compiler);

            // static methods
            for (size_t i = 0; i < statement->class_stmt.static_methods_num; ++i) {
                Statement *func_stmt = statement->class_stmt.static_methods[i];
                int index = declare_variable(compiler, func_stmt->function_stmt.name);
                // must be a local, mark the function as initialized.
                (&(compiler->local_context->local->data[index]))->depth = compiler->local_context->scope_depth;

                // compile function
                ObjFunction *func = function(compiler, func_stmt->function_stmt.function, COMPILE_METHOD);
                func->name = get_string_intern(compiler->context, func_stmt->function_stmt.name);
                // set function
                VARIABLE_OPERATION(compiler, index, func_stmt->line, OP_SET_LOCAL);
                if (local_index != -1) {
                    // class is local variable
                    VARIABLE_OPERATION(compiler, i + 1, func_stmt->line, OP_METHOD_LOCAL);
                    emit_byte(compiler, 1, func_stmt->line);
                } else {
                    // global class
                    VARIABLE_OPERATION(compiler, global_index, func_stmt->line, OP_METHOD_GLOBAL);
                    emit_byte(compiler, 1, func_stmt->line);
                }
            }

            if (statement->class_stmt.super_class != NULL) {
                // if a class has super class, create a new scope and make it a local variable. -> "var super = SuperClass;"
                Token synthetic_token = {
                    .line = statement->class_stmt.super_class->line,
                    .length = strlen(SUPER_LITERAL),
                    .start = SUPER_LITERAL,
                    .type = TOKEN_IDENTIFIER
                };

                Statement synthetic_stmt = {
                    .type = VAR_DECL_STMT,
                    .line = statement->class_stmt.super_class->line,
                    .var_decl.initializer = statement->class_stmt.super_class,
                    .var_decl.name = synthetic_token
                };

                statement_emit_bytecode(compiler, &synthetic_stmt);
            }

            // methods
            for (size_t i = 0; i < statement->class_stmt.methods_num; ++i) {
                // methods are function statements
                Statement *func_stmt = statement->class_stmt.methods[i];
                // is initialization method
                size_t init_name_len = strlen(INIT_METHOD_NAME);
                bool is_init = func_stmt->function_stmt.name.length == init_name_len
                               && memcmp(func_stmt->function_stmt.name.start, INIT_METHOD_NAME, init_name_len) == 0;
                CompileType ct = is_init ? COMPILE_INIT : COMPILE_METHOD;

                int index = declare_variable(compiler, func_stmt->function_stmt.name);
                // must be a local, mark the function as initialized.
                (&(compiler->local_context->local->data[index]))->depth = compiler->local_context->scope_depth;

                // compile function
                ObjFunction *func = function(compiler, func_stmt->function_stmt.function, ct);
                func->name = get_string_intern(compiler->context, func_stmt->function_stmt.name);
                // set function
                VARIABLE_OPERATION(compiler, index, func_stmt->line, OP_SET_LOCAL);

                if (local_index != -1) {
                    // class is local variable
                    VARIABLE_OPERATION(compiler, local_index, func_stmt->line, OP_METHOD_LOCAL);
                    emit_byte(compiler, 0, func_stmt->line);
                } else {
                    // global class
                    VARIABLE_OPERATION(compiler, global_index, func_stmt->line, OP_METHOD_GLOBAL);
                    emit_byte(compiler, 0, func_stmt->line);
                }
            }

            end_scope(compiler);

            return;
        }
        default: {
            Token t = {.type = TOKEN_ERROR, .line = statement->line};
            error_at(compiler, &t, "Unsupported statement.");
        };
    }
}

static void *expression_emit_bytecode(Compiler *compiler, Expression *expression) {
    if (expression == NULL) return NULL;
    ParseType type = expression->type;
    switch (type) {
        case ASSIGN_EXPR: {
            int var_index = resolve_local(compiler, expression->assign.name);
            if (var_index != -1) {
                // local
                expression_emit_bytecode(compiler, expression->assign.right);
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_SET_LOCAL);
            } else if ((var_index = resolve_upvalue(compiler, expression->variable.name)) != -1) {
                // upvalue
                expression_emit_bytecode(compiler, expression->assign.right);
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_SET_UPVALUE);
            } else {
                // global
                var_index = global_variable_assign(compiler, expression);
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_SET_GLOBAL);
            }
            return NULL;
        }
        case THIS_EXPR: {
            int var_index = resolve_local(compiler, expression->variable.name);
            if (var_index != -1) {
                // local
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_GET_LOCAL);
            } else if ((var_index = resolve_upvalue(compiler, expression->variable.name)) != -1) {
                // upvalue
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_GET_UPVALUE);
            }
            return NULL;
        }
        case SET_EXPR: {
            expression_emit_bytecode(compiler, expression->set.object);
            expression_emit_bytecode(compiler, expression->set.value);

            ObjString *name = get_string_intern(compiler->context, expression->set.name);
            Value prop_name = OBJ_VAL(name);
            write_set_prop(current_chunk(compiler), prop_name, expression->line);
            return NULL;
        }
        case GET_EXPR: {
            expression_emit_bytecode(compiler, expression->get.object);
            ObjString *name = get_string_intern(compiler->context, expression->get.name);
            Value prop_name = OBJ_VAL(name);
            if (expression->get.object->type == SUPER_EXPR) {
                // need to push 'this'(the subclass instance) on stack as the receiver for method call
                emit_synthetic_this_expression(compiler, expression->get.object->line);
                // after this, the VM's stack looks like: ..[super class][subclass instance]
                write_get_super_prop(current_chunk(compiler), prop_name, expression->line);
            } else {
                write_get_prop(current_chunk(compiler), prop_name, expression->line);
            }
            return NULL;
        }
        case CALL_EXPR: {
            // if callee is get expression, emit OP_INVOKE to optimize method call
            bool is_get = expression->call.function->type == GET_EXPR;
            bool is_super_invoke = is_get && expression->call.function->get.object->type == SUPER_EXPR;
            if (is_get) {
                if (is_super_invoke) {
                    // super invoke, the receiver is 'this' variable
                    emit_synthetic_this_expression(compiler, expression->call.function->get.object->line);
                } else {
                    expression_emit_bytecode(compiler, expression->call.function->get.object);
                }
                // compile arguments
                for (int i = 0; i < expression->call.arg_num; ++i) {
                    expression_emit_bytecode(compiler, expression->call.args[i]);
                }

                ObjString *name = get_string_intern(compiler->context, expression->call.function->get.name);
                Value prop_name = OBJ_VAL(name);

                // [invoke][name][args num]
                if (is_super_invoke) {
                    // push an extra superclass on VM's stack
                    // [this][arg0][arg1]..[argn][superclass]
                    emit_synthetic_super_expression(compiler, expression->super_.super_.line);
                    write_invoke_super(current_chunk(compiler), prop_name, expression->line);
                } else {
                    write_invoke(current_chunk(compiler), prop_name, expression->line);
                }
                emit_byte(compiler, (uint8_t) expression->call.arg_num, expression->line);
            } else {
                expression_emit_bytecode(compiler, expression->call.function);
                // compile arguments
                for (int i = 0; i < expression->call.arg_num; ++i) {
                    expression_emit_bytecode(compiler, expression->call.args[i]);
                }

                // [call] [args num]
                emit_byte(compiler, OP_CALL, expression->line);
                emit_byte(compiler, (uint8_t) expression->call.arg_num, expression->line);
            }
            return NULL;
        }
        case CONDITIONAL_EXPR: {
            /**
             *        {CONDITION}                        |    condition expression
             *    --- [OP_JUMP_IF_FALSE] [X][X][X]       |    jump if condition is false.  3 8-bits place holder of offset
             *   |    [OP_POP]                           |     <- pop condition value
             *   |    {THEN EXPRESSION}                  |    then expression
             *   |    [OP_JUMP] [X][X][X] ---            |    jump with no condition to skip else branch
             *   |--> [OP_POP]              |            |    <- pop condition value if the condition is false
             *        {ELSE EXPRESSION}     |            |    else expression
             *        [XXX]      <--------- |            |    next codes
             */
            Expression *condition = expression->conditional.condition;
            Expression *then_expression = expression->conditional.then_exp;
            Expression *else_expression = expression->conditional.else_exp;
            // condition
            expression_emit_bytecode(compiler, condition);
            // jump if false
            size_t then_jump = emit_jump(compiler, OP_JUMP_IF_FALSE, condition->line);
            // pop condition value
            emit_byte(compiler, OP_POP, condition->line);
            // then expression
            expression_emit_bytecode(compiler, then_expression);
            size_t else_jump = emit_jump(compiler, OP_JUMP, condition->line);
            // patch
            patch_jump(compiler, then_jump);
            // pop after then branch
            emit_byte(compiler, OP_POP, condition->line);
            // else expression
            expression_emit_bytecode(compiler, else_expression);
            // patch else jump
            patch_jump(compiler, else_jump);
            return NULL;
        }
        case FUNC_EXPR: {
            // save the enclosing compiling function
            // init compiler
            return function(compiler, expression, COMPILE_FUNCTION);
        }

        case SUPER_EXPR: {
            // super expression is equivalent to variable expression whose token name is "super"
            // so make a synthetic variable expression and let it do the job
            emit_synthetic_super_expression(compiler, expression->super_.super_.line);
            return NULL;
        }

        case ARRAY_EXPR: {
            if (expression->array.size > 0) {
                for (size_t i = 0; i < expression->array.size; ++i) {
                    expression_emit_bytecode(compiler, expression->array.elements[i]);
                }
            }
            emit_byte(compiler, OP_ARRAY, expression->line);
            emit_int_3bytes(compiler, expression->array.size, expression->line);
            return NULL;
        }

        case INDEXING_EXPR: {
            expression_emit_bytecode(compiler, expression->indexing.object);
            expression_emit_bytecode(compiler, expression->indexing.index);
            emit_byte(compiler, OP_INDEXING, expression->line);
            return NULL;
        }

        case SLICING_EXPR: {
            expression_emit_bytecode(compiler, expression->slicing.object);
            bool index0 = expression->slicing.index0 != NULL;
            expression_emit_bytecode(compiler, expression->slicing.index0);
            bool index1 = expression->slicing.index1 != NULL;
            expression_emit_bytecode(compiler, expression->slicing.index1);
            emit_byte(compiler, OP_SLICING, expression->line);
            emit_byte(compiler, index0, expression->line);
            emit_byte(compiler, index1, expression->line);
            return NULL;
        }

        case ELEMENT_ASSIGN_EXPR: {
            expression_emit_bytecode(compiler, expression->element_assign.object);
            expression_emit_bytecode(compiler, expression->element_assign.index);
            expression_emit_bytecode(compiler, expression->element_assign.right);
            emit_byte(compiler, OP_ELEMENT_ASSIGN, expression->line);
            return NULL;
        }

        case VARIABLE_EXPR: {
            int var_index = resolve_local(compiler, expression->variable.name);
            if (var_index != -1) {
                // local
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_GET_LOCAL);
            } else if ((var_index = resolve_upvalue(compiler, expression->variable.name)) != -1) {
                // upvalue
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_GET_UPVALUE);
            } else {
                // global
                var_index = global_variable_get(compiler, expression);
                VARIABLE_OPERATION(compiler, var_index, expression->line, OP_GET_GLOBAL);
            }
            return NULL;
        }
        case OPERATOR_EXPR: {
            expression_emit_bytecode(compiler, expression->operator.left);
            expression_emit_bytecode(compiler, expression->operator.right);
            infix(compiler, expression);
            return NULL;
        }
        case PREFIX_EXPR: {
            expression_emit_bytecode(compiler, expression->prefix.right);
            prefix(compiler, expression);
            return NULL;
        }
        case POSTFIX_EXPR: {
            expression_emit_bytecode(compiler, expression->postfix.left);
            postfix(compiler, expression);
            return NULL;
        }
        case OR_EXPR: {
            /**
             *        {LEFT OPERAND EXPRESSION}            |    LEFT expression
             *    --- [OP_JUMP_IF_FALSE] [X][X][X]         |    jump if condition is false.  3 8-bits place holder of offset
             *   |    [OP_JUMP] [X][X][X]          ---     |
             *   |->  [OP_POP]                       |     |     <- pop condition value
             *        {RIGHT OPERAND EXPRESSION}     |     |    RIGHT expression
             *        [XXX]                        <-|     |    next
             *
           */
            expression_emit_bytecode(compiler, expression->or.left);
            size_t jump_if_false = emit_jump(compiler, OP_JUMP_IF_FALSE, expression->line);
            size_t jump_if_true = emit_jump(compiler, OP_JUMP, expression->line);
            patch_jump(compiler, jump_if_false);
            emit_byte(compiler, OP_POP, expression->line);

            expression_emit_bytecode(compiler, expression->or.right);
            patch_jump(compiler, jump_if_true);
            return NULL;
        }
        case AND_EXPR: {
            /**
              *        {LEFT OPERAND EXPRESSION}            |    LEFT expression
              *    --- [OP_JUMP_IF_FALSE] [X][X][X]         |    jump if condition is false.  3 8-bits place holder of offset
              *   |    [OP_POP]                             |     <- pop condition value
              *   |    {RIGHT OPERAND EXPRESSION}          |    RIGHT expression
              *   |->   [XXX]                              |    next
              *
            */
            expression_emit_bytecode(compiler, expression->and.left);
            size_t jump_if_false = emit_jump(compiler, OP_JUMP_IF_FALSE, expression->line);
            emit_byte(compiler, OP_POP, expression->line);
            expression_emit_bytecode(compiler, expression->and.right);
            patch_jump(compiler, jump_if_false);
            return NULL;
        }
        case LITERAL_EXPR: {
            literal(compiler, expression);
            return NULL;
        }

        default: {
            Token t = {.type = TOKEN_ERROR, .line = expression->line};
            error_at(compiler, &t, "Unsupported expression.");
            return NULL;
        }
    }
}

static Chunk *current_chunk(Compiler *compiler) {
    return compiler->function->chunk;
}

static void emit_byte(Compiler *compiler, uint8_t byte, size_t line) {
    add_chunk(current_chunk(compiler), byte, line);
}

static void emit_return(Compiler *compiler, size_t line) {
    emit_byte(compiler, OP_RETURN, line);
}

static void end_compiler(Compiler *compiler, size_t line) {
    if (compiler->type == COMPILE_INIT) {
        // return this in init function
        VARIABLE_OPERATION(compiler, 0, line, OP_GET_LOCAL);
    } else {
        emit_byte(compiler, OP_NIL, line);
    }
    emit_return(compiler, line);
#ifdef DEBUG_TRACE_EXECUTION
    //  if (!has_error()) {
    //    disassemble_chunk(current_chunk(), "code");
    //  }
#endif
#ifdef WASM_LOG
    if (!compiler->has_compile_error) {
        wasm_disassemble_chunk(compiler->function->chunk);
    }
#endif
    BreakPointfree_arraylist(compiler->loop_state->current_loop_breaks);
    Localfree_arraylist(compiler->local_context->local);
    free(compiler->loop_state);
}

static void debug_print_string_table(Compiler *compiler) {
    printf("size of string string_intern: %zu\n",
           String_size_of_hash_table(compiler->context->string_intern));
    fflush(stdout);
    String_HashtableIterator *i = String_hashtable_iterator(compiler->context->string_intern);
    while (String_hashtable_iter_has_next(i)) {
        String_KVEntry *e = String_hashtable_next_entry(i);
        printf("    [str obj: %p, %s]\n", e->key, e->key->string);
        fflush(stdout);
    }
    String_free_hashtable_iter(i);
    printf("\n");
}

static void literal(Compiler *compiler, Expression *e) {
    // TODO values
    switch (e->value.type) {
        case VAL_NUMBER: {
            number(compiler, e);
            break;
        }
        case VAL_NIL: {
            emit_byte(compiler, OP_NIL, e->line);
            break;
        }
        case VAL_BOOL: {
            emit_byte(compiler, e->value.boolean ? OP_TRUE : OP_FALSE, e->line);
            break;
        }
        case VAL_OBJ: {
            if (IS_STRING(e->value)) {
                // wrap a new Value and copy string
                ObjString *os = AS_STRING(e->value);
                char *str = malloc(os->length + 1);
                memcpy(str, os->string, os->length);
                str[os->length] = '\0';
#ifdef DEBUG_TRACE_EXECUTION
        printf("compile string: %s -(%lu)\n", str, strlen(str));
#endif
                uint32_t hash = fnv1a_hash(str, os->length);
                ObjString *nos = new_string_obj(str, os->length, hash);
                String_KVEntry *entry = String_get_entry_hash_table(compiler->context->string_intern, nos);
                if (entry != NULL) {
                    free_object((Obj *) nos);
                    emit_constant(compiler, OBJ_VAL(entry->key), e->line);
                } else {
                    String_put_hash_table(compiler->context->string_intern, nos, 1);
                    emit_constant(compiler, OBJ_VAL(nos), e->line);
                    // prepend_list(compiler->context->objs, nos);
                }
            }
            break;
        }
        default: return;
    }
}

static void int_number(Compiler *compiler, Expression *e) {
    // emit instruction
    emit_constant(compiler, e->value, e->line);
}

static void number(Compiler *compiler, Expression *e) {
    // emit instruction
    emit_constant(compiler, e->value, e->line);
}

static void prefix(Compiler *compiler, Expression *expression) {
    TokenType type = expression->prefix.operator;

    // Emit the operator instruction:
    // Evaluate the operand first which leaves its value on the stack.
    // Then pop that value, negate it, and push the result.
    // So the OP_NEGATE instruction should be emitted last.
    switch (type) {
        case TOKEN_MINUS: {
            emit_byte(compiler, OP_NEGATE, expression->line);
            break;
        }
        case TOKEN_BANG: {
            // ! is NOT in prefix
            emit_byte(compiler, OP_NOT, expression->line);
            break;
        }
        default: return;
    }
}

static void postfix(Compiler *compiler, Expression *e) {
    TokenType type = e->prefix.operator;

    switch (type) {
        case TOKEN_BANG: {
            // ! is FACTORIAL in postfix
            emit_byte(compiler, OP_FACTORIAL, e->line);
            break;
        }
        default: return;
    }
}

static void infix(Compiler *compiler, Expression *expression) {
    TokenType type = expression->operator.operator;
    size_t line = expression->line;

    switch (type) {
        case TOKEN_PLUS: {
            emit_byte(compiler, OP_ADD, line);
            break;
        }
        case TOKEN_MINUS: {
            emit_byte(compiler, OP_SUBTRACT, line);
            break;
        }
        case TOKEN_STAR: {
            emit_byte(compiler, OP_MULTIPLY, line);
            break;
        }
        case TOKEN_SLASH: {
            emit_byte(compiler, OP_DIVIDE, line);
            break;
        }
        case TOKEN_EXPONENT: {
            emit_byte(compiler, OP_EXPONENT, line);
            break;
        }
        case TOKEN_PERCENT: {
            emit_byte(compiler, OP_MOD, line);
            break;
        }
        case TOKEN_BANG_EQUAL: {
            emit_byte(compiler, OP_EQUAL, line);
            emit_byte(compiler, OP_NOT, line);
            break;
        }
        case TOKEN_EQUAL_EQUAL: {
            emit_byte(compiler, OP_EQUAL, line);
            break;
        }
        case TOKEN_GREATER: {
            emit_byte(compiler, OP_GREATER, line);
            break;
        }
        case TOKEN_GREATER_EQUAL: {
            emit_byte(compiler, OP_LESS, line);
            emit_byte(compiler, OP_NOT, line);
            break;
        }
        case TOKEN_LESS: {
            emit_byte(compiler, OP_LESS, line);
            break;
        }
        case TOKEN_LESS_EQUAL: {
            emit_byte(compiler, OP_GREATER, line);
            emit_byte(compiler, OP_NOT, line);
            break;
        }
        default: return;
    }
}

static void emit_constant(Compiler *compiler, Value value, size_t line) {
    write_constant(current_chunk(compiler), value, line);
}

static size_t emit_jump(Compiler *compiler, OpCode op_code, size_t line) {
    emit_byte(compiler, op_code, line);
    // place holder, 24-bits
    emit_byte(compiler, 0xff, line);
    emit_byte(compiler, 0xff, line);
    emit_byte(compiler, 0xff, line);
    return get_code_size(current_chunk(compiler)) - 3;
}

static void emit_int_3bytes(Compiler *compiler, size_t i, size_t line_) {
    emit_byte((compiler), (uint8_t) ((i) & 0xff), (line_));
    emit_byte((compiler), (uint8_t) (((i) >> 8) & 0xff), (line_));
    emit_byte((compiler), (uint8_t) (((i) >> 16) & 0xff), (line_));
}

static void patch_jump(Compiler *compiler, size_t offset) {
    size_t jump_length = get_code_size(current_chunk(compiler)) - offset - 3;
    if (jump_length > 1 << 24) {
        error_at(compiler, &compiler->parser->current, "too much code to jump over.");
    }
    current_chunk(compiler)->code->data[offset] = (uint8_t) (jump_length & 0xff);
    current_chunk(compiler)->code->data[offset + 1] = (uint8_t) ((jump_length >> 8) & 0xff);
    current_chunk(compiler)->code->data[offset + 2] = (uint8_t) ((jump_length >> 16) & 0xff);
}

static void emit_loop(Compiler *compiler, size_t location, size_t line) {
    emit_byte(compiler, OP_JUMP_BACK, line);

    size_t offset = get_code_size(current_chunk(compiler)) - location + 3;
    if (offset > 1 << 24) {
        // TODO fix line number
        error_at(compiler, &compiler->parser->current, "Loop body too large.");
    }
    emit_byte(compiler, (uint8_t) (offset & 0xff), line);
    emit_byte(compiler, (uint8_t) ((offset >> 8) & 0xff), line);
    emit_byte(compiler, (uint8_t) ((offset >> 16) & 0xff), line);
}

static void emit_class(Compiler *compiler, Value value, size_t line) {
    write_class(current_chunk(compiler), value, line);
    // Add class objects
    Valueappend_arraylist(compiler->context->class_objs, value);
}

static int global_variable_decl(Compiler *compiler, Token name, Expression *expression, bool emit_nil) {
    ObjString *os = get_string_intern(compiler->context, name);
    if (os->length >= 2 && os->string[0] == '_' && os->string[1] == '_') {
        // Can't define global variable that starts with "__"
        error_at(compiler, &name, "Can't define global variable starts with \"__\".");
    }

    int var_index = String_get_hash_table(compiler->context->global_names, os);
    if (var_index != -1) {
        // defined before
        warn_at(compiler, &name, "Redeclare global variable will override old value.");
    } else {
        // new variable
        var_index = (int) compiler->context->global_values->size;
        String_put_hash_table(compiler->context->global_names, os, var_index);
        Valueappend_arraylist(compiler->context->global_values, undefined_value);
    }

    if (expression != NULL) {
        void *ret = expression_emit_bytecode(compiler, expression);
        if (expression->type == FUNC_EXPR) {
            ObjFunction *func = (ObjFunction *) ret;
            func->name = os;
        }
    } else {
        if (emit_nil) {
            emit_byte(compiler, OP_NIL, name.line);
        }
    }
    return var_index;
}

static ObjString *get_string_intern(CompileContext *context, Token v) {
    char name[v.length + 1];
    memcpy(name, v.start, v.length);
    name[v.length] = '\0';

    uint32_t hash = fnv1a_hash(name, v.length);
    ObjString obj_string = {.obj = (Obj){.type = OBJ_STRING}, .length = v.length, .string = name, .hash = hash};

    String_KVEntry *e = String_get_entry_hash_table(context->string_intern, &obj_string);
    if (e != NULL) {
        return (ObjString *) String_table_entry_key(e);
    } else {
        char *str = malloc(v.length + 1);
        memcpy(str, name, v.length);
        str[v.length] = '\0';
        ObjString *os = new_string_obj(str, v.length, hash);
        String_put_hash_table(context->string_intern, os, 1);
        // DO NOT add string objects
        //append_list(context->objs, os);
        return os;
    }
}

/**
 * look for a local variable declared in the surrounding functions.
 *
 * @param compiler
 * @param name
 * @return an upvalue index for the variable. -1 if the variable wasn't found.
 */
static int resolve_upvalue(Compiler *compiler, Token name) {
    if (compiler->enclosing == NULL) return -1;

    int local = resolve_local(compiler->enclosing, name);
    // find local variable
    if (local != -1) {
        bool overflow;
        int index = add_upvalue(compiler, local, true, &overflow);
        if (overflow) {
            error_at(compiler, &name, "Too many closure variables(> 255) in a function.");
        } else {
            // mark the local as captured.
            compiler->enclosing->local_context->local->data[local].is_captured = 1;
        }

        return index;
    }

    int upvalue_index = resolve_upvalue(compiler->enclosing, name);
    if (upvalue_index != -1) {
        bool overflow;
        int index = add_upvalue(compiler, upvalue_index, false, &overflow);
        if (overflow) {
            error_at(compiler, &name, "Too many closure variables(> 255) in a function.");
        }
        return index;
    }

    return -1;
}

/**
 *
 * @param compiler
 * @param index
 * @param is_local
 * @return
 */
static int add_upvalue(Compiler *compiler, size_t index, bool is_local, bool *overflow) {
    *overflow = false;
    int upvalue_count = compiler->function->upvalue_count;

    // check if already have the upvalue
    for (int i = 0; i < upvalue_count; ++i) {
        Upvalue *upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->is_local == is_local) {
            return i;
        }
    }
    if (upvalue_count > UINT8_MAX) {
        *overflow = true;
        return 0;
    }

    Upvalue *upvalue = &compiler->upvalues[upvalue_count];
    upvalue->index = index;
    upvalue->is_local = is_local;
    return compiler->function->upvalue_count++;
}

static ObjFunction *function(Compiler *compiler, Expression *expression, CompileType type) {
    // save the enclosing compiling function
    // init compiler
    Compiler c;
    ObjFunction *func = new_function();
    func->arity = expression->function.param_num;

    c.function = func;
    c.type = type;
    c.context = compiler->context;
    c.parser = compiler->parser;
    LocalContext local_context;
    c.local_context = &local_context;
    init_compiler(&c);
    // set enclosing
    c.enclosing = compiler;

    begin_scope(&c);
    for (int i = 0; i < expression->function.param_num; ++i) {
        Token name = expression->function.parameters[i];
        size_t local_index = declare_variable(&c, name);
        // define variable
        (&((&c)->local_context->local->data[local_index]))->depth = (&c)->local_context->scope_depth;
    }

    // body
    begin_scope(&c);
    for (size_t i = 0; i < expression->function.body->block.stmt_nums; ++i) {
        statement_emit_bytecode(&c, expression->function.body->block.statements[i]);
    }
    end_scope(&c);

    end_scope(&c);
    end_compiler(&c, expression->function.body->line);

    c.enclosing = NULL;
    // if the function have upvalues, it's a closure
    if (func->upvalue_count == 0) {
        write_constant(compiler->function->chunk, OBJ_VAL(func), expression->function.body->line);
    } else {
        write_closure(compiler->function->chunk, OBJ_VAL(func), expression->function.body->line);
        // emit upvalues
        for (int i = 0; i < func->upvalue_count; ++i) {
            emit_byte(compiler, (&c)->upvalues[i].is_local ? 1 : 0, expression->function.body->line);
            emit_byte(compiler, (&c)->upvalues[i].index, expression->function.body->line);
        }
    }
#ifdef DEBUG_TRACE_EXECUTION
  if (!(&c)->has_compile_error) {
    //disassemble_chunk((&c)->function->chunk, "=====function compiler end. bytecodes: ======");
    //printf("======\n");
  }
#endif

    append_list(c.context->objs, func);

    return func;
}

static int global_variable_assign(Compiler *compiler, Expression *expression) {
    Token v = expression->assign.name;
    expression_emit_bytecode(compiler, expression->assign.right);
    ObjString *os = get_string_intern(compiler->context, v);

    int var_index = String_get_hash_table(compiler->context->global_names, os);
    if (var_index != -1) {
        // already defined
    } else {
        // define the variable with nil
        var_index = (int) compiler->context->global_values->size;
        String_put_hash_table(compiler->context->global_names, os, var_index);
        Valueappend_arraylist(compiler->context->global_values, undefined_value);
    }
    return var_index;
}

static int global_variable_get(Compiler *compiler, Expression *expression) {
    Token v = expression->variable.name;

    ObjString *os = get_string_intern(compiler->context, v);
    int var_index = String_get_hash_table(compiler->context->global_names, os);
    if (var_index != -1) {
        // already defined
    } else {
        // define the variable with nil
        var_index = (int) compiler->context->global_values->size;
        String_put_hash_table(compiler->context->global_names, os, var_index);
        Valueappend_arraylist(compiler->context->global_values, undefined_value);
    }
    return var_index;
}

static bool is_same_name(Token name1, Token name2) {
    if (name1.length != name2.length) return false;
    return memcmp(name1.start, name2.start, name1.length) == 0;
}

static int declare_variable(Compiler *compiler, Token name) {
    // global variables are late bound so compiler don't track them.
    if (compiler->local_context->scope_depth == 0) return -1;

    // error to have 2 variables with the same name in the same local scope
    for (int i = compiler->local_context->local->size - 1; i >= 0; --i) {
        Local local = Localget_data_arraylist(compiler->local_context->local, i);
        if (local.depth != -1 && local.depth < compiler->local_context->scope_depth) {
            break;
        }
        if (is_same_name(name, local.name)) {
            error_at(compiler, &name, "Duplicate variable name in this scope.");
        }
    }
    return add_local(compiler, name);
}

static int add_local(Compiler *compiler, Token name) {
    // make depth -1 means the local variable is "uninitialized"
    Local local = {.name = name, .depth = -1, .hash = fnv1a_hash(name.start, name.length), .is_captured = 0};
    Localappend_arraylist(compiler->local_context->local, local);
    return (int) compiler->local_context->local->size - 1;
}

static int resolve_local(Compiler *compiler, Token name) {
    for (int i = compiler->local_context->local->size - 1; i >= 0; --i) {
        Local local = Localget_data_arraylist(compiler->local_context->local, i);
        if (is_same_name(name, local.name)) {
            if (local.depth == -1) {
                error_at(compiler, &name, "Can't use local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

static void begin_scope(Compiler *compiler) {
    compiler->local_context->scope_depth++;
}

static void end_scope(Compiler *compiler) {
    compiler->local_context->scope_depth--;
    while (compiler->local_context->local->size > 0
           && Localget_data_arraylist(compiler->local_context->local, compiler->local_context->local->size - 1).depth
           > compiler->local_context->scope_depth) {
        Local l = Localget_data_arraylist(compiler->local_context->local, compiler->local_context->local->size - 1);
        if (l.is_captured == 1) {
            emit_byte(compiler, OP_CLOSE_UPVALUE, l.name.line);
        } else {
            emit_byte(compiler, OP_POP, l.name.line);
        }

        Localremove_arraylist(compiler->local_context->local, compiler->local_context->local->size - 1);
    }
}

static void error_at(Compiler *compiler, Token *token, const char *msg) {
#ifdef WASM_LOG
    char err[100];
    sprintf(err,"[status][error]-ZHI Compiler: [line %zu] Error", token->line);
#else
    fprintf(stderr, "[line %zu] Error", token->line);
#endif
    if (token->type == TOKEN_EOF) {
#ifdef WASM_LOG
        size_t el = strlen(err);
        sprintf(err+el," at end");
        EM_ASM_({console.warn(UTF8ToString($0));}, err);
#else
        fprintf(stderr, " at end");
#endif
    } else if (token->type == TOKEN_ERROR) {
#ifdef WASM_LOG
        EM_ASM_({console.warn(UTF8ToString($0));}, err);
#endif
    } else {
#ifdef WASM_LOG
        size_t el = strlen(err);
        sprintf(err+el, " at '%.*s'", token->length, token->start);
        EM_ASM_({console.warn(UTF8ToString($0));}, err);
#else
        fprintf(stderr, " at '%.*s'", token->length, token->start);
#endif
    }

#ifdef WASM_LOG
    char wasm_msg[512];
    sprintf(wasm_msg, "[status][error]-ZHI Compiler: %s", msg);
    EM_ASM_({console.warn(UTF8ToString($0));},wasm_msg);
#else
    fprintf(stderr, ": %s\n", msg);
#endif
    compiler->has_compile_error = true;
    Compiler *c = compiler->enclosing;
    while (c != NULL) {
        c->has_compile_error = true;
        c = c->enclosing;
    }
}

static void warn_at(Compiler *compiler, Token *token, const char *msg) {
#ifdef WASM_LOG
    char err[100];
    sprintf(err,"[status][info]-ZHI Compiler: [line %zu] WARNING", token->line);
#else
    fprintf(stdout, "[line %zu] WARNING", token->line);
#endif
    if (token->type == TOKEN_EOF) {
#ifdef WASM_LOG
        size_t el = strlen(err);
        sprintf(err+el," at end");
        EM_ASM_({console.warn(UTF8ToString($0));}, err);
#else
        fprintf(stdout, " at end");
#endif
    } else if (token->type == TOKEN_ERROR) {
#ifdef WASM_LOG
        EM_ASM_({console.warn(UTF8ToString($0));}, err);
#endif
    } else {
#ifdef WASM_LOG
        size_t el = strlen(err);
        sprintf(err+el, " at '%.*s'", token->length, token->start);
        EM_ASM_({console.warn(UTF8ToString($0));}, err);
#else
        fprintf(stdout, " at '%.*s'", token->length, token->start);
#endif
    }

#ifdef WASM_LOG
    char wasm_msg[512];
    sprintf(wasm_msg, "[status][info]-ZHI Compiler: %s", msg);
    EM_ASM_({console.warn(UTF8ToString($0));},wasm_msg);
#else
    fprintf(stdout, ": %s\n", msg);
#endif
}

static int obj_compare(Obj *obj1, Obj *obj2) {
    if (obj1 == obj2)return 0;
    return -1;
}

void register_native_function(CompileContext *context) {
    for (int i = 0; NATIVE_FUNC[i] != NULL; ++i) {
        NativeFunctionDecl *f = NATIVE_FUNC[i];
        define_native(context, f->name, f->function, f->arity);
    }
}

void register_native_values(CompileContext *context) {
    for (int i = 0; NATIVE_VAL[i] != NULL; ++i) {
        NativeValueDecl *f = NATIVE_VAL[i];
        define_native_variable(context, f->name, f->value);
    }
}

void define_native_variable(CompileContext *context, const char *name, Value value) {
    Token token = {.start = name, .length = strlen(name)};
    ObjString *g_name = get_string_intern(context, token);
    // new variable
    int var_index = (int) context->global_values->size;
    String_put_hash_table(context->global_names, g_name, var_index);
    Valueappend_arraylist(context->global_values, value);
}

void define_native(CompileContext *context, const char *name, NativeFunction function, int arity) {
    Token token = {.start = name, .length = strlen(name)};
    ObjString *fn_name = get_string_intern(context, token);
    ObjNative *native = new_native(function, arity);
    native->name = fn_name;

    int var_index = String_get_hash_table(context->global_names, fn_name);
    if (var_index != -1) {
        // defined before
        Value old_value = Valueget_data_arraylist(context->global_values, var_index);
        if (old_value.type == VAL_OBJ) {
            // free object in object list
            remove_list_equals_to(context->objs, old_value.obj, (LinkedListValueCompare) obj_compare);
        }
        free_value(old_value);
        Valueset_arraylist_data(context->global_values, var_index, OBJ_VAL(native));
    } else {
        // new variable
        var_index = (int) context->global_values->size;
        String_put_hash_table(context->global_names, fn_name, var_index);
        Valueappend_arraylist(context->global_values, OBJ_VAL(native));
    }
    // add to object list
    append_list(context->objs, native);
}

static void emit_synthetic_this_expression(Compiler *compiler, size_t line) {
    Token synthetic_token = {
        .line = line,
        .length = strlen(THIS_LITERAL),
        .start = THIS_LITERAL,
        .type = TOKEN_THIS
    };
    Expression synthetic_expression = {
        .type = THIS_EXPR,
        .line = line,
        .this_.this_ = synthetic_token
    };
    expression_emit_bytecode(compiler, &synthetic_expression);
}

static void emit_synthetic_super_expression(Compiler *compiler, size_t line) {
    Token synthetic_token = {
        .line = line,
        .length = strlen(SUPER_LITERAL),
        .start = SUPER_LITERAL,
        .type = TOKEN_IDENTIFIER
    };
    Expression synthetic_expression = {
        .type = VARIABLE_EXPR,
        .line = line,
        .variable.name = synthetic_token
    };
    expression_emit_bytecode(compiler, &synthetic_expression);
}
