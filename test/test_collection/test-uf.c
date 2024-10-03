//
// Created by ran on 2024/1/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <threads.h>

#include "common/framework.h"
#include "unionfind/union_find.h"

void test_uf_create() {
  int size = 100;
  UnionFind *uf = create_uf(size);
  assert(uf_size(uf) == 100);
  for (int i = 0; i < size - 1; ++i) {
    assert(uf_same_set(uf, i, i + 1) == 0);
  }

  for (int i = 0; i < 50; ++i) {
    uf_union(uf, i, size - i - 1);
    assert(uf_same_set(uf, i, size - i - 1) == 1);
  }

  free_uf(uf);
}

static UnitTestFunction tests[] = {
    test_uf_create,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);
  return 0;
}
