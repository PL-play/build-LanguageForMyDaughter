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
    "1+3*3;",
    "(1-2)/3;",
    "-1-2;",
    "(2-3)*4/3;",
    "print 1.234*3.14-1--2;",
    "23.234-34*(123+-5*343)/23-23;",
    "23.234--2353.39;",
    "(-1 + 2) * 3 - -4;",
    "-3!+4^3;",
    "true;",
    "!false;",
    "nil;",
    "!0;",
    "!1;",
    "!nil;",
    "1>2;",
    "2>=2;",
    "1<2;",
    "1<=2;",
    "2<=2;",
    "3>2;",
    "!(6 - 4 > 3 * 4 == !nil);",
    "print 1;",
    "print \"12345\";",
    "print \"2\";",
    "\"aaa\"==\"aaa\";",
    "\"aab\"==\"aaa\";",
    "print \"aaa\"+\"bbb\"+\"123\";",
    "print 1+2;",
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
      "var a=1;print a=1;",
      "var a;\nprint a;",
      "var a=1;\nprint a;",
      "var a=1;\nprint a; \nvar a=2; var b =3;\nprint a+b;",
      "var i = \"baba\"; var j = \"mama\"; var q = \"duoduo\";\n print i+\" love \"+j +\" love \"+ q;",
      "var i=3;var j=i;print i; print j; i=6; print i; print j;",
      "var aa=3; aa=6; print aa;",
      "aa=7; print aa;",
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
      "var a=1;{ var a=2; print a;} print a; a=3; print a;",
      "var a=1;{ var b=2; print a;} print a;",
      "var a=1;{  var b=a; print b;} var b=4;print b;",
//      "{var a=1;var a=2;}",
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
      "var a=1;{ var a=2; print a;} print a; a=3; print a;",
      "var a=1;{ var b=2; print a;} print a;",
      "var a=1;{  var b=a; print b;} var b=4;print b;",
//      "{var a=1;var a=2;}",
      NULL
  };
  VM vm;
  init_VM(&vm, NULL,NULL, true);
  for (int i = 0; i < 257; ++i) {
    char a[27];
    sprintf(a, "{var v%-3d=%-3d;print v%-3d;}", i, i, i);
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
    sprintf(a, "var v%-3d=%-3d;print v%-3d;", i, i, i);
    printf("%s\n", a);
    InterpretResult result = interpret(&vm, NULL, a);
    assert(result == INTERPRET_OK);
  }

  for (int i = 0; i < 10; ++i) {
    char a[100];
    sprintf(a, "var t%-2d=%-2d; t%-2d=%d;\nprint t%-2d;", i, i, i, i * i, i);
    printf("%s\n", a);
    printf("++++++++++++\n interpret expression: %s \n+++++++++++\n", a);
    printf("\n------ result ------\n");
    InterpretResult result = interpret(&vm, NULL, a);
    assert(result == INTERPRET_OK);
  }

  free_VM(&vm);
}

static void test_if_statement() {
  VM vm;
  init_VM(&vm, NULL,NULL, true);
  char *es[] = {
      "var a=1;if(a>=1){ print a;} print 3;",
      "if(1)  print 2;",
      "var a =5; if(a>3){"
      "   print a;"
      "   var b = 5;"
      "   if(a>b) print \"duoduo\"; "
      "     else print \" love\";"
      "} else {"
      "   print \"baba\";"
      "}",
      "var a = 4; if(a>8) {print 8;} else if(a>6){print 6;} else {print a;}",
      "var a = 9; if(a>8) {print 8;} else if(a>6){print 6;} else {print a;}",
      "var a = 8; if(a>8) {print 8;} else if(a>6){print 6;} else {print a;}",
      NULL
  };
  for (int i = 0; es[i] != NULL; ++i) {
    char *source = es[i];

    printf("++++++++++++\n interpret if statements: %s \n+++++++++++\n", source);
    printf("\n------ result ------\n");
    InterpretResult result = interpret(&vm, NULL, source);
    assert(result == INTERPRET_OK);
    printf("\n\n\n");
  }
  free_VM(&vm);
}

static void test_while_statement() {
  VM vm;
  init_VM(&vm, NULL,NULL, true);
  char *es[] = {
      "var a=5; \n"
      "while(a>=0){\n"
      "  a = a-1;\n"
      "  print a && (a-1) || \"i love duoduo\";\n"
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

static void test_for_statement() {
  VM vm;
  init_VM(&vm,NULL, NULL, true);
  char *es[] = {
      "for(var a=0;a<5;a=a+1){\n"
      "    print \"duoduo cool\";\n"
      "}",

      "var b;\n"
      "for(b=5;b>0;b=b-1){\n"
      "  print b;\n"
      "  print \"mama love duoduo\";\n"
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

static void test_continue() {
  VM vm;
  init_VM(&vm, NULL,NULL, true);
  char *es[] = {
      "print 1 + 1+nil+false+true;\n",
      "print \"\" + 1 + 1;\n",
      "var a=1; print \"\" + a + a;\n",
      "for(var a=0;a<5;a=a+1){\n"
      "   if(a==3) continue;  \n"
      "   print \"a=\"+(a+1);\n"
      "}",
      "for(var a =0;a<5;a=a+1){\n"
      "  for(var b=0;b<5; b=b+1){\n"
      "    if(a+b==7) continue;\n"
      "    print \"\"+ a +\"+\"+ b+\"=\"+(a+b);\n"
      "  }\n"
      "  print \"\";"
      "}\n",
      "var b;\n"
      "for(b=5;b>0;b=b-1){\n"
      "  if(b%2==0) continue;\n"
      "  print b;\n"
      "  print \"mama love duoduo\";\n"
      "}",
      "var a=5; \n"
      "while(a>=0){\n"
      "  a = a-1;\n"
      "  if(a%2==0) continue;"
      "  print a && (a-1) || \"i love duoduo\";\n"
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
      "var b1 = false;\n"
      "var b2 = false;\n"
      "print \"b1==b2 : \" + (b1==b2);\n"
      "b2=true;"
      "print \"b1==b2 : \" + (b1==b2);\n"
      "var s1 = \"duoduo\";\n"
      "var s2 = \"duo\"+\"duo\";\n"
      "print \"s1==s2 : \" + (s1==s2);\n"
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
      "for(var a=0;a<5;a=a+1){\n"
      "   if(a==3) break;  \n"
      "   if(a%2==0) continue;"
      "   print a;\n"
      "}\n"
      "print \"end\";",
      "var a=5; \n"
      "while(a>=0){\n"
      "  a = a-1;\n"
      "  if(a==0) break;\n"
      "  if(a%2==0) continue;\n"
      "  print \"i love duoduo\"+a;\n"
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

static void test_function() {
  VM vm;
  init_VM(&vm, NULL,NULL, true);
  char *es[] = {
      "fun f1(a,b){\n"
      "      return a+b;\n"
      "   }\n"
      "print f1(2,4);",
      "fun f1(a){\n"
      "   if (a<=1) return 1;\n"
      "   return a*f1(a-1);\n"
      "}\n"
      "print f1(7)+f1(2);",
      "{\n"
      "     fun f2(a,b){return a+b;}\n"
      "     print f2(4,5);\n"
      "}",
      "lambda(a,b){print a+b;}(1,\"234\");\n",
      "var ff = lambda(a,b){print a+b; return a+b;};\n"
      "ff(\"i love\", ff(\" mama ,\", \"duoduo\"));",
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

static void test_function2() {
  VM vm;
  init_VM(&vm, NULL,NULL, true);
  char *es[] = {
      "fun f1(a,b){\n"
      "      return a(b);\n"
      "   }\n"
      "print f1(lambda(a){return a+a;}, \"duo\");",
      "fun f1(a){\n"
      "   if (a<2) return a;\n"
      "   return f1(a-2)+f1(a-1);\n"
      "}\n"
      "var start = __clock();\n"
      "print f1(5);\n"
      "print __clock() - start;\n",
      "print __clock();",
      "fun f1(a,b){\n"
      "      return a+b;\n"
      "   }\n"
      "print f1(1,4);\n",
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

static void test_closure() {
  VM vm;
  init_VM(&vm,NULL, NULL, true);
  char *es[] = {
      "fun outer(){\n"
      "  var x = \"duoduo\";\n"
      "  fun inner(){\n"
      "    x = \"I love \" + x;\n"
      "    print x;\n"
      "  }\n"
      "  inner();\n"
      "}\n"
      "outer();\n",
      "{\n"
      "   var x = \"outside\";\n"
      "   fun inner() {\n"
      "    x = \"inside\";\n"
      "     print x;\n"
      "   }\n"
      "   inner();\n"
      " }\n",
      "\n"
      "fun outer(){\n"
      "  var x = \"duoduo\";\n"
      "  fun inner(){\n"
      "    x = \"baba love \"+x;"
      "    print x;\n"
      "  }\n"
      "  return inner;\n"
      "}\n"
      "var closure = outer();\n"
      "closure();"
      "",
      "\n"
      "fun outer(){\n"
      "  var x = \"duoduocool\";\n"
      "  fun middle(){\n"
      "    fun inner(){\n"
      "       print x;\n"
      "    }"
      "    print \"create inner closure\";\n"
      "    return inner;\n"
      "  }\n"
      "  print \"return from outer\";\n"
      "  return middle;\n"
      "}\n"
      "var mid = outer();\n"
      "var in = mid();\n"
      "in();\n",
      "{\n"
      "   var y = \"outside\";\n"
      "   fun inner() {\n"
      "     print y;\n"
      "   }\n"
      "   inner();\n"
      " }\n",
      NULL
  };
  for (int i = 0; es[i] != NULL; ++i) {
    char *source = es[i];

    printf("++++++++++++\n interpret for closure: \n\n%s \n+++++++++++\n", source);
    printf("\n------ result ------\n");
    InterpretResult result = interpret(&vm, NULL, source);
    assert(result == INTERPRET_OK);
    printf("\n\n\n");
  }
  free_VM(&vm);
}

static void test_class() {
  char *es[] = {
      "class A{}\n"
      "print A;\n"
      "{\n"
      "  class B{}\n"
      "  print A;\n"
      "  print B;\n"
      "  var a = A;\n"
      "  print \"a=\"+a;\n"
      "   a = B;\n"
      "  print \"a=\"+a;\n"
      "}\n"
      "var a =A;\n"
      "print \"a=\"+a;\n"
      "",
      "fun f1(){\n"
      "  class Duoduo{}\n"
      "  print Duoduo;\n"
      "  fun inf1(){\n"
      "    print \"inf1:\"+Duoduo;\n"
      "    return Duoduo;\n"
      "  }\n"
      "  return inf1;\n"
      "}\n"
      "var f = f1();\n"
      "var c = f();\n"
      "print c;\n",
      "print A();\n"
      "print c();\n"
      "var ic = c();\n"
      "print \"ic=\"+ic;\n",
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
      "class A{}\n"
      "var a = A();\n"
      "a.f1=3;\n"
      "a.f2 = 8;\n"
      "print a.f1;\n"
      "print a.f2;\n"
      "print a.f1+a.f2;\n"
      "print __has_field(a,\"f1\");\n"
      "__del_field(a,\"f1\");\n"
      "print __has_field(a,\"f1\");\n",
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
      "class A{\n"
      "    init(name){\n"
      "       this.name = name;\n"
      "       print \"set name in init(): \"+this.name;\n"
      "    }\n"
      "    m1(){\n"
      "       print \"get name in m1():\"+ this.name;\n"
      "    }\n"
      "    m2(name){\n"
      "      this.name = name;\n"
      "      print \"call m1() inside m2()\";\n"
      "      this.m1();\n"
      "      print \"set name in m2(): \"+this.name;\n"
      "   }\n"
      "   m3(p){\n"
      "      fun f(){"
      "        print this.name+\" \"+ p;"
      "      }\n"
      "      return f;"
      "   }\n"
      "}\n"
      ""
      "class B{\n"
      "  init(){\n"
      "     this.name = \"MAMA\";"
      "     print \"init B\";\n"
      "  }\n"
      "}\n"
      "print \"call init\";\n"
      "var a = A(\"baba and \");\n",
      "print \"get name in global:\"+ a.name;\n"
      "print \"call m1\";\n",
      "a.m1();\n",
      "print \"call m2\";\n"
      "a.m2(\"duoduo\");\n"
      "print \"call m1\";\n"
      "var m1 = a.m1;\n"
      "m1();"
      "print \"get name in global call:\"+ a.name;\n"
      "print \"call m3\";\n"
      "var f = a.m3(\"baba mama\");\n"
      "f();\n"
      "a.b = B;\n"
      "var b = a.b();\n"
      "print \"b name: \"+ b.name;\n"
      "a.c = lambda(x){print x.name;};\n"
      "print \"call a.c()\";\n"
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
      "class A{\n"
      "    m1(){\n"
      "       print \"A m1()\";\n"
      "    }\n"
      "    m2(){\n"
      "       print \"m2\";\n"
      "    }\n"
      "}\n"
      ""
      "class B < A{\n"
      "  init(){\n"
      "    super.m1();\n"
      "    var m = super.m1;\n"
      "    m();\n"
      "  }\n"
      "  m1(){\n"
      "    print \"B m1\";\n"
      "  }\n"
      "}\n"
      "var b = B();\n"
      "b.m2();\n"
      "b.m1();\n"
      "print __has_method(b,\"m1\");\n",
      "print __has_method(b,\"m2\");\n",
      "print __has_method(A,\"m2\");\n",
      "print __has_method(B,\"m2\");\n",
      "print __has_method(A,\"init\");\n",
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
      "class A{\n"
      "    static m1(){\n"
      "       print \"A static m1()\";\n"
      "    }\n"
      "    m2(){\n"
      "       print \"A m2\";\n"
      "       A.m1();\n"
      "    }\n"
      "}\n"
      "class B<A{\n"
      "  m3(){\n"
      "     print \"b call super methods: \";\n"
      "     super.m1();\n"
      "     super.m2();\n"
      "  }\n"
      ""
      "}\n"
      ""
      "A.m1();\n"
      "var a = A();\n"
      "a.m1();\n"
      "var m1 = A.m1;\n"
      "m1();\n"
      "print \"call m2: \";\n"
      "a.m2();\n"
      "var b = B();\n"
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
      "var i = 3;\n"
      "var j = i>2?1:-1;\n"
      "print j;\n",// expect 1
      "var age = 22;\n"
      "var income = 45000;\n"
      "var creditScore = 700;\n"
      "var completedFinancialCourse = true;\n"
      "// Nested ternary conditional expression\n"
      "var eligibility = (age >= 18 && age <= 60) ?\n"
      "    (income >= 30000 && income <= 90000) ?\n"
      "        (creditScore >= 650) ?\n"
      "            (age < 25 ? (completedFinancialCourse ? \"Qualifies\" : \"Does not qualify\") : \"Qualifies\")\n"
      "        : \"Does not qualify\"\n"
      "    : \"Does not qualify\"\n"
      ": \"Does not qualify\";\n"
      "print eligibility;\n",//Output: Qualifies
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
      "print \"a\\\"b\\n\";\n"
      "print \"abc123\\n\";\n"
      "print \"b\u6735\u6735s01b\";\n",
      "print \"\\u6735\" +\"\\n\";\n",
      "print \"\u6735\";",
      "print 1.312341234123412341234;",
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
    test_if_statement,
    test_while_statement,
    test_for_statement,
    test_continue,
    test_equals,
    test_break,
    test_function,
    test_function2,
    test_closure,
    test_class,
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