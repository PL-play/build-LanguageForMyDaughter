//
// Created by ran on 2024-03-13.
//
#include "chunk.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef CHUNK_INIT_SIZE
#define CHUNK_INIT_SIZE 256
#endif

#ifndef LINEINFO_INIT_SIZE
#define LINEINFO_INIT_SIZE 256
#endif

#ifdef ALLOC_TESTING
#include "common/alloc-testing.h"
#endif

DEFINE_ARRAY_LIST(OP_NULL, Uint8t_)

/**
 * push to chunk with given size, return the pointer of the start of the pushed size.
 *
 * @param c
 * @param size
 * @return
 */
static void update_lines(Chunk *c, size_t line);
static void free_line_info(LineInfo *line_info);
static void init_line_info(LineInfo *line_info);
static size_t find_exist_value(Chunk *c, Value target);

Chunk *init_chunk() {
  Chunk *chunk = malloc(sizeof(Chunk));

  chunk->code = Uint8t_new_arraylist(32);
//  init_value_array(&chunk->constants);
  chunk->constants = Valuenew_arraylist(10);
  init_line_info(&chunk->line_info);

  return chunk;
}
void add_chunk(Chunk *c, uint8_t byte, size_t line) {
  Uint8t_append_arraylist(c->code, byte);
  update_lines(c, line);
}
uint8_t get_code_of(Chunk *chunk, size_t index) {
  return Uint8t_get_data_arraylist(chunk->code, index);
}
size_t get_code_size(Chunk *chunk) {
  return chunk->code->size;
}
static void update_lines(Chunk *c, size_t line) {
  if (c->line_info.count > 0 &&
      c->line_info.lines[c->line_info.count - 1].line == line) {
    // still on the same line.
    return;
  }
  //append new line start
  GROW_ARRAY(&c->line_info, capacity, count, lines, LINEINFO_INIT_SIZE, LineStart);
  LineStart *lineStart = &c->line_info.lines[c->line_info.count++];
  lineStart->offset = c->code->size - 1;
  lineStart->line = line;
}

void free_chunk(Chunk *chunk) {
  assert(chunk != NULL);
  //free_value_array(&chunk->constants);
  Valuefree_arraylist(chunk->constants);
  free_line_info(&chunk->line_info);
  Uint8t_free_arraylist(chunk->code);
  free(chunk);
}

size_t add_constant(Chunk *c, Value value) {
  assert(c != NULL);
//  add_value_array(&c->constants, value);
  Valueappend_arraylist(c->constants, value);
  return c->constants->size - 1;
}

static size_t add_constant_and_write(Chunk *c, Value v, size_t line, uint8_t op) {
  bool found = false;
  size_t i = find_value_in_constant(c, v, &found);
  size_t index = found ? i : add_constant(c, v);
  if (index < 256) {
    add_chunk(c, op, line);
    add_chunk(c, (uint8_t) index, line);
  } else {
    add_chunk(c, op + 1, line);
    // little-endian
    add_chunk(c, (uint8_t) (index & 0xff), line);
    add_chunk(c, (uint8_t) ((index >> 8) & 0xff), line);
    add_chunk(c, (uint8_t) ((index >> 16) & 0xff), line);
  }
  return index;
}

void write_constant(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  add_constant_and_write(c, v, line, OP_CONSTANT);
}

void write_closure(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  add_constant_and_write(c, v, line, OP_CLOSURE);
}

size_t write_class(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  return add_constant_and_write(c, v, line, OP_CLASS);
}

size_t find_value_in_constant(Chunk *c, Value target, bool *found) {
  for (size_t i = 0; i < c->constants->size; ++i) {
    Value v = Valueget_data_arraylist(c->constants, i);
    if (value_equals(v, target)) {
      *found = true;
      return i;
    }
  }
  *found = false;
  return 0;
}

void write_get_prop(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  add_constant_and_write(c, v, line, OP_GET_PROP);
}

void write_get_super_prop(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  add_constant_and_write(c, v, line, OP_GET_SUPER_PROP);
}

void write_set_prop(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  add_constant_and_write(c, v, line, OP_SET_PROP);
}

void write_invoke(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  add_constant_and_write(c, v, line, OP_INVOKE);
}

void write_invoke_super(Chunk *c, Value v, size_t line) {
  assert(c != NULL);
  add_constant_and_write(c, v, line, OP_INVOKE_SUPER);
}

static void free_line_info(LineInfo *line_info) {
  free(line_info->lines);
  init_line_info(line_info);
}

static void init_line_info(LineInfo *line_info) {
  line_info->capacity = 0;
  line_info->count = 0;
  line_info->lines = NULL;
}

size_t get_line(Chunk *c, size_t offset) {
  assert(c != NULL);
  assert(c->line_info.count > 0);
  assert(offset < get_code_size(c));
  size_t start = 0;
  size_t end = c->line_info.count - 1;

  for (;;) {
    size_t mid = (start + end) / 2;
    LineStart *line = &c->line_info.lines[mid];
    if (offset < line->offset) {
      end = mid - 1;
    } else if (mid == c->line_info.count - 1 ||
        offset < c->line_info.lines[mid + 1].offset) {
      return line->line;
    } else {
      start = mid + 1;
    }
  }
}