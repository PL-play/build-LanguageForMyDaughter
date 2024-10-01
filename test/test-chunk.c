//
// Created by ran on 2024-03-13.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "chunk/chunk.h"
#include "chunk/value.h"
#include "chunk/debug.h"
#include "common/framework.h"
#include <string.h>

static void test_chunk_init() {
  Chunk *chunk = init_chunk();
  for (int i = 0; i < 10; ++i) {
    add_chunk(chunk, OP_CONSTANT, i);
    assert(chunk->code->size == i + 1);
  }

  for (int i = 0; i < 300; ++i) {
    add_chunk(chunk, OP_CONSTANT, i);
    size_t index = add_constant(chunk, NUMBER_VAL(i));
    assert(i == index);
    assert(chunk->constants->size == i + 1);
  }
  free_chunk(chunk);
}

static void test_disassemble() {
  Chunk *chunk = init_chunk();
  for (int i = 0; i < 10; ++i) {
    add_chunk(chunk, OP_RETURN, 1);
    assert(chunk->code->size == i + 1);
  }

  disassemble_chunk(chunk, "test");
  free_chunk(chunk);

}

static void test_init_values() {
  Chunk *chunk = init_chunk();
  size_t constant_index = add_constant(chunk, NUMBER_VAL(3.14));
  assert(constant_index == 0);
  add_chunk(chunk, OP_CONSTANT, 1);
  add_chunk(chunk, constant_index, 1);
  add_chunk(chunk, OP_RETURN, 2);
  disassemble_chunk(chunk, "test");
  free_chunk(chunk);
}

static void test_long_constant() {
  Chunk *chunk = init_chunk();
  for (int i = 0; i < 257; ++i) {
    write_constant(chunk, NUMBER_VAL(i), i);
  }
  disassemble_chunk(chunk, "constant long");
  assert(chunk->code->data[chunk->code->size - 4] == OP_CONSTANT_LONG);
  uint32_t constant = chunk->code->data[chunk->code->size - 4 + 1] |
      (chunk->code->data[chunk->code->size - 4 + 2] << 8) |
      (chunk->code->data[chunk->code->size - 4 + 3] << 16);
  assert(constant == 256);
  free_chunk(chunk);
}

static void test_opcodes() {
  Chunk *chunk = init_chunk();
  for (int i = 0; i < 257; ++i) {
    switch (i % 7) {
      case 1:
      case 2:write_constant(chunk, NUMBER_VAL(i), i);
        continue;
      default:add_chunk(chunk, i % 7, i);
        continue;
    }
  }
  disassemble_chunk(chunk, "opcodes");
  free_chunk(chunk);
}

static UnitTestFunction tests[] = {
    test_chunk_init,
    test_disassemble,
    test_init_values,
    test_long_constant,
    test_opcodes,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);
  return 0;
}
