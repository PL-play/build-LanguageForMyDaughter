//
// Created by ran on 24-3-15.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "vm/vm.h"
#include "common/framework.h"

static void test_computed_goto() {
#ifdef __GNUC__
    printf("This code is compiled with GCC.\n");
    void *jumpTable[] = {&&label1, &&label2};
    int idx = 0; // Change this as needed to test different jumps

    goto
    *jumpTable[idx];

label1:
    printf("Jumped to label1\n");
    return;

label2:
    printf("Jumped to label2\n");
    return;
#else
  printf("This code is not compiled with GCC.\n");
#endif
}

static char *exps[] = {
    "1;",
    "puff 1;",
    "1+3*3;",
    "puff 1+3*3;",
    "(1-2)/3;",
    "puff (1-2)/3;",
    "-1-2;",
    "puff -1-2;",
    "(2-3)*4/3;",
    "puff (2-3)*4/3;",
    "puff 1.234*3.14-1--2;",
    "23.234-34*(123+-5*343)/23-23;",
    "puff 23.234-34*(123+-5*343)/23-23;",
    "23.234--2353.39;",
    "puff 23.234--2353.39;",
    "(-1 + 2) * 3 - -4;",
    "puff (-1 + 2) * 3 - -4;",
    "-3!+4^3;",
    "puff -3!+4^3;",
    "aow;",
    "puff aow;",
    "!aow;",
    "puff !aow;",
    "nil;",
    "puff nil;",
    "!0;",
    "puff !0;",
    "!1;",
    "puff !1;",
    "!nil;",
    "puff !nil;",
    "1>2;",
    "puff 1>2;",
    "2>=2;",
    "puff 2>=2;",
    "1<2;",
    "puff 1<2;",
    "1<=2;",
    "puff 1<=2;",
    "2<=2;",
    "puff 2<=2;",
    "3>2;",
    "puff 3>2;",
    "!(6 - 4 > 3 * 4 == !nil);",
    "puff !(6 - 4 > 3 * 4 == !nil);",
    "puff 1;",
    "puff \"12345\";",
    "puff \"2\";",
    "\"aaa\"==\"aaa\";",
    "puff \"aaa\"==\"aaa\";",
    "\"aab\"==\"aaa\";",
    "puff \"aab\"==\"aaa\";",
    "puff \"aaa\"+\"bbb\"+\"123\";",
    "puff 1+2;",
    NULL
};

static void test_vm() {
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; exps[i] != NULL; ++i) {
        char *source = exps[i];

        printf("++++++++++++\n interpret expression: %s \n+++++++++++\n", source);
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }

    free_VM(&vm);
}

static void test_global_variable() {
    char *es[] = {
        "waa a=1;puff a=1;",
        "waa a;\npuff a;",
        "waa a=1;\npuff a;",
        "waa a=1;\npuff a; \nwaa a=2; waa b =3;\npuff a+b;",
        "waa i = \"baba\"; waa j = \"mama\"; waa q = \"duoduo\";\n puff i+\" love \"+j +\" love \"+ q;",
        "waa i=3;waa j=i;puff i; puff j; i=6; puff i; puff j;",
        "waa aa=3; aa=6; puff aa;",
        "aa=7; puff aa;",
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret global_variable: %s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }

    free_VM(&vm);
}

static void test_local_variable() {
    char *es[] = {
        "waa a=1;{ waa a=2; puff a;} puff a; a=3; puff a;",
        "waa a=1;{ waa b=2; puff a;} puff a;",
        "waa a=1;{  waa b=a; puff b;} waa b=4;puff b;",
        //      "{waa a=1;waa a=2;}",
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret local_variable: %s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }

    free_VM(&vm);
}

static void test_local_variable_long() {
    char *es[] = {
        "waa a=1;{ waa a=2; puff a;} puff a; a=3; puff a;",
        "waa a=1;{ waa b=2; puff a;} puff a;",
        "waa a=1;{  waa b=a; puff b;} waa b=4;puff b;",
        //      "{waa a=1;waa a=2;}",
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);
    for (int i = 0; i < 257; ++i) {
        char a[27];
        sprintf(a, "{waa v%-3d=%-3d;puff v%-3d;}", i, i, i);
        printf("%s\n", a);
        InterpretResult result = interpret(&vm, NULL, a);
        assert(result == INTERPRET_OK);
    }

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret expression: %s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }

    free_VM(&vm);
}

static void test_global_variable_long() {
    VM vm;
    init_VM(&vm, NULL,NULL, true);
    for (int i = 0; i < 257; ++i) {
        char a[100];
        sprintf(a, "waa v%-3d=%-3d;puff v%-3d+\"\\n\";", i, i, i);
        printf("%s\n", a);
        InterpretResult result = interpret(&vm, NULL, a);
        assert(result == INTERPRET_OK);
    }

    for (int i = 0; i < 10; ++i) {
        char a[100];
        sprintf(a, "waa t%-2d=%-2d; t%-2d=%d;\npuff t%-2d+\"\\n\";", i, i, i, i * i, i);
        printf("%s\n", a);
        printf("++++++++++++\n interpret expression: %s \n+++++++++++\n", a);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, a);
        assert(result == INTERPRET_OK);
    }

    free_VM(&vm);
}

static void test_wish_statement() {
    VM vm;
    init_VM(&vm, NULL,NULL, true);
    char *es[] = {
        "waa a=1;wish(a>=1){ puff a;} puff 3;",
        "wish(1)  puff 2;",
        "waa a =5; wish(a>3){"
        "   puff a;"
        "   waa b = 5;"
        "   wish(a>b) puff \"duoduo\"; "
        "     dream puff \" love\";"
        "} dream {"
        "   puff \"baba\";"
        "}",
        "waa a = 4; wish(a>8) {puff 8;} dream wish(a>6){puff 6;} dream {puff a;}",
        "waa a = 9; wish(a>8) {puff 8;} dream wish(a>6){puff 6;} dream {puff a;}",
        "waa a = 8; wish(a>8) {puff 8;} dream wish(a>6){puff 6;} dream {puff a;}",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret wish statements: %s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_wloop_statement() {
    VM vm;
    init_VM(&vm, NULL,NULL, true);
    char *es[] = {
        "waa a=5; \n"
        "wloop(a>=0){\n"
        "  a = a-1;\n"
        "  puff a && (a-1) || \"i love duoduo\";\n"
        " puff \"\\n\";"
        "}\n",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret while statements: %s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_loop_statement() {
    VM vm;
    init_VM(&vm,NULL, NULL, true);
    char *es[] = {
        "loop(waa a=0;a<5;a=a+1){\n"
        "    puff \"duoduo cool \";\n"
        "}",

        "waa b;\n"
        "loop(b=5;b>0;b=b-1){\n"
        "  puff b;\n"
        "  puff \"mama love duoduo \";\n"
        "}",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret while statements: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_skip() {
    VM vm;
    init_VM(&vm, NULL,NULL, true);
    char *es[] = {
        "puff 1 + 1+nil+emm+aow;\n"
        "puff \"\\n\";\n",

        "puff \"\" + 1 + 1;\n"
        "puff \"\\n\";\n",
        "waa a=1; puff \"\" + a + a;\n"
        "puff \"\\n\";\n",
        "loop(waa a=0;a<5;a=a+1){\n"
        "   wish(a==3) skip;  \n"
        "   puff \"a=\"+(a+1);\n"
        "   puff \"\\n\";\n"
        "}",
        "loop(waa a =0;a<5;a=a+1){\n"
        "  loop(waa b=0;b<5; b=b+1){\n"
        "    wish(a+b==7) skip;\n"
        "    puff \"\"+ a +\"+\"+ b+\"=\"+(a+b);\n"
        "    puff \"\\n\";\n"
        "  }\n"
        "  puff \"\";"
        "}\n",
        "waa b;\n"
        "loop(b=5;b>0;b=b-1){\n"
        "  wish(b%2==0) skip;\n"
        "  puff b;\n"
        "  puff \"\\n\";\n"
        "  puff \"mama love duoduo\";\n"
        "  puff \"\\n\";\n"
        "}",
        "waa a=5; \n"
        "wloop(a>=0){\n"
        "  a = a-1;\n"
        "  wish(a%2==0) skip;"
        "  puff a && (a-1) || \"i love duoduo\";\n"
        "  puff \"\\n\";\n"
        "}\n",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for continue: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_equals() {
    VM vm;
    init_VM(&vm,NULL, NULL, true);
    char *es[] = {
        ""
        "waa b1 = emm;\n"
        "waa b2 = emm;\n"
        "puff \"b1==b2 : \" + (b1==b2) +\"\\n\";\n"
        "b2=aow;"
        "puff \"b1==b2 : \" + (b1==b2)+\"\\n\";\n"
        "waa s1 = \"duoduo\";\n"
        "waa s2 = \"duo\"+\"duo\";\n"
        "puff \"s1==s2 : \" + (s1==s2)+\"\\n\";\n"
        "",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for equals: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_break() {
    VM vm;
    init_VM(&vm,NULL, NULL, true);
    char *es[] = {
        "loop(waa a=0;a<5;a=a+1){\n"
        "   wish(a==3) break;  \n"
        "   wish(a%2==0) skip;"
        "   puff a;\n"
        "}\n"
        "puff \"end\\n\";",
        "waa a=5; \n"
        "wloop(a>=0){\n"
        "  a = a-1;\n"
        "  wish(a==0) break;\n"
        "  wish(a%2==0) skip;\n"
        "  puff \"i love duoduo\"+a+\"\\n\";\n"
        "}\n",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for statements: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_magic() {
    VM vm;
    init_VM(&vm, NULL,NULL, true);
    char *es[] = {
        "magic f1(a,b){\n"
        "      home a+b;\n"
        "   }\n"
        "puff f1+\"\\n\";"
        "puff f1(2,4);",
        "magic f1(a){\n"
        "   wish (a<=1) home 1;\n"
        "   home a*f1(a-1);\n"
        "}\n"
        "puff f1(7)+f1(2);",
        "{\n"
        "     magic f2(a,b){home a+b;}\n"
        "     puff f2(4,5);\n"
        "}",
        "shadow(a,b){puff a+b;}(1,\"234\");\n",
        "waa ff = shadow(a,b){puff a+b; home a+b;};\n"
        "ff(\"i love\", ff(\" mama ,\", \"duoduo\"));",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for magic: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_magic2() {
    VM vm;
    init_VM(&vm, NULL,NULL, true);
    char *es[] = {
        "magic f1(a,b){\n"
        "      home a(b);\n"
        "   }\n"
        "puff f1(shadow(a){home a+a;}, \"duo\");",
        "magic f1(a){\n"
        "   wish (a<2) home a;\n"
        "   home f1(a-2)+f1(a-1);\n"
        "}\n"
        "waa start = __clock();\n"
        "puff f1(5);\n"
        "puff __clock() - start;\n",
        "puff __clock();",
        "magic f1(a,b){\n"
        "      home a+b;\n"
        "   }\n"
        "puff f1(1,4);\n",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for function: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_charm() {
    VM vm;
    init_VM(&vm,NULL, NULL, true);
    char *es[] = {
        "magic outer(){\n"
        "  waa x = \"duoduo\";\n"
        "  magic inner(){\n"
        "    x = \"I love \" + x;\n"
        "    puff x;\n"
        "  }\n"
        "  inner();\n"
        "}\n"
        "outer();\n",
        "{\n"
        "   waa x = \"outside\";\n"
        "   magic inner() {\n"
        "    x = \"inside\";\n"
        "     puff x;\n"
        "   }\n"
        "   inner();\n"
        " }\n",
        "\n"
        "magic outer(){\n"
        "  waa x = \"duoduo\";\n"
        "  magic inner(){\n"
        "    x = \"baba love \"+x;"
        "    puff x;\n"
        "  }\n"
        "  home inner;\n"
        "}\n"
        "waa closure = outer();\n"
        "puff closure+\"\\n\";\n"
        "closure();"
        "",
        "\n"
        "magic outer(){\n"
        "  waa x = \"duoduocool\";\n"
        "  magic middle(){\n"
        "    magic inner(){\n"
        "       puff x;\n"
        "    }"
        "    puff \"create inner closure\";\n"
        "    home inner;\n"
        "  }\n"
        "  puff \"return from outer\";\n"
        "  home middle;\n"
        "}\n"
        "waa mid = outer();\n"
        "waa in = mid();\n"
        "in();\n",
        "{\n"
        "   waa y = \"outside\";\n"
        "   magic inner() {\n"
        "     puff y;\n"
        "   }\n"
        "   inner();\n"
        " }\n",
        NULL
    };
    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for charm: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_castle() {
    char *es[] = {
        "castle A{}\n"
        "puff A;\n"
        "{\n"
        "  castle B{}\n"
        "  puff A;\n"
        "  puff B;\n"
        "  waa a = A;\n"
        "  puff \"a=\"+a;\n"
        "   a = B;\n"
        "  puff \"a=\"+a;\n"
        "}\n"
        "waa a =A;\n"
        "puff \"a=\"+a;\n"
        "",
        "magic f1(){\n"
        "  castle Duoduo{}\n"
        "  puff Duoduo;\n"
        "  magic inf1(){\n"
        "    puff \"inf1:\"+Duoduo;\n"
        "    home Duoduo;\n"
        "  }\n"
        "  home inf1;\n"
        "}\n"
        "waa f = f1();\n"
        "waa c = f();\n"
        "puff c;\n",
        "puff A();\n"
        "puff c();\n"
        "waa ic = c();\n"
        "puff \"ic=\"+ic;\n",
        NULL
    };
    VM vm;
    init_VM(&vm,NULL, NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for test class: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_property() {
    char *es[] = {
        "castle A{}\n"
        "waa a = A();\n"
        "a.f1=3;\n"
        "a.f2 = 8;\n"
        "puff a.f1;\n"
        "puff a.f2;\n"
        "puff a.f1+a.f2;\n"
        "puff __has_field(a,\"f1\");\n"
        "__del_field(a,\"f1\");\n"
        "puff __has_field(a,\"f1\");\n",
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for test property: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_method() {
    char *es[] = {
        "castle A{\n"
        "    init(name){\n"
        "       this.name = name;\n"
        "       puffln(\"set name in init(): \"+this.name);\n"
        "    }\n"
        "    m1(){\n"
        "       puffln(\"get name in m1():\"+ this.name);\n"
        "    }\n"
        "    m2(name){\n"
        "      this.name = name;\n"
        "      puffln(\"call m1() inside m2()\");\n"
        "      this.m1();\n"
        "      puffln( \"set name in m2(): \"+this.name);\n"
        "   }\n"
        "   m3(p){\n"
        "      magic f(){"
        "        puffln( this.name+\" \"+ p);"
        "      }\n"
        "      home f;"
        "   }\n"
        "}\n"
        ""
        "castle B{\n"
        "  init(){\n"
        "     this.name = \"MAMA\";"
        "     puffln( \"init B\");\n"
        "  }\n"
        "}\n"
        "puffln( \"call init\");\n"
        "waa a = A(\"baba and \");\n",
        "puffln(\"get name in global:\"+ a.name);\n"
        "puffln( \"call m1\");\n",
        "a.m1();\n",
        "puffln( \"call m2\");\n"
        "a.m2(\"duoduo\");\n"
        "puffln( \"call m1\");\n"
        "waa m1 = a.m1;\n"
        "m1();"
        "puffln( \"get name in global call:\"+ a.name);\n"
        "puffln( \"call m3\");\n"
        "waa f = a.m3(\"baba mama\");\n"
        "f();\n"
        "a.b = B;\n"
        "waa b = a.b();\n"
        "puffln( \"b name: \"+ b.name);\n"
        "a.c = shadow(x){puff x.name;};\n"
        "puffln( \"call a.c()\");\n"
        "a.c(b);\n",
        NULL
    };
    VM vm;
    init_VM(&vm,NULL, NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for test method: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_super() {
    char *es[] = {
        "castle A{\n"
        "    m1(){\n"
        "       puffln( \"A m1()\");\n"
        "    }\n"
        "    m2(){\n"
        "       puffln( \"m2\");\n"
        "    }\n"
        "}\n"
        ""
        "castle B < A{\n"
        "  init(){\n"
        "    hero.m1();\n"
        "    waa m = hero.m1;\n"
        "    m();\n"
        "  }\n"
        "  m1(){\n"
        "    puffln(\"B m1\");\n"
        "  }\n"
        "}\n"
        "waa b = B();\n"
        "b.m2();\n"
        "b.m1();\n"
        "puffln(__has_method(b,\"m1\"));\n"
        "puffln(__has_method(b,\"m2\"));\n"
        "puffln(__has_method(A,\"m2\"));\n"
        "puffln(__has_method(B,\"m2\"));\n"
        "puffln(__has_method(A,\"init\"));\n",
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for test super: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_static_method() {
    char *es[] = {
        "castle A{\n"
        "    static m1(){\n"
        "       puffln( \"A static m1()\");\n"
        "    }\n"
        "    m2(){\n"
        "       puffln( \"A m2\");\n"
        "       A.m1();\n"
        "    }\n"
        "}\n"
        "castle B<A{\n"
        "  m3(){\n"
        "     puffln(  \"b call super methods: \");\n"
        "     hero.m1();\n"
        "     hero.m2();\n"
        "  }\n"
        ""
        "}\n"
        ""
        "A.m1();\n"
        "waa a = A();\n"
        "a.m1();\n"
        "waa m1 = A.m1;\n"
        "m1();\n"
        "puffln(\"call m2: \");\n"
        "a.m2();\n"
        "waa b = B();\n"
        "b.m3();\n",
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for test static methods: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_condition() {
    char *es[] = {
        "waa i = 3;\n"
        "waa j = i>2?1:-1;\n"
        "puff j;\n", // expect 1
        "waa age = 22;\n"
        "waa income = 45000;\n"
        "waa creditScore = 700;\n"
        "waa completedFinancialCourse = aow;\n"
        "// Nested ternary conditional expression\n"
        "waa eligibility = (age >= 18 && age <= 60) ?\n"
        "    (income >= 30000 && income <= 90000) ?\n"
        "        (creditScore >= 650) ?\n"
        "            (age < 25 ? (completedFinancialCourse ? \"Qualifies\" : \"Does not qualify\") : \"Qualifies\")\n"
        "        : \"Does not qualify\"\n"
        "    : \"Does not qualify\"\n"
        ": \"Does not qualify\";\n"
        "puff eligibility;\n", //Output: Qualifies
        NULL
    };
    VM vm;
    init_VM(&vm, NULL,NULL, true);

    for (int i = 0; es[i] != NULL; ++i) {
        char *source = es[i];

        printf("++++++++++++\n interpret for test condition expression: \n\n%s \n+++++++++++\n", source);
        printf("\n------ result ------\n");
        InterpretResult result = interpret(&vm, NULL, source);
        assert(result == INTERPRET_OK);
        printf("\n\n\n");
    }
    free_VM(&vm);
}

static void test_string() {
    char *es[] = {
        "puff \"a\\\"b\\n\";\n"
        "puff \"abc123\\n\";\n"
        "puff \"b\u6735\u6735s01b\";\n",
        "puff \"\\u6735\" +\"\\n\";\n",
        "puff \"\u6735\";",
        "puff 1.312341234123412341234;",
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
    test_computed_goto,
    test_vm,
    test_global_variable,
    test_global_variable_long,
    test_local_variable,
    test_local_variable_long,
    test_wish_statement,
    test_wloop_statement,
    test_loop_statement,
    test_skip,
    test_equals,
    test_break,
    test_magic,
    test_magic2,
    test_charm,
    test_castle,
    test_property,
    test_method,
    test_super,
    test_static_method,
    test_condition,
    test_string,
    NULL
};

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    run_tests(tests);
    return 0;
}
