//
// Created by ran on 24-10-2.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "vm/vm.h"
#include "common/framework.h"

static void test_str() {
    char *es[] = {
        "puff __str;",
        "puff __str(1);",
        "puff __str(1.234);",
        "puff __str(1+1);",
        "puff __str(aow);",
        "puff __str(emm);",
        "puff __str(__clock);",
        "puff __str(shadow(a,b){puff a+b; home a+b;});",
        "castle A{}\n"
        "puff __str(A);",
        "puff __str(A());",
        "puff __str(1>2?1:-1);",
        "puff __str(\"Baba\")+ __str(\"\\n\")+__str(\"love \n\")+__str(\"duoduo!\");",
        "puff __str([1,\"duoduo\",3]);",
        "puff __str([]);",
        "puff __str([aow,emm,3.14,3+5,__clock,shadow(a,b){home a+b;}(1,41)]);",
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for test native __str: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static UnitTestFunction tests[] = {
    test_str,
    NULL
};

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    run_tests(tests);
    return 0;
}
