//
// Created by ran on 2024-03-24.
//
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "common/framework.h"
#include "compiler/parser.h"
#include "chunk/chunk.h"

static char *exps[] = {
    "a%2;",
    "1;",
    "a=1;",
    "b=e=4;",
    "a + b ? c * d : e / f;",
    "a ? b : c ? d : e;",
    "1 + 2;",
    "1+3*3;",
    "(1-2)/3;",
    "-1-2;",
    "(2-3)*4/3;",
    "1.234*3.14-1--2;",
    "23.234-34*(123+-5*343)/23-23;",
    "(-1 + 2) * 3 - -4;",
    "nil;",
    "aow;",
    "1>2;",
    "2<4;",
    "2==5;",
    "3>=5;",
    "4<=1;",
    "a(1,2,3,4,5,6);",
    "waa a;",
    // "print this;",
    "waa a=1+2;",
    "a.b;",
    "a=efs;",
    "a.b=efs;",
    "a.b(c).d.f=\"duoduo\";",
    "{waa a=1; {waa b=2;}}",
    "wish(a>b) puff a; dream puff b;",
    "a and b or !c;",
    "a && b || !c;",
    "a + !b;",
    "a + -b;",
    "a && b && !c;",
    "wloop(a){puff a;}",
    "waa a=\"5\"; \n",
    "wloop(a>0){\n"
    "  puff a;\n"
    "  a = a-1;\n"
    "}\n",
    "loop(;;){\n"
    "   puff a;\n"
    "}",
    "loop(a=1;;){\n"
    "   puff a;\n"
    "}",
    "loop(waa a=1;;){\n"
    "   puff a;\n"
    "}",
    "loop(;a>0;){\n"
    "   puff a;\n"
    "}",
    "loop(waa a=1;a>0;a=a+1){\n"
    "   puff a;\n"
    "}",

    "loop(;;){\n"
    "   skip;\n"
    "} ",
    "loop(;;){\n"
    "   break;\n"
    "}",
    "a(1,2,3);",
    "a(1,2,3)();",
    "n(a,b,v){puff a;};",
    "magic n(a,b,v){puff a;}",
    "magic f1(a,b){\n"
    "      home a+b;\n"
    "   }",
    "shadow(a,b){home a+b;}(1,2);",
    "1; waa x = \"outside\";\n",
    "   magic inner() {\n"
    "     puff x;\n"
    "   }\n"
    "   inner();\n",
    "magic outer(){\n"
    "magic outer(){\n"
    "  waa x = \"duoduo\";\n"
    "  magic inner(){\n"
    "    x = \"I love \" + x;\n"
    "    puff x;\n"
    "  }\n"
    "  inner();\n"
    "  }\n"
    "}\n"
    "outer();\n",
    NULL

};
static void test_parse_exp() {
  for (int i = 0; exps[i] != NULL; ++i) {
    char *source = exps[i];
    printf("exp:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_parse_stmt() {
  char *stmts[] = {
      "class C1 < C2 {\n"
      "    init(a){\n"
      "      this.a = a;\n "
      "    }\n"
      "    method1(){\n"
      "      print this.a;\n"
      "    }\n"
      "    static method2(a,b,c){\n"
      "      method1();\n"
      "    }\n"
      "}\n",
      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_parse_invoke() {
  char *stmts[] = {
      "a();\n",
      "a.b();",
      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_parse_super() {
  char *stmts[] = {
      "class A < B{\n"
      "    init(){\n"
      "      super.m();\n"
      "    }\n"
      "}\n",
      "class A{\n"
      "  static m(){\n"
      "     class B<A{\n"
      "       static m(){\n"
      "       }\n"
      "       m1(){\n"
      "         this;\n"
      "         super.m();\n"
      "       }\n"
      "     }\n"
      "  }\n"
      "}\n",
      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_static_method() {
  char *stmts[] = {
      "class A {\n"
      "    static m(){\n"
      "      this;"
      "      super.m();\n"
      "    }\n"
      "}\n",

      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_ERROR);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_comment() {
  char *stmts[] = {
      "// 1;//\n"
      "/*\n"
      "*\n"
      "*sdfgsdf\n"
      "32145//asdf\n"
      "//sdfg"
      "*/\n"
      "//\n"
      "// 2;\n",

      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_import() {
  char *stmts[] = {
      "want \"a.b\" as c;",
      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_array() {
  char *stmts[] = {
      "[a,b,1,2];",
      "a[1];",
      "a[1:2];",
      "a[1:];",
      "a[:2];",
      "a[:];",
      "a[b];",
      "a[b:c][1]=2;",
      "a[b:];",
      "a[:c];",
      "a[1]=2;",
      "a[1][2][4]=2;",
      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}

static void test_string() {
  char *stmts[] = {
      "print \"abc\\n\";",
      "print \"a\\\"b\\n\";\n",
      "print \"abc123\\n\";\n",
      "print \"b\u6735\u6735s01b\";\n",
      "print \"\\u6735\" +\"\\n\";\n",
      "print \"\u6735\";",
//      "\"${a+b}\";",
      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}


static void test_try() {
  char *stmts[] = {
      "try{\n"
      "  var a=1;\n"
      "} catch(Expression as e){\n"
      "  print false;\n"
      "} catch(E2 as e){\n"
      "  throw e;\n"
      "}\n"
      "finally {\n"
      "  a=a+1;\n"
      "}\n",
      NULL
  };
  for (int i = 0; stmts[i] != NULL; ++i) {
    char *source = stmts[i];
    printf("stmt:\n %s\n", source);
    StatementArrayList *statements = NULL;
    Parser parser;
    int result = parse(&parser, source, &statements);
    assert(result == PARSE_OK);
    char *ast = print_statements(&parser, statements);
    printf("%s\n", ast);
    free(ast);
    for (int j = 0; j < statements->size; ++j) {
      free_statement(&parser, Statementget_data_arraylist(statements, j));
    }
    Statementfree_arraylist(statements);
  }
}


static UnitTestFunction tests[] = {
    test_parse_exp,
    // test_parse_stmt,
    // test_parse_invoke,
    // test_parse_super,
    // test_static_method,
    // test_comment,
    // test_import,
    // test_array,
    // test_string,
    // test_try,
    NULL
};

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  run_tests(tests);
  return 0;
}