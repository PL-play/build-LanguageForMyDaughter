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

static void test_array() {
  char *es[] = {
    "want \"./duoduolib/libarr.duo\" as ll;\n",
    //      "__len(1);", //throw error
    "print [1,2,3];",
    "print [];",
    "print __len([]);",
    "waa arr = [1,2,3];\n"
    "print __len(arr);\n",
    "print [1,2,3][1];\n",
    "ll.printv(1);\n",
    "arr[1] = 100;\n"
    "ll.printv(arr);\n"
    "arr[-1] =3.14;\n"
    "ll.printv(arr);\n",
    "arr = [0,1,2,3,4,5,6,7,8,9];\n"
    "ll.printv(arr);\n",
    "ll.printv(arr[1:]);\n",
    "ll.printv(arr[1:4]);\n",
    "ll.printv(arr[:]);\n",
    "ll.printv(arr[:0]);\n",
    "ll.printv(arr[-10:]);\n",
    "print(arr[-10:][2]=5);\n",
    "waa a1 = [[1,2,3],[4,5,6]];\n"
    "print __type(a1);\n",
    "print __array_type;\n",
    "print __type(a1)== __array_type;\n",
    "ll.printv(a1);",
    "ll.printv(a1[1:2][0][1:2]);",
    "ll.printv ([[1,2,3,[\"a\",\"b\",\"c\"]],[4,5,6]]);",
    NULL
  };
  VM vm;
  init_VM(&vm, NULL,NULL, true);

  for (int i = 0; es[i] != NULL; ++i) {
    char *source = es[i];

    printf("++++++++++++\n interpret for test string: \n\n%s \n+++++++++++\n", source);
    printf("\n------ result ------\n");
    InterpretResult result = interpret(&vm, NULL, source);
    assert(result == INTERPRET_OK);
    printf("\n\n\n");
  }
  free_VM(&vm);
}

static UnitTestFunction tests[] = {
  test_array,
  NULL
};

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  //  setbuf(stderr, NULL);
  run_tests(tests);
  return 0;
}
