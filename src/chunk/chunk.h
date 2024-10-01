//
// Created by ran on 2024-03-13.
//

#ifndef ZHI_CHUNK_CHUNK_H_
#define ZHI_CHUNK_CHUNK_H_

#include <stdint.h>
#include "value.h"

/**
 * instruction format
 *
 *  |   name          |  size   | operands |  semantic
 *  |                 | (bytes) | (bytes)  |
 * ==========================================================================
 * |  OP_RETURN       |   1    |   0      | return
 * --------------------------------------------------------------------------
 * | OP_CONSTANT      |   1    |   1     |  declare a constant value
 * ---------------------------------------------------------------------------
 * | OP_CONSTANT_LONG |   1    |   3     |  declare a constant value with which index>256
 * ---------------------------------------------------------------------------
 * | OP_NEGATE        |   1    |   1     |  make a negative value
 * ---------------------------------------------------------------------------
 * | OP_ADD           |   1    |   2     | add 2 operands
 * ---------------------------------------------------------------------------
 * | OP_SUBTRACT      |   1    |   2     | subtract 2 operands
 * ---------------------------------------------------------------------------
 * | OP_MULTIPLY      |   1    |   2     | multiply 2 operands
 * ---------------------------------------------------------------------------
 * | OP_DIVIDE        |   1    |   2     | divide 2 operands
 * ---------------------------------------------------------------------------
 * | OP_EXPONENT      |   1    |   2     | a^b exponent of 2 operands
 * ---------------------------------------------------------------------------
 * | OP_FACTORIAL     |   1    |   1     | a! factorial of 1 operand
 * ---------------------------------------------------------------------------
 * | OP_NOT           |   1    |   1     | !a the NOT operator
 * ---------------------------------------------------------------------------
 * | OP_NIL           |   1    |   0     | Nil literal
 * ---------------------------------------------------------------------------
 * | OP_TRUE          |   1    |   0     | true literal
 * ---------------------------------------------------------------------------
 * | OP_FALSE         |   1    |   0     | false literal
 * ---------------------------------------------------------------------------
 * | OP_EQUAL         |   1    |   2     | ==
 * ---------------------------------------------------------------------------
 * | OP_GREATER       |   1    |   2     | >
 * ---------------------------------------------------------------------------
 * | OP_LESS          |   1    |   2     | <
 * ---------------------------------------------------------------------------
 * | OP_PRINT         |   1    |   1     | print value
 * ---------------------------------------------------------------------------
 * | OP_POP           |   1    |   1     | pop the value off the stack
 * ---------------------------------------------------------------------------
 * | OP_DEFINE_GLOBAL |   1    |   1     | define a global variable which index < 256
 * ---------------------------------------------------------------------------
 * | OP_DEFINE_GLOBAL_LONG |   1    |   3    | define a global variable which index >= 256
 * ---------------------------------------------------------------------------
 * | OP_GET_GLOBAL |   1    |   1    | GET a global variable which index < 256
 * ---------------------------------------------------------------------------
 * | OP_GET_GLOBAL_LONG |   1    |   3    | GET a global variable which index >= 256
 *---------------------------------------------------------------------------
 * | OP_SET_GLOBAL|   1    |   1    | assign a global variable which index >= 256
 * ---------------------------------------------------------------------------
 * | OP_SET_GLOBAL_LONG |   1    |   3    | assign a global variable which index >= 256
 *---------------------------------------------------------------------------
 * | OP_SET_LOCAL    |   1    |   1    | set a local variable
 * ---------------------------------------------------------------------------
 * | OP_SET_LOCAL_LONG|   1    |   3    | set a local variable which index >=256
 * ---------------------------------------------------------------------------
 * | OP_GET_LOCAL |   1    |   1    | get a local variable
 * ---------------------------------------------------------------------------
 * | OP_GET_LOCAL_LONG|   1 |   3    | get a local variable which index >=256
 * ---------------------------------------------------------------------------
 * | OP_JUMP_IF_FALSE|   1 |   3    | jump the offset defined by operands if condition is false
 * ---------------------------------------------------------------------------
 * | OP_JUMP   |   1 |   3    | jump the offset defined by operands
 * ---------------------------------------------------------------------------
 * | OP_JUMP_BACK   |   1 |   3    | jump BACK the offset defined by operands
 * ---------------------------------------------------------------------------
 * | OP_MOD        |   1  |   1    | modulus or remainder operator
 * ---------------------------------------------------------------------------
 * | OP_CALL       |   1  |   1 + 1   | call function with argument count
 * ---------------------------------------------------------------------------
 * | OP_CLOSURE    |   1  |  1 + N| declare a closure value, followed by N upvalues
 * ---------------------------------------------------------------------------
 * | OP_CLOSURE_LONG |   1  |  3 + N| declare a closure value if the index of the closure is > 255, followed by N upvalues
 * ---------------------------------------------------------------------------
 * | OP_GET_UPVALUE |   1  |   1    | get a up value variable
 * ---------------------------------------------------------------------------
 * | OP_SET_UPVALUE |   1  |   1    | assign a up value variable
 * ---------------------------------------------------------------------------
 * | OP_CLASS       |   1  |   1    | declare a class object from class value array with given index
 * ---------------------------------------------------------------------------
 * | OP_CLASS_LONG  |   1  |   3    | declare a class object from class value array with given index>255
 * ---------------------------------------------------------------------------
 * | OP_METHOD_LOCAL     |   1  |   1 + 1   | bind the method and LOCAL class. the method is at the top of and index is the offset to it where class stays
 * ---------------------------------------------------------------------------
 * | OP_METHOD_LOCAL_LONG|   1  |   3 + 1   | bind the method and LOCAL class. the method is at the top of and index(>255) is the offset to it where class stays
 * ---------------------------------------------------------------------------
 * | OP_METHOD_GLOBAL     |   1  |   1    | bind the method and GLOBAL class. the method is at the top of and index is the offset to it where class stays
 * ---------------------------------------------------------------------------
 * | OP_METHOD_GLOBAL_LONG|   1  |   3    | bind the method and GLOBAL class. the method is at the top of and index(>255) is the offset to it where class stays
 * ---------------------------------------------------------------------------
 * | OP_GET_PROP   |   1  |   1   | get property of instance
 * ---------------------------------------------------------------------------
 * | OP_GET_PROP_LONG   |   1  |   3   | get property(index>255) of instance
 * ---------------------------------------------------------------------------
 * | OP_SET_PROP   |   1  |   1  | set property of instance
 * ---------------------------------------------------------------------------
 * | OP_SET_PROP_LONG   |   1  |   3  | set property(index>255) of instance
 * ---------------------------------------------------------------------------
 * | OP_GET_SUPER_PROP   |   1  |   1   | get method of super class
 * ---------------------------------------------------------------------------
 * | OP_GET_SUPER_PROP_LONG   |   1  |  3  | get method of super class(index>255)
 * ---------------------------------------------------------------------------
 * | OP_INVOKE   |   1  |   1 + 1   | invoke method call by the name , followed by arg nums
 * ---------------------------------------------------------------------------
 * | OP_INVOKE_LONG|   1  |   3 + 1   | invoke method call by the name where index>255, followed by arg nums
 * --------------------------------------------------------------------------
 * | OP_INVOKE_SUPER   |   1  |   1 + 1   | invoke SUPER method call by the name , followed by arg nums
 * ---------------------------------------------------------------------------
 * | OP_INVOKE_SUPER_LONG|   1  |   3 + 1   | invoke SUPER method call by the name where index>255, followed by arg nums
 * ---------------------------------------------------------------------------
 * | OP_INHERIT |   1  |   0   | inherit class, super class and subclass are on the top of stack
 * ---------------------------------------------------------------------------
 * | OP_IMPORT |   1  |   1   | import module, the operand is always set to 1 for now
 * ---------------------------------------------------------------------------
 * | OP_ARRAY |   1  |   3  | create a array with size of given operands
 * ---------------------------------------------------------------------------
 * | OP_INDEXING |  1  |   0  | get the element of given index and object on the stack
 *  ---------------------------------------------------------------------------
 * | OP_ELEMENT_ASSIGN |  1  |   0  | set the element of given value and object on the stack
 *  ---------------------------------------------------------------------------
 * | OP_SLICING |  1  |  2  | create a NEW array with given slicing indexes on the stack, operands indicate if the index exist
 */
typedef uint8_t OpCode;
#define OP_RETURN (OpCode)0
#define OP_CONSTANT (OpCode)1
#define OP_CONSTANT_LONG (OpCode)2
#define OP_NEGATE (OpCode)3
#define OP_ADD (OpCode)4
#define OP_SUBTRACT (OpCode)5
#define OP_MULTIPLY (OpCode)6
#define OP_DIVIDE (OpCode)7
#define OP_EXPONENT (OpCode)8
#define OP_FACTORIAL (OpCode)9
#define OP_NOT (OpCode)10
#define OP_NIL (OpCode)11
#define OP_TRUE (OpCode)12
#define OP_FALSE (OpCode)13
#define OP_EQUAL (OpCode)14
#define OP_GREATER (OpCode)15
#define OP_LESS (OpCode)16
#define OP_PRINT (OpCode)17
#define OP_POP (OpCode) 18
#define OP_DEFINE_GLOBAL (OpCode) 19
#define OP_DEFINE_GLOBAL_LONG (OpCode) 20
#define OP_GET_GLOBAL (OpCode) 21
#define OP_GET_GLOBAL_LONG (OpCode) 22
#define OP_SET_GLOBAL (OpCode) 23
#define OP_SET_GLOBAL_LONG (OpCode) 24
#define OP_SET_LOCAL (OpCode) 25
#define OP_SET_LOCAL_LONG (OpCode) 26
#define OP_GET_LOCAL (OpCode) 27
#define OP_GET_LOCAL_LONG (OpCode) 28
#define OP_JUMP_IF_FALSE (OpCode) 29
#define OP_JUMP (OpCode) 30
#define OP_JUMP_BACK (OpCode) 31
#define OP_MOD (OpCode) 32
#define OP_CALL (OpCode) 33
#define OP_CLOSURE (OpCode) 34
#define OP_CLOSURE_LONG (OpCode) 35
#define OP_GET_UPVALUE (OpCode) 36
#define OP_SET_UPVALUE (OpCode) 37
#define OP_CLOSE_UPVALUE (OpCode)38
#define OP_CLASS (OpCode)39
#define OP_CLASS_LONG (OpCode)40
#define OP_GET_PROP (OpCode)41
#define OP_GET_PROP_LONG (OpCode) 42
#define OP_SET_PROP (OpCode) 43
#define OP_SET_PROP_LONG (OpCode)44
#define OP_METHOD_LOCAL (OpCode)45
#define OP_METHOD_LOCAL_LONG (OpCode)46
#define OP_METHOD_GLOBAL (OpCode)47
#define OP_METHOD_GLOBAL_LONG (OpCode)48
#define OP_INVOKE (OpCode) 49
#define OP_INVOKE_LONG (OpCode) 50
#define OP_GET_SUPER_PROP (OpCode)51
#define OP_GET_SUPER_PROP_LONG (OpCode) 52
#define OP_INVOKE_SUPER (OpCode) 53
#define OP_INVOKE_SUPER_LONG (OpCode) 54
#define OP_INHERIT (OpCode) 55
#define OP_IMPORT (OpCode) 56
#define OP_ARRAY (OpCode) 57
#define OP_INDEXING (OpCode) 58
#define OP_ELEMENT_ASSIGN (OpCode) 59
#define OP_SLICING (OpCode) 60
#define OP_NULL (OpCode)255 // will never be used

// each LineStart marks the beginning of a new source line.and the corresponding
// byte offset of the first instruction on that line. Any bytes after that first
// one are understood to be on that same line, until it hits the next LineStart.
typedef struct {
  size_t offset;
  size_t line;
} LineStart;

typedef struct {
  size_t capacity, count;
  LineStart *lines;

} LineInfo;
DECLARE_ARRAY_LIST(uint8_t, Uint8t_)
typedef struct {
  Uint8t_ArrayList *code;
  ValueArrayList *constants;
  LineInfo line_info;
} Chunk;

/**
 * Exception table manages exception handling within a function. Each function has an associated exception table
 * that lists the ranges of bytecode instructions covered by try blocks and the corresponding catch blocks. The
 * table entries contain the following information:
 * - Start PC (Program Counter): The starting bytecode address (inclusive) of the try block.
 * - End PC: The ending bytecode address (exclusive) of the try block.
 * - Handler PC: The bytecode address of the catch block (exception handler).
 * - Catch Type: The type of exception to catch. This is a reference to the constant pool that specifies the exception.
 */
typedef struct ExceptionTable {
  size_t start_pc; // The beginning of the try block.
  size_t end_pc; // The end of the try block.
  size_t handle_pc; // The start of the catch block.
  size_t catch_type; // The type of exception to catch. (constant pool index)
} ExceptionTable;

Chunk *init_chunk();
void free_chunk(Chunk *chunk);
void add_chunk(Chunk *c, uint8_t byte, size_t line);
uint8_t get_code_of(Chunk *chunk, size_t index);
size_t get_code_size(Chunk *chunk);
/**
 * add a constant and return the index of it.
 *
 * @param c
 * @param value
 * @return
 */
size_t add_constant(Chunk *c, Value value);

static void add_constant_and_write(Chunk *c, Value v, size_t line, uint8_t op);
/**
 * add constant and write chunk.
 *
 * @param c
 * @param line
 */
void write_constant(Chunk *c, Value, size_t line);

/**
 * add closure and write chunk
 *
 * @param c
 * @param line
 */
void write_closure(Chunk *c, Value, size_t line);

/**
 * add class and write chunk
 *
 * @param c
 * @param v
 * @param line
 */
void write_class(Chunk *c, Value v, size_t line);

/**
 * add property name and write chunk
 *
 * @param c
 * @param v
 * @param line
 */
void write_get_prop(Chunk *c, Value v, size_t line);

/**
 * add property name and write chunk
 *
 * @param c
 * @param v
 * @param line
 */
void write_get_super_prop(Chunk *c, Value v, size_t line);

/**
 * add property name and write chunk
 *
 * @param c
 * @param v
 * @param line
 */
void write_set_prop(Chunk *c, Value v, size_t line);

/**
 * add invoke name and write chunk
 *
 * @param c
 * @param v
 * @param line
 */
void write_invoke(Chunk *c, Value v, size_t line);

/**
 * add invoke super name and write chunk
 *
 * @param c
 * @param v
 * @param line
 */
void write_invoke_super(Chunk *c, Value v, size_t line);

/**
  * find value in constant of chunk. return the index of value if found
  *
  * @param c
  * @param target
  * @param found
  * @return
  */
size_t find_value_in_constant(Chunk *c, Value target, bool *found);

/**
 * get the line number of the instruction with given offset.
 *
 * @param c
 * @param offset
 * @return
 */
size_t get_line(Chunk *c, size_t offset);

#endif //ZHI_CHUNK_CHUNK_H_
