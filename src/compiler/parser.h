//
// Created by ran on 2024-03-24.
//

#ifndef ZHI_COMPILER_PARSER_H_
#define ZHI_COMPILER_PARSER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include "compiler/scanner.h"
#include "chunk/object.h"
typedef char ParseType;
#define ASSIGN_EXPR (ParseType)0 // An assignment expression like "a = b"
#define CALL_EXPR (ParseType) 1 // A function call like "a(b, c, d)"
#define CONDITIONAL_EXPR (ParseType)2 // A ternary conditional expression like "a ? b : c"
#define VARIABLE_EXPR (ParseType)3 // A simple variable name expression like "abc".
#define OPERATOR_EXPR (ParseType)4 // A binary arithmetic expression like "a + b" or "c ^ d"
#define PREFIX_EXPR (ParseType)5 // A prefix prefix arithmetic expression like "!a" or "-b"
#define POSTFIX_EXPR (ParseType)6 // A postfix prefix arithmetic expression like "a!"
#define LITERAL_EXPR (ParseType)7 // A literal like '1',"ab,'true','nil'
#define GET_EXPR (ParseType)8 // A.B
#define SET_EXPR (ParseType)9 // A.B = C
#define OR_EXPR (ParseType)10 // || or
#define AND_EXPR (ParseType)11 // && and
#define FUNC_EXPR (ParseType) 12 // function
#define THIS_EXPR (ParseType) 13 // this
#define SUPER_EXPR (ParseType) 14 // super
#define ARRAY_EXPR (ParseType) 15 // array []
#define INDEXING_EXPR (ParseType) 16 // array[i]
#define SLICING_EXPR (ParseType) 17 // array[i:j]
#define ELEMENT_ASSIGN_EXPR (ParseType) 18 // array[i] = j

#define PRINT_STMT (ParseType) 20
#define EXPRESSION_STMT (ParseType)21
#define VAR_DECL_STMT (ParseType)22
#define BLOCK_STMT (ParseType)23
#define IF_STMT (ParseType)24
#define WHILE_STMT (ParseType)25
#define FOR_STMT (ParseType)26
#define CONTINUE_STMT (ParseType)27
#define BREAK_STMT (ParseType)28
#define FUNC_STMT (ParseType)29
#define RETURN_STMT (ParseType)30
#define CLASS_STMT (ParseType)31
#define IMPORT_STMT (ParseType)32
#define TRY_STMT (ParseType)33
#define CATCH_STMT (ParseType)34
#define THROW_STMT (ParseType)35

#define PARSE_OK 0
#define PARSE_ERROR 11

typedef struct Parser {
  Token current;
  Token prev;
  bool has_err;
  bool panic_mode;
  Scanner *scanner;
  int loop_depth;
  int func_depth;
  int class_depth;
  bool class_has_super[MAX_CLASS_NESTING];
  int static_method_depth;
} Parser;

typedef struct ParseTree {
  ParseType type;
  size_t line;
} ParseTree;

typedef struct Expression Expression;
typedef struct Statement Statement;

// type for all expression AST node types
struct Expression {
  ParseType type;
  size_t line;

  union {
    struct {
      Token name;
      Expression *right;
    } assign;

    struct {
      Expression *object;
      Expression *index;
      Expression *right;
    } element_assign;

    struct {
      Expression *object;
      Expression *index;
    } indexing;

    struct {
      Expression *object;
      Expression *index0;
      Expression *index1;
    } slicing;

    struct {
      Expression *function;
      size_t arg_num;
      Expression **args;
    } call;

    struct {
      Token *parameters;
      size_t param_num;
      Statement *body;
    } function;

    struct {
      Expression *object;
      Token name;
    } get;

    struct {
      Expression *object;
      Token name;
      Expression *value;
    } set;

    struct {
      Expression *condition;
      Expression *then_exp;
      Expression *else_exp;
    } conditional;

    struct {
      Token name;
    } variable;

    struct {
      Token this_;
    } this_;

    struct {
      Token super_;
    } super_;

    struct {
      Expression *left;
      TokenType operator;
      Expression *right;
    } operator;

    struct {
      Expression *left;
      Expression *right;
    } or;

    struct {
      Expression *left;
      Expression *right;
    } and;

    struct {
      TokenType operator;
      Expression *right;
    } prefix;

    struct {
      TokenType operator;
      Expression *left;
    } postfix;

    struct {
      size_t size;
      Expression **elements;
    } array;

    Value value;
  };
};

struct Statement {
  ParseType type;
  size_t line;

  union {
    Expression *expression;
    Expression *print_expr;

    struct {
      Token name;
      Expression *initializer;
    } var_decl;

    struct {
      size_t stmt_nums;
      struct Statement **statements;
    } block;

    struct {
      Expression *condition;
      struct Statement *then_stmt;
      struct Statement *else_stmt;
    } if_stmt;

    struct {
      Expression *condition;
      Statement *body;
    } while_stmt;

    struct {
      // either a variable declaration or expression statement or NULL
      struct Statement *initializer;
      Expression *condition;
      Expression *increment;
      struct Statement *body;
    } for_stmt;

    struct {
      Token token;
    } continue_stmt;

    struct {
      Token token;
    } break_stmt;

    struct {
      Token name;
      Expression *function;
    } function_stmt;

    struct {
      Token keyword;
      Expression *value;
    } return_stmt;

    struct {
      Token name;
      Expression *super_class;
      Statement **methods;
      size_t methods_num;
      Statement **static_methods;
      size_t static_methods_num;
    } class_stmt;

    struct {
      Token lib;
      Expression *as;
    } import_stmt;

    struct {
      Statement *try_block;
      Statement **catch_stmt;
      size_t catch_nums;
      Statement *finally_block;
    } try_stmt;

    struct {
      Expression *catch_exception;
      Expression *as;
      Statement *catch_block;
    } catch_stmt;

    struct {
      Token kw;
      Expression *throwable;
    } throw_stmt;
  };
};

DECLARE_ARRAY_LIST(Statement*, Statement)

DECLARE_ARRAY_LIST(Token, Token)

int parse(Parser *parser, const char *source, StatementArrayList **array_list);

char *print_statements(Parser *parser, StatementArrayList *statements);

void free_statement(Parser *parser, Statement *statement);

void free_statements(Parser *parser, StatementArrayList *statements);

size_t eof_line(Parser *parser);

bool has_error(Parser *parser);
#ifdef __cplusplus
}
#endif

#endif //ZHI_COMPILER_PARSER_H_
