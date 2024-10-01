//
// Created by ran on 2024/7/1.
//
#include "common/framework.h"
#include "common/try.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define EX_FILE_NOT_FOUND 0x0100
static void test_throw1();

static void test1() {
  cx_try {
        cx_try {
              test_throw1();
            } cx_catch(EX_FILE_NOT_FOUND) {
              printf("cx_catch :  %d\n", cx_impl_try_block_head->caught_xid);
//              cx_throw();
              cx_throw(EX_FILE_NOT_FOUND, "user data111");
            } cx_finally {
            printf("f1 :  %s\n", (char *) cx_current_exception()->user_data);
          }
      } cx_catch() {
        cx_exception_t *e = cx_current_exception();
        printf("catch {%d}, user data:%s \n", e->thrown_xid, (char *) e->user_data);
      } cx_finally {
      printf("f2\n");
    }
}

static void do_something_with(FILE *file) {
  // nothing here
  printf("do things with existing file.");
}

static void test_throw1() {
  char *path = "not_exist";
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    cx_throw(EX_FILE_NOT_FOUND);
  }
  cx_try {
        do_something_with(f);
      } cx_finally {
      fclose(f);
    }

}
static void test_throw() {
  char *path = "not_exist";
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    cx_throw(EX_FILE_NOT_FOUND);
  }
}

static void test_catch() {
  cx_try {
        test_throw();
      } cx_catch(EX_FILE_NOT_FOUND) {
        printf("catch file not found exception.\n");
      } cx_finally {
      printf("cx_finally.\n");
    }
}

static void test_catch1() {
  cx_try {
        test_throw();
      } cx_catch(EX_FILE_NOT_FOUND) {
        printf("catch file not found exception.\n");
      } cx_finally {
      printf("cx_finally.\n");
    }
}

static void test_nesting_cx_try() {
  cx_try {
        printf("---cx_try1\n");
        cx_try {
              printf("    ---cx_try2\n");
              cx_try {

                    printf("        ---cx_try3\n");

                    test_throw();
                  } cx_catch() {
                    printf("        ---catch3\n");
                    cx_throw(2);
                  }
            } cx_catch() {
              printf("    ---catch2\n");
              cx_throw(1);
            }
      } cx_catch(1) {
        printf("---catch1\n");
      } cx_catch(EX_FILE_NOT_FOUND) {
        printf("---catch %d \n", EX_FILE_NOT_FOUND);
      } cx_finally {
      printf("cx_try1 cx_finally\n");
    }
}

static UnitTestFunction tests[] = {
    test1,
//    test_throw,
    test_catch,
    test_nesting_cx_try,
    NULL
};

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  run_tests(tests);
  return 0;
}