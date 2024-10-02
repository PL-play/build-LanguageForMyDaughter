//
// Created by ran on 2024/6/27.
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "vm/vm.h"
#include "common/framework.h"

static void test_require() {
  char *source = "want \"./duoduolib/lib.duo\" as a;\n"
                 "puffln( a.global);\n" // expect "BABA love DuoDuo"
                 "waa f = a.f1();\n"
                 "f();\n" // expect "MAMA DuoDuo"
                 "puffln( a.global);\n" // expect "DuoDuo"
                 "waa ca = a.A(\"BABA\");\n" // new instance with name
                 "puffln( ca.name);\n"// expect "BABA"
                 "puffln( ca);\n" // expect "<A:instance>"
                 "ca.methodA1();\n" // change this.name to global
                 "puffln( ca.name);\n" // expect "DuoDuo"
                 "puffln( a.ca);\n" // expect "<class:A>"
                 "waa cb = a.B();\n" //
                 "puffln( \"----\");\n"
                 "cb.methodA1(\"LALA\");"// call super.methodA1 will change cb.name to global, which is "DuoDuo", then change back to "LALA"
                 "puffln( cb.name);\n"// expect "LALA"
                 "cb.methodB1();\n" // will print "DuoDuo" then change name to "BABA"
                 "puffln( cb.name);\n" // expect "BABA"
                 "a.B.methodB2(6);\n"// expect "out of range"
                 "a.changeGlobal(\"Other\");\n"
                 "waa bb = a.cb.methodB2(1);\n" // will print "Other"
                 "puffln( bb.name);\n" // will print "0 love"
                 "bb = a.cb.methodB2(2);\n"
                 "puffln( bb.name);\n" // will print "Other" and "BABA"
                 // "a = 1;" // runtime error : Can't set module variable.
                 "a.getlib2G();\n" // expect "love"
                 ""
                 ""
                 "";//
  VM vm;
  init_VM(&vm, NULL, NULL, true);
  InterpretResult result = interpret(&vm, NULL, source);
  assert(result == INTERPRET_OK);
#ifdef DEBUG_TRACE_EXECUTION
  int i = vm.compile_context.global_values->size;
  for (int j = 0; j < i; ++j) {
    printf("global[%d] ", j);
    print_value(vm.compile_context.global_values->data[j]);
    printf("\n");
  }
#endif
  free_VM(&vm);
}

static void test_require2() {
  char *source = "want \"./duoduolib/lib3.duo\" as a;\n"
                 ""
                 ""
                 "";//
  VM vm;
  init_VM(&vm, NULL, NULL, true);
  InterpretResult result = interpret(&vm, NULL, source);
  assert(result == INTERPRET_OK);
  free_VM(&vm);
}

/**
 * libc1 -> libc2 -> libc3 -> lib -> lib2
 */
static void test_dependency() {
  // libc1.duo and libc2.duo is circular dependent
  // exit with code 70
  char *source = "want \"./duoduolib/libc1.duo\" as a;\n"
                 ""
                 ""
                 "";//
  VM vm;
  init_VM(&vm, NULL, NULL, true);
  InterpretResult result = interpret(&vm, NULL, source);
  assert(result == INTERPRET_OK);
  free_VM(&vm);
}

static void test_circular_dependency() {
  // libc1.duo and libc2.duo is circular dependent
  // exit with code 70
  char *source = "want \"./duoduolib/circular/c1.duo\" as a;\n"
                 ""
                 ""
                 "";//
  VM vm;
  init_VM(&vm, NULL, NULL, true);
  InterpretResult result = interpret(&vm, NULL, source);
  assert(result == INTERPRET_RUNTIME_ERROR);
  free_VM(&vm);
}

static UnitTestFunction tests[] = {
    test_require,
    test_require2,
    test_dependency,
    // test_circular_dependency,
    NULL
};

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  run_tests(tests);
  return 0;
}