//
// Created by ran on 24-4-24.
//
#include <setjmp.h>
#include <stdio.h>
#include "common/framework.h"
#include <stdlib.h>
#include <string.h>

void fun1(jmp_buf *caller_context);
void fun2(jmp_buf *caller_context);
void fun3(jmp_buf *caller_context);

void fun1(jmp_buf *caller_context) {
  jmp_buf context;
  if (setjmp(context) == 0) {
    printf("  -fun1\n");
    fun2(&context);
    printf("  -return fun1\n");
  } else {
    printf("  -catch fun1\n");
    longjmp(*caller_context, 1); // Propagate error to caller
  }
}

void fun2(jmp_buf *caller_context) {
  jmp_buf context;
  if (setjmp(context) == 0) {
    printf("    -fun2\n");
    fun3(&context);
    printf("    -return fun2\n");
  } else {
    printf("    -catch fun2\n");
    longjmp(*caller_context, 1); // Propagate error to caller
  }
}

void fun3(jmp_buf *caller_context) {
  jmp_buf context;
  if (setjmp(context) == 0) {
    printf("      -fun3\n");
    longjmp(context, 1); // Trigger an error
    printf("return fun3\n");
  } else {
    printf("      -catch fun3\n");
    longjmp(*caller_context, 1); // Propagate error to caller
  }
}


static void test_local_jump(){
  jmp_buf context;
  if (setjmp(context) == 0) {
    printf("-main\n");
    fun1(&context);
  } else {
    printf("-catch main\n");
  }
}



static UnitTestFunction tests[] = {
    test_local_jump,
    NULL
};

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  run_tests(tests);
  return 0;
}