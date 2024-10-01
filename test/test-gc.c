//
// Created by ran on 2024-05-28.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vm/vm.h"
#include "common/framework.h"
#include "vm/gc.h"

// NOTE: must config '-DENABLE_GC_DEBUG=ON -DINIT_GC=128 -DGC_TRIGGER=1' in cmake options to run test cases!
static void test_gc() {
  printf("init gc size: %d, gc call trigger: %d\n", INITIAL_GC_SIZE, GC_CALL_TRIGGER);
  assert(INITIAL_GC_SIZE == 128);
  assert(GC_CALL_TRIGGER == 1);
  printf("test gc!");
}

/**
 * Will print :
 *
 * +++++sweep+++++
    type:0. i love duoduo
    free string:[0x55c2ac9a86b0] - [i love duoduo] - (13)   free string obj[0x55c2ac9a86b0]: i love duoduo
    -----sweep-----
    ++update next gc size
       ********* next gc size: [128] bytes **********
    --update next gc size


    ----------------------------------------
    !!!!!!!!!!GC time: 0.066000 ms!!!!!!!!!!!
    ----------------------------------------
 *
 *
 */
static void test_gc1() {
  printf("---with __GC() call\n");
  char *s = "var global = \"i love \"+\"duoduo\";\n"
            "global = 1;\n"
            "__GC();\n"
            "print global;";
  VM vm;
  init_VM(&vm, NULL, NULL, true);
  InterpretResult result = interpret(&vm, NULL, s);
  assert(result == INTERPRET_OK);
  free_VM(&vm);
  printf("\n");
}

/**
 * Won't print gc logs
 */
static void test_gc2() {
  printf("---NO __GC() call\n");
  char *s = "var global = \"i love \"+\"duoduo\";\n"
            "global = 1;\n"
            //"__GC();\n"
            "print global;";
  VM vm;
  init_VM(&vm, NULL,NULL, true);
  InterpretResult result = interpret(&vm,NULL, s);
  assert(result == INTERPRET_OK);
  free_VM(&vm);
  printf("\n");
}

static UnitTestFunction tests[] = {
    test_gc,
    test_gc1,
    test_gc2,
    NULL
};

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  run_tests(tests);
  return 0;
}