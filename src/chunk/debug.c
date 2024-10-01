//
// Created by ran on 2024-03-14.
//
#include "chunk/debug.h"
#include <stdlib.h>
#include <stdio.h>

static size_t simple_instruction(const char *name, size_t offset);
static size_t constant_instruction(const char *name, Chunk *chunk, size_t offset);
static size_t constant_long_instruction(const char *name, Chunk *chunk, size_t offset);
static size_t variable_instruction(const char *name, Chunk *chunk, size_t offset);
static size_t variable_instruction_long(const char *name, Chunk *chunk, size_t offset);
static size_t closure_instruction(const char *name, Chunk *chunk, size_t offset);
static size_t closure_long_instruction(const char *name, Chunk *chunk, size_t offset);
static size_t class_instruction(const char *name, Chunk *chunk, size_t offset);
static size_t invoke_instruction(const char *name, Chunk *chunk, size_t offset);
static size_t invoke_instruction_long(const char *name, Chunk *chunk, size_t offset);
static size_t slicing_instruction(const char *name, Chunk *chunk, size_t offset);

void disassemble_chunk(Chunk *chunk, const char *name) {
  printf("-- %s --\n", name);
  size_t offset = 0;
  while (offset < get_code_size(chunk)) {
    offset = disassemble_instruction(chunk, offset);
  }
}

size_t disassemble_instruction(Chunk *chunk, size_t offset) {
  printf("%04zu ", offset);
  size_t line = get_line(chunk, offset);
  if (offset > 0 && line == get_line(chunk, offset - 1)) {
    printf("  -  ");
  } else {
    printf("%4zu ", line);
  }
  uint8_t instruction = get_code_of(chunk, offset);
  switch (instruction) {
    case OP_RETURN: {
      return simple_instruction("OP_RETURN", offset);
    }
    case OP_CONSTANT: {
      return constant_instruction("OP_CONSTANT", chunk, offset);
    }
    case OP_CONSTANT_LONG: {
      return constant_long_instruction("OP_CONSTANT_LONG", chunk, offset);
    }
    case OP_NEGATE: {
      return simple_instruction("OP_NEGATE", offset);
    }
    case OP_ADD: {
      return simple_instruction("OP_ADD", offset);
    }
    case OP_SUBTRACT: {
      return simple_instruction("OP_SUBTRACT", offset);
    }
    case OP_MULTIPLY: {
      return simple_instruction("OP_MULTIPLY", offset);
    }
    case OP_DIVIDE: {
      return simple_instruction("OP_DIVIDE", offset);
    }
    case OP_EXPONENT: {
      return simple_instruction("OP_EXPONENT", offset);
    }
    case OP_FACTORIAL: {
      return simple_instruction("OP_FACTORIAL", offset);
    }
    case OP_NOT: {
      return simple_instruction("OP_NOT", offset);
    }
    case OP_NIL: {
      return simple_instruction("OP_NIL", offset);
    }
    case OP_TRUE: {
      return simple_instruction("OP_TRUE", offset);
    }
    case OP_FALSE: {
      return simple_instruction("OP_FALSE", offset);
    }
    case OP_EQUAL: {
      return simple_instruction("OP_EQUAL", offset);
    }
    case OP_GREATER: {
      return simple_instruction("OP_GREATER", offset);
    }
    case OP_LESS: {
      return simple_instruction("OP_LESS", offset);
    }
    case OP_PRINT: {
      return simple_instruction("OP_PRINT", offset);
    }
    case OP_POP: {
      return simple_instruction("OP_POP", offset);
    }
    case OP_DEFINE_GLOBAL: {
      return variable_instruction("OP_DEFINE_GLOBAL", chunk, offset);
    }
    case OP_DEFINE_GLOBAL_LONG: {
      return variable_instruction_long("OP_DEFINE_GLOBAL_LONG", chunk, offset);
    }
    case OP_GET_GLOBAL: {
      return variable_instruction("OP_GET_GLOBAL", chunk, offset);
    }
    case OP_GET_GLOBAL_LONG: {
      return variable_instruction_long("OP_GET_GLOBAL_LONG", chunk, offset);
    }
    case OP_GET_LOCAL: {
      return variable_instruction("OP_GET_LOCAL", chunk, offset);
    }
    case OP_GET_LOCAL_LONG: {
      return variable_instruction_long("OP_GET_LOCAL_LONG", chunk, offset);
    }
    case OP_SET_GLOBAL: {
      return variable_instruction("OP_SET_GLOBAL", chunk, offset);
    }
    case OP_SET_GLOBAL_LONG: {
      return variable_instruction_long("OP_SET_GLOBAL_LONG", chunk, offset);
    }
    case OP_SET_LOCAL: {
      return variable_instruction("OP_SET_LOCAL", chunk, offset);
    }
    case OP_SET_LOCAL_LONG: {
      return variable_instruction_long("OP_SET_LOCAL_LONG", chunk, offset);
    }
    case OP_JUMP_IF_FALSE: {
      return variable_instruction_long("OP_JUMP_IF_FALSE jump offset", chunk, offset);
    }
    case OP_JUMP: {
      return variable_instruction_long("OP_JUMP jump offset", chunk, offset);
    }
    case OP_JUMP_BACK: {
      return variable_instruction_long("OP_JUMP_BACK jump offset", chunk, offset);
    }
    case OP_MOD: {
      return simple_instruction("OP_MOD", offset);
    }
    case OP_CALL: {
      return variable_instruction("OP_CALL", chunk, offset);
    }
    case OP_CLOSURE: {
      return closure_instruction("OP_CLOSURE", chunk, offset);
    }
    case OP_CLOSURE_LONG: {
      return closure_long_instruction("OP_CLOSURE_LONG", chunk, offset);
    }
    case OP_SET_UPVALUE: {
      return variable_instruction("OP_SET_UPVALUE", chunk, offset);
    }
    case OP_GET_UPVALUE: {
      return variable_instruction("OP_GET_UPVALUE", chunk, offset);
    }
    case OP_CLOSE_UPVALUE: {
      return simple_instruction("OP_CLOSE_UPVALUE", offset);
    }
    case OP_CLASS: {
      return constant_instruction("OP_CLASS", chunk, offset);
    }
    case OP_CLASS_LONG: {
      return constant_long_instruction("OP_CLASS_LONG", chunk, offset);
    }
    case OP_GET_PROP: {
      return constant_instruction("OP_GET_PROP", chunk, offset);
    }
    case OP_GET_PROP_LONG: {
      return constant_long_instruction("OP_GET_PROP_LONG", chunk, offset);
    }
    case OP_GET_SUPER_PROP: {
      return constant_instruction("OP_GET_SUPER_PROP", chunk, offset);
    }
    case OP_GET_SUPER_PROP_LONG: {
      return constant_long_instruction("OP_GET_SUPER_PROP_LONG", chunk, offset);
    }
    case OP_SET_PROP: {
      return constant_instruction("OP_SET_PROP", chunk, offset);
    }
    case OP_SET_PROP_LONG: {
      return constant_long_instruction("OP_SET_PROP_LONG", chunk, offset);
    }
    case OP_METHOD_LOCAL: {
      return variable_instruction("OP_METHOD_LOCAL", chunk, offset);
    }
    case OP_METHOD_LOCAL_LONG: {
      return variable_instruction_long("OP_METHOD_LOCAL_LONG", chunk, offset);
    }
    case OP_METHOD_GLOBAL: {
      return variable_instruction("OP_METHOD_GLOBAL", chunk, offset);
    }
    case OP_METHOD_GLOBAL_LONG: {
      return variable_instruction_long("OP_METHOD_GLOBAL_LONG", chunk, offset);
    }
    case OP_INVOKE: {
      return invoke_instruction("OP_INVOKE", chunk, offset);
    }
    case OP_INVOKE_LONG: {
      return invoke_instruction_long("OP_INVOKE_LONG", chunk, offset);
    }
    case OP_INVOKE_SUPER: {
      return invoke_instruction("OP_INVOKE_SUPER", chunk, offset);
    }
    case OP_INVOKE_SUPER_LONG: {
      return invoke_instruction_long("OP_INVOKE_SUPER_LONG", chunk, offset);
    }
    case OP_INHERIT: {
      return simple_instruction("OP_INHERIT", offset);
    }
    case OP_IMPORT: {
      return variable_instruction("OP_IMPORT", chunk, offset);
    }
    case OP_ARRAY: {
      return variable_instruction_long("OP_ARRAY", chunk, offset);
    }
    case OP_INDEXING: {
      return simple_instruction("OP_INDEXING", offset);
    }
    case OP_ELEMENT_ASSIGN: {
      return simple_instruction("OP_ELEMENT_ASSIGN", offset);
    }
    case OP_SLICING: {
      return slicing_instruction("OP_SLICING", chunk, offset);
    }

    default: {
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
    }
  }
}

static size_t constant_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint8_t constant_index = get_code_of(chunk, offset + 1);
  printf("%-16s %4d '", name, constant_index);
  print_value(Valueget_data_arraylist(chunk->constants, constant_index));
//  print_value(chunk->constants.values[constant_index]);
  printf("'\n");
  return offset + 2;
}
static size_t constant_long_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint32_t constant = get_code_of(chunk, offset + 1) |
      (get_code_of(chunk, offset + 2) << 8) |
      (get_code_of(chunk, offset + 3) << 16);
  printf("%-16s %4d '", name, constant);
  print_value(Valueget_data_arraylist(chunk->constants, constant));
  printf("'\n");
  return offset + 4;
}

static size_t class_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint32_t constant = get_code_of(chunk, offset + 1) |
      (get_code_of(chunk, offset + 2) << 8) |
      (get_code_of(chunk, offset + 3) << 16);
  printf("%-16s %4d '", name, constant);
  printf("'\n");
  return offset + 4;
}

static size_t invoke_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint8_t constant_index = get_code_of(chunk, offset + 1);
  uint8_t arg_count = get_code_of(chunk, offset + 2);
  printf("%-16s %4d args '", name, arg_count);
  print_value(Valueget_data_arraylist(chunk->constants, constant_index));
  printf("'\n");
  return offset + 3;
}

static size_t invoke_instruction_long(const char *name, Chunk *chunk, size_t offset) {
  uint32_t constant = get_code_of(chunk, offset + 1) |
      (get_code_of(chunk, offset + 2) << 8) |
      (get_code_of(chunk, offset + 3) << 16);
  uint8_t arg_count = get_code_of(chunk, offset + 4);
  printf("%-16s %4d args '", name, arg_count);
  print_value(Valueget_data_arraylist(chunk->constants, constant));
  printf("'\n");
  return offset + 4;
}

static size_t closure_instruction(const char *name, Chunk *chunk, size_t offset) {
  offset++;
  uint8_t constant_index = get_code_of(chunk, offset++);
  printf("%-16s %4d '", name, constant_index);
  ObjFunction *function = AS_FUNCTION(Valueget_data_arraylist(chunk->constants, constant_index));
  print_object((Obj *) function);
  printf("'\n");
  for (int i = 0; i < function->upvalue_count; ++i) {
    int is_local = chunk->code->data[offset++];
    int index = chunk->code->data[offset++];
    printf("%04zu   |      %s %d\n", offset - 2, is_local ? "local" : "upvalue", index);
  }
  return offset;
}

static size_t closure_long_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint32_t constant_index = get_code_of(chunk, offset + 1) |
      (get_code_of(chunk, offset + 2) << 8) |
      (get_code_of(chunk, offset + 3) << 16);
  printf("%-16s %4d '", name, constant_index);
  offset += 4;
  ObjFunction *function = AS_FUNCTION(Valueget_data_arraylist(chunk->constants, constant_index));
  print_object((Obj *) function);
  printf("'\n");
  for (int i = 0; i < function->upvalue_count; ++i) {
    int is_local = chunk->code->data[offset++];
    int index = chunk->code->data[offset++];
    printf("%04zu   |    %s %d\n", offset - 2, is_local ? "local" : "upvalue", index);
  }
  return offset;
}

static size_t variable_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint8_t constant_index = get_code_of(chunk, offset + 1);
  printf("%-16s %4d '", name, constant_index);
  printf("'\n");
  return offset + 2;
}

static size_t variable_instruction_long(const char *name, Chunk *chunk, size_t offset) {
  uint32_t constant = get_code_of(chunk, offset + 1) |
      (get_code_of(chunk, offset + 2) << 8) |
      (get_code_of(chunk, offset + 3) << 16);

  printf("'\n");
  return offset + 4;
}

static size_t simple_instruction(const char *name, size_t offset) {
  printf("%s\n", name);
  return offset + 1;
}

static size_t slicing_instruction(const char *name, Chunk *chunk, size_t offset) {
  uint8_t index0 = get_code_of(chunk, offset + 1);
  uint8_t index1 = get_code_of(chunk, offset + 2);
  printf("%-16s %4d %4d\n", name, index0, index1);
  return offset + 3;
}