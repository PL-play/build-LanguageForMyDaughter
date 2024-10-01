//
// Created by ran on 2024-03-24.
//
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "list/linked_list.h"
#include "zjson.h"
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef ALLOC_TESTING
#include "common/alloc-testing.h"
#endif

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_CONDITIONAL, // ? :
  PREC_OR, // or
  PREC_AND, // and
  PREC_EQUALITY, // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM, // + -
  PREC_FACTOR, // * /
  PREC_EXPONENT, // ^
  PREC_PREFIX, // ! -
  PREC_POSTFIX, // !
  PREC_CALL, // . () []
  PREC_PRIMARY // literals
} Precedence;

DEFINE_ARRAY_LIST(NULL, Statement)

DEFINE_ARRAY_LIST((Token) {.type=TOKEN_ERROR}, Token)

#define LITERAL_EXP(v, line_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = LITERAL_EXPR;\
                              e->value = v;                              \
                              e->line=line_; \
                              e;\
                           })

#define PREFIX_EXP(token_type, r, line_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = PREFIX_EXPR;\
                              e->prefix.operator = token_type;                 \
                              e->prefix.right = r;                       \
                              e->line=line_; \
                              e;\
                           })

#define ID_EXP(identifier_name) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = VARIABLE_EXPR;\
                              e->variable.name = identifier_name;      \
                              e->line=identifier_name.line; \
                              e;\
                           })

#define THIS_EXP(identifier_name) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = THIS_EXPR;\
                              e->this_.this_ = identifier_name;      \
                              e->line=identifier_name.line; \
                              e;\
                           })

#define SUPER_EXP(identifier_name) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = SUPER_EXPR;\
                              e->super_.super_ = identifier_name;      \
                              e->line=identifier_name.line; \
                              e;\
                           })

#define OPERATOR_EXP(l, op, r, line_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = OPERATOR_EXPR;\
                              e->operator.left = l;                   \
                              e->operator.operator = op;                 \
                              e->operator.right = r;                     \
                              e->line=line_; \
                              e;\
                           })

#define OR_EXP(l, r, line_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = OR_EXPR;\
                              e->or.left = l;                   \
                              e->or.right = r;                     \
                              e->line=line_; \
                              e;\
                           })

#define AND_EXP(l, r, line_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = AND_EXPR;\
                              e->and.left = l;                   \
                              e->and.right = r;                     \
                              e->line=line_; \
                              e;\
                           })

#define POSTFIX_EXP(token_type, l, line_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = POSTFIX_EXPR;\
                              e->postfix.left = l;                   \
                              e->postfix.operator = token_type;         \
                              e->line=line_; \
                              e;\
                           })

#define CONDITIONAL_EXP(con, then_, else_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = CONDITIONAL_EXPR;\
                              e->conditional.condition = con;                   \
                              e->conditional.then_exp = then_;        \
                              e->conditional.else_exp = else_;                 \
                              e;\
                           })

#define ASSIGN_EXP(n, r) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = ASSIGN_EXPR;\
                              e->assign.name = n;                   \
                              e->assign.right = r;                       \
                              e->line=n.line;\
                              e;\
                           })

#define SET_EXP(obj_, name_, value_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = SET_EXPR;\
                              e->set.name = name_;                       \
                              e->line=name_.line;\
                              e->set.object = obj_;                      \
                              e->set.value=value_;\
                              e;\
                           })

#define GET_EXP(obj_, name_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = GET_EXPR;\
                              e->get.name = name_;                       \
                              e->line=name_.line;\
                              e->get.object = obj_;                      \
                              e;\
                           })
#define CALL_EXP(func, arg_nums, arg_list, line_) ({\
                              Expression *e = malloc(sizeof(Expression)); \
                              e->type = CALL_EXPR;\
                              e->line = line_;                     \
                              e->call.function = func;                   \
                              e->call.arg_num = arg_nums;                 \
                              e->call.args = (arg_list);        \
                              e;\
                           })
#define INDEXING_EXP(obj_, index_, line_) ({\
                              Expression *e = malloc(sizeof(Expression)); \
                              e->type = INDEXING_EXPR;\
                              e->line = line_;                     \
                              e->indexing.object = (obj_);                   \
                              e->indexing.index = (index_);        \
                              e;\
                           })
#define SLICING_EXP(obj_, index0_, index1_, line_) ({\
                              Expression *e = malloc(sizeof(Expression)); \
                              e->type = SLICING_EXPR;\
                              e->line = line_;                     \
                              e->slicing.object = (obj_);                   \
                              e->slicing.index0 = (index0_);              \
                              e->slicing.index1 = (index1_);        \
                              e;\
                           })
#define ELEMENT_ASSIGN_EXP(object_, index_, right_, line_) ({\
                              Expression *e = malloc(sizeof(Expression)); \
                              e->type = ELEMENT_ASSIGN_EXPR;\
                              e->line = line_;                     \
                              e->element_assign.object = (object_);                   \
                              e->element_assign.index_ = (index_);        \
                              e->element_assign.right = (right_);   \
                              e;\
                           })
#define ARR_EXP(ele_nums, ele_list, line_) ({\
                              Expression *e = malloc(sizeof(Expression)); \
                              e->type = ARRAY_EXPR;\
                              e->line = line_;                     \
                              e->array.size = ele_nums;                 \
                              e->array.elements = (ele_list);        \
                              e;\
                           })
#define FUNC_EXP(params_num_, parameters_, body_) ({\
                              Expression *e = malloc(sizeof(Expression));\
                              e->type = FUNC_EXPR;\
                              e->function.param_num = params_num_;\
                              e->function.parameters = parameters_;\
                              e->function.body = body_;\
                              e;\
                          })

#define PRINT_STMTS(exp, line_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = PRINT_STMT;                       \
                                 stmt->line = line_;                             \
                                 stmt->print_expr = (exp);                      \
                                 stmt;\
                               })

#define EXPR_STMTS(exp, line_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = EXPRESSION_STMT;                       \
                                 stmt->line = line_;                             \
                                 stmt->expression = (exp);                      \
                                 stmt;\
                               })

#define VAR_DECL_STMTS(t, exp, line_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = VAR_DECL_STMT;                       \
                                 stmt->line = line_;                             \
                                 stmt->var_decl.name = (t);                    \
                                 stmt->var_decl.initializer = (exp);  \
                                 stmt;\
                               })

#define CLASS_DECL_STMTS(cname, sname, methods_, methods_num_, static_methods_, static_methods_num_, line_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = CLASS_STMT;                      \
                                 stmt->line = line_;         \
                                 stmt->class_stmt.name = (cname);                    \
                                 stmt->class_stmt.super_class = (sname);                                       \
                                 stmt->class_stmt.methods = methods_;                                                     \
                                 stmt->class_stmt.methods_num = methods_num_;                                             \
                                 stmt->class_stmt.static_methods = static_methods_;            \
                                 stmt->class_stmt.static_methods_num = static_methods_num_;             \
                                 stmt;\
                               })

#define BLOCK_STMTS(stmts, n, line_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = BLOCK_STMT;                      \
                                 stmt->line=line_;      \
                                 stmt->block.statements = stmts;                             \
                                 stmt->block.stmt_nums = n;                    \
                                 stmt;\
                               })

#define IF_STMTS(cond, then_stmt_, else_stmt_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = IF_STMT;                       \
                                 stmt->if_stmt.condition = (cond);                             \
                                 stmt->if_stmt.then_stmt = (then_stmt_);                    \
                                 stmt->if_stmt.else_stmt = (else_stmt_);                    \
                                 stmt;\
                               })
#define WHILE_STMTS(cond, body_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = WHILE_STMT;                       \
                                 stmt->while_stmt.condition = (cond);                             \
                                 stmt->while_stmt.body = (body_);              \
                                 stmt;\
                               })
#define FOR_STMTS(init_, cond_, incre_, body_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = FOR_STMT;                        \
                                 stmt->for_stmt.initializer = (init_); \
                                 stmt->for_stmt.condition = (cond_);           \
                                 stmt->for_stmt.increment = (incre_);      \
                                 stmt->for_stmt.body = (body_);              \
                                 stmt;\
                               })
#define CONTINUE_STMTS(token_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = CONTINUE_STMT;                        \
                                 stmt->continue_stmt.token = (token_); \
                                 stmt;\
                               })
#define BREAK_STMTS(token_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = BREAK_STMT;                        \
                                 stmt->continue_stmt.token = (token_); \
                                 stmt;\
                               })
#define RETURN_STMTS(token_, exp_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = RETURN_STMT;                     \
                                 stmt->line = token_.line;\
                                 stmt->return_stmt.keyword = (token_);           \
                                 stmt->return_stmt.value = (exp_); \
                                 stmt;\
                               })

#define FUNC_STMTS(name_, func_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = FUNC_STMT;                       \
                                 stmt->line=(name_).line;   \
                                 stmt->function_stmt.name = (name_); \
                                 stmt->function_stmt.function = (func_); \
                                 stmt;\
                               })

#define IMPORT_STMTS(name_, as_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = IMPORT_STMT;                       \
                                 stmt->line=(name_).line;   \
                                 stmt->import_stmt.lib = (name_); \
                                 stmt->import_stmt.as = (as_); \
                                 stmt;\
                               })

#define TRY_STMTS(try_block_, catch_stmt_, finally_block_, catch_nums_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = TRY_STMT;                       \
                                 stmt->try_stmt.try_block = (try_block_); \
                                 stmt->try_stmt.catch_stmt = (catch_stmt_);    \
                                  stmt->try_stmt.finally_block = (finally_block_); \
                                  stmt->try_stmt.catch_nums = (catch_nums_); \
                                  stmt;\
                               })

#define CATCH_STMTS(catch_block_, exception_, as_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = CATCH_STMT;                       \
                                 stmt->catch_stmt.catch_block = (catch_block_); \
                                 stmt->catch_stmt.as = (as_);                  \
                                 stmt->catch_stmt.catch_exception = (exception_); \
                                 stmt;\
                               })

#define THROW_STMTS(kw_, throwable_) ({ \
                                 Statement* stmt = malloc(sizeof(Statement));  \
                                 stmt->type = THROW_STMT;                      \
                                  stmt->line=(kw_).line;   \
                                 stmt->throw_stmt.kw = (kw_); \
                                 stmt->throw_stmt.throwable = (throwable_);                  \
                                 stmt;\
                               })

typedef Expression *(*ParseFn)(Parser *parser, Token token, Expression *left);

typedef struct {
  // the function to compile a prefix expression starting with a token of that type
  ParseFn prefix;
  // the function to compile an infix expression whose left operand is followed by a token of that type
  ParseFn infix;
  // The precedence of an infix expression that uses that token as an operator.
  //  We don’t need to track the precedence of the prefix expression starting with a given
  //  token because all prefix operators have the same precedence
  Precedence precedence;
} ParseRule;

static void error_at_current(Parser *parser, const char *msg);
static void error(Parser *parser, const char *msg);
static void error_at(Parser *parser, Token *token, const char *msg);
static void advance(Parser *parser);
static void consume(Parser *parser, TokenType type, const char *msg);
static bool check(Parser *parser, TokenType type);
static bool match(Parser *parser, TokenType type);
static void synchronize(Parser *parser);

static Expression *parse_expression(Parser *parser, Precedence precedence);
static void free_expression(Parser *parser, Expression *e);

static ParseRule *get_rule(Parser *parser, TokenType type);
static Precedence precedence_of(Parser *parser, TokenType type);
static bool is_right_associative(Parser *parser, TokenType type);

static Statement *declaration(Parser *parser);
static Statement *statement(Parser *parser);
static Statement *print_statement(Parser *parser);
static Statement *expression_statement(Parser *parser);
static Statement *var_declaration(Parser *parser);
static Statement *fun_declaration(Parser *parser, bool is_method, bool is_static);
static Statement *block(Parser *parser);
static Statement *if_statement(Parser *parser);
static Statement *while_statement(Parser *parser);
static Statement *for_statement(Parser *parser);
static Statement *continue_statement(Parser *parser);
static Statement *break_statement(Parser *parser);
static Statement *return_statement(Parser *parser);
static Statement *class_declaration(Parser *parser);
static Statement *import_statement(Parser *parser);
static Statement *try_statement(Parser *parser);
static Statement *catch_statement(Parser *parser);
static Statement *throw_statement(Parser *parser);

static Expression *expression(Parser *parser);
static Expression *number(Parser *parser, Token token, Expression *left);
static Expression *nil(Parser *parser, Token token, Expression *left);
static Expression *boolean_value(Parser *parser, Token token, Expression *left);
static Expression *this_(Parser *parser, Token token, Expression *left);
static Expression *super_(Parser *parser, Token token, Expression *left);
static Expression *string(Parser *parser, Token token, Expression *left);
static Expression *grouping(Parser *parser, Token token, Expression *left);
static Expression *unary(Parser *parser, Token token, Expression *left);
static Expression *binary(Parser *parser, Token token, Expression *left);
static Expression *conditional(Parser *parser, Token token, Expression *left);
static Expression *identifier(Parser *parser, Token token, Expression *left);
static Expression *call(Parser *parser, Token token, Expression *left);
static Expression *fun_or_call(Parser *parser, Token token, Expression *left);
static Expression *lambda(Parser *parser, Token token, Expression *left);
static Expression *assign(Parser *parser, Token token, Expression *left);
static Expression *postfix(Parser *parser, Token token, Expression *left);
static Expression *getter(Parser *parser, Token token, Expression *left);
static Expression *or(Parser *parser, Token token, Expression *left);
static Expression *and(Parser *parser, Token token, Expression *left);
static Expression *array(Parser *parser, Token token, Expression *left);
static Expression *indexing_or_slicing(Parser *parser, Token token, Expression *left);

static json_value *expression_printer(Parser *parser, Expression *expression);
static json_value *statement_printer(Parser *parser, Statement *statement);
static json_value *tree_json(Parser *parser, ParseTree *tree);
static char *print_tree(Parser *parser, ParseTree *tree);

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, fun_or_call, PREC_CALL},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACKET] = {array, indexing_or_slicing, PREC_CALL},
    [TOKEN_RIGHT_BRACKET] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, getter, PREC_CALL},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_QUESTION] = {NULL, conditional, PREC_CONDITIONAL},
    [TOKEN_EXPONENT] = {NULL, binary, PREC_EXPONENT},
    [TOKEN_COLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_BANG] = {unary, postfix, PREC_POSTFIX},
    [TOKEN_PERCENT] = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, assign, PREC_ASSIGNMENT},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {identifier, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_INTERPOLATION] = {NULL, NULL, PREC_NONE},
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, and, PREC_AND},
    [TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE] = {boolean_value, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL] = {nil, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, or, PREC_OR},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER] = {super_, NULL, PREC_NONE},
    [TOKEN_THIS] = {this_, NULL, PREC_NONE},
    [TOKEN_TRUE] = {boolean_value, NULL, PREC_NONE},
    [TOKEN_VAR] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_IMPORT] = {NULL, NULL, PREC_NONE},
    [TOKEN_AS] = {NULL, NULL, PREC_NONE},
    [TOKEN_LAMBDA] = {lambda, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

int parse(Parser *parser, const char *source, StatementArrayList **array_list) {
  if (*array_list == NULL) *array_list = Statementnew_arraylist(32);
  // init parser
  Scanner scanner;
  init_scanner(&scanner, source);
  parser->panic_mode = false;
  parser->has_err = false;
  parser->scanner = &scanner;
  parser->loop_depth = 0;
  parser->func_depth = 0;
  parser->class_depth = 0;
  parser->static_method_depth = 0;

  advance(parser);
  while (!match(parser, TOKEN_EOF)) {
    Statementappend_arraylist(*array_list, declaration(parser));
  }
  consume(parser, TOKEN_EOF, "Expect end of expression.");
  return parser->has_err ? PARSE_ERROR : PARSE_OK;
}

size_t eof_line(Parser *parser) {
  return parser->has_err ? 0 : parser->prev.line;;
}
bool has_error(Parser *parser) {
  return parser->has_err;
}

static void free_expression(Parser *parser, Expression *expression) {
  if (expression == NULL) return;
  ParseType type = expression->type;
  switch (type) {
    case ASSIGN_EXPR: {
      free_expression(parser, expression->assign.right);
      free(expression);
      return;
    }
    case CALL_EXPR: {
      free_expression(parser, expression->call.function);

      if (expression->call.arg_num > 0) {
        for (int i = 0; i < expression->call.arg_num; ++i) {
          free_expression(parser, expression->call.args[i]);
        }
        free(expression->call.args);
      }
      free(expression);
      return;
    }
    case ARRAY_EXPR: {
      if (expression->array.size > 0) {
        for (int i = 0; i < expression->array.size; ++i) {
          free_expression(parser, expression->array.elements[i]);
        }
        free(expression->array.elements);
      }
      free(expression);
      return;
    }
    case INDEXING_EXPR: {
      free_expression(parser, expression->indexing.object);
      free_expression(parser, expression->indexing.index);
      free(expression);
      return;
    }
    case SLICING_EXPR: {
      free_expression(parser, expression->slicing.object);
      free_expression(parser, expression->slicing.index0);
      free_expression(parser, expression->slicing.index1);
      free(expression);
      return;
    }
    case ELEMENT_ASSIGN_EXPR: {
      free_expression(parser, expression->element_assign.object);
      free_expression(parser, expression->element_assign.index);
      free_expression(parser, expression->element_assign.right);
      free(expression);
      return;
    }
    case GET_EXPR: {
      free_expression(parser, expression->get.object);
      free(expression);
      return;
    }
    case SET_EXPR: {
      free_expression(parser, expression->set.object);
      free_expression(parser, expression->set.value);
      free(expression);
      return;
    }
    case CONDITIONAL_EXPR: {
      free_expression(parser, expression->conditional.condition);
      free_expression(parser, expression->conditional.then_exp);
      free_expression(parser, expression->conditional.else_exp);
      free(expression);
      return;
    }
    case VARIABLE_EXPR: {
      free(expression);
      return;
    }
    case THIS_EXPR: {
      free(expression);
      return;
    }
    case SUPER_EXPR: {
      free(expression);
      return;
    }
    case OPERATOR_EXPR: {
      free_expression(parser, expression->operator.left);
      free_expression(parser, expression->operator.right);
      free(expression);
      return;
    }
    case PREFIX_EXPR: {
      free_expression(parser, expression->prefix.right);
      free(expression);
      return;
    }
    case POSTFIX_EXPR: {
      free_expression(parser, expression->postfix.left);
      free(expression);
      return;
    }
    case OR_EXPR: {
      free_expression(parser, expression->or.left);
      free_expression(parser, expression->or.right);
      free(expression);
      return;
    }
    case AND_EXPR: {
      free_expression(parser, expression->and.left);
      free_expression(parser, expression->and.right);
      free(expression);
      return;
    }
    case LITERAL_EXPR: {
      if (IS_OBJ(expression->value)) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("free literal expr: ");
#endif
        free_object(AS_OBJ(expression->value));
      }
      free(expression);
      return;
    }
    case FUNC_EXPR: {
      if (expression->function.parameters != NULL) {
        free(expression->function.parameters);
      }
      free_statement(parser, expression->function.body);
      free(expression);
      return;
    }
    default: {
      return;
    }
  }
}

static bool is_statement(ParseType type) {
  return type >= PRINT_STMT;
}

void free_statement(Parser *parser, Statement *statement) {
  if (statement == NULL)return;
  ParseType type = statement->type;
  switch (type) {
    case PRINT_STMT: {
      free_expression(parser, statement->print_expr);
      free(statement);
      return;
    }
    case RETURN_STMT: {
      free_expression(parser, statement->return_stmt.value);
      free(statement);
      return;
    }
    case EXPRESSION_STMT: {
      free_expression(parser, statement->expression);
      free(statement);
      return;
    }
    case VAR_DECL_STMT: {
      free_expression(parser, statement->var_decl.initializer);
      free(statement);
      return;
    }
    case BLOCK_STMT: {
      for (int i = 0; i < statement->block.stmt_nums; ++i) {
        free_statement(parser, statement->block.statements[i]);
      }
      free(statement->block.statements);
      free(statement);
      return;
    }
    case IF_STMT: {
      free_expression(parser, statement->if_stmt.condition);
      free_statement(parser, statement->if_stmt.then_stmt);
      free_statement(parser, statement->if_stmt.else_stmt);
      free(statement);
      return;
    }
    case WHILE_STMT: {
      free_expression(parser, statement->while_stmt.condition);
      free_statement(parser, statement->while_stmt.body);
      free(statement);
      return;
    }
    case FOR_STMT: {
      free_statement(parser, statement->for_stmt.initializer);
      free_expression(parser, statement->for_stmt.condition);
      free_expression(parser, statement->for_stmt.increment);
      free_statement(parser, statement->for_stmt.body);
      free(statement);
      return;
    }
    case CONTINUE_STMT: {
      free(statement);
      return;
    }
    case BREAK_STMT: {
      free(statement);
      return;
    }
    case FUNC_STMT: {
      free_expression(parser, statement->function_stmt.function);
      free(statement);
      return;
    }
    case IMPORT_STMT: {
      free_expression(parser, statement->import_stmt.as);
      free(statement);
      return;
    }
    case TRY_STMT: {
      free_statement(parser, statement->try_stmt.try_block);
      for (int i = 0; i < statement->try_stmt.catch_nums; ++i) {
        free_statement(parser, statement->try_stmt.catch_stmt[i]);
      }
      free(statement->try_stmt.catch_stmt);
      free_statement(parser, statement->try_stmt.finally_block);
      free(statement);
      return;
    }
    case CATCH_STMT: {
      free_expression(parser, statement->catch_stmt.catch_exception);
      free_expression(parser, statement->catch_stmt.as);
      free_statement(parser, statement->catch_stmt.catch_block);
      free(statement);
      return;
    }
    case THROW_STMT: {
      free_expression(parser, statement->throw_stmt.throwable);
      free(statement);
      return;
    }
    case CLASS_STMT: {
      free_expression(parser, statement->class_stmt.super_class);
      for (size_t i = 0; i < statement->class_stmt.methods_num; ++i) {
        free_statement(parser, statement->class_stmt.methods[i]);
      }
      free(statement->class_stmt.methods);
      for (size_t i = 0; i < statement->class_stmt.static_methods_num; ++i) {
        free_statement(parser, statement->class_stmt.static_methods[i]);
      }
      free(statement->class_stmt.static_methods);
      free(statement);
      return;
    }
    default:free(statement);
      return;
  }
}
void free_statements(Parser *parser, StatementArrayList *statements) {
  if (statements != NULL) {
    for (size_t i = 0; i < statements->size; ++i) {
      free_statement(parser, Statementget_data_arraylist(statements, i));
    }
  }
}
char *print_statements(Parser *parser, StatementArrayList *statements) {
  json_value v;
  v.type = JSON_NULL;
  init_as_array(&v, 0);
  for (size_t i = 0; i < statements->size; i++) {
    json_value *t = tree_json(parser, (ParseTree *) Statementget_data_arraylist(statements, i));
    *insert_json_array(&v, i) = *t;
  }
  char *ast = json_stringify(&v, NULL);
  free_json_value(&v);
  return ast;
}

static json_value *tree_json(Parser *parser, ParseTree *tree) {
  ParseType type = tree->type;
  if (is_statement(type)) {
    return statement_printer(parser, (Statement *) tree);
  } else {
    return expression_printer(parser, (Expression *) tree);
  }
}
char *print_tree(Parser *parser, ParseTree *tree) {
  json_value *v = tree_json(parser, tree);
  if (v == NULL) return NULL;
  char *ast = json_stringify(v, NULL);
  free_json_value(v);
  free(v);
  return ast;
}

Expression *parse_expression(Parser *parser, Precedence precedence) {
  advance(parser);
  ParseFn prefix = get_rule(parser, parser->prev.type)->prefix;
  if (prefix == NULL) {
    error(parser, "Expect expression.");
    return NULL;
  }
  Expression *left = prefix(parser, parser->prev, NULL);
  while (precedence <= precedence_of(parser, parser->current.type)) {
    advance(parser);
    ParseFn infix = get_rule(parser, parser->prev.type)->infix;
    left = infix(parser, parser->prev, left);
  }
  return left;
}

static ParseRule *get_rule(Parser *parser, TokenType type) {
  return &rules[type];
}

static Precedence precedence_of(Parser *parser, TokenType type) {
  return get_rule(parser, type)->precedence;
}

static bool is_right_associative(Parser *parser, TokenType type) {
  if (type == TOKEN_EXPONENT) return true;
  return false;
}

static void advance(Parser *parser) {
  parser->prev = parser->current;
  for (;;) {
    parser->current = scan_token(parser->scanner);
    if (parser->current.type != TOKEN_ERROR) break;
    error_at_current(parser, parser->current.start);
  }
}

static void error_at_current(Parser *parser, const char *msg) {
  error_at(parser, &parser->current, msg);
}

static void error(Parser *parser, const char *msg) {
  error_at(parser, &parser->prev, msg);
}

static void error_at(Parser *parser, Token *token, const char *msg) {
  if (parser->panic_mode) return;
  parser->panic_mode = true;

  fprintf(stderr, "[line %zu] Error", token->line);
  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }
  fprintf(stderr, ": %s\n", msg);
  parser->has_err = true;
}

static Expression *number(Parser *parser, Token token, Expression *left) {
  char *end_ptr;
  errno = 0;
  double value = strtod(token.start, &end_ptr);

  if (end_ptr == token.start) {
    error(parser, "No number conversion could be performed.\n");
    return NULL;
  }
  if (errno == ERANGE) {
    error(parser, "The number is out of range.\n");
    return NULL;
  }
  // check it it's an integer
//  if (value == (double) (int) value) {
//    // Additionally check if within int range
//    if (value >= INT_MIN && value <= INT_MAX) {
//      for (int i = 0; i < token.length; ++i) {
//        if (token.start[i] == '.') {
//          return LITERAL_EXP(NUMBER_VAL(value), token.line);
//        }
//      }
//      return LITERAL_EXP(INT_VAL(value), token.line);
//    }
//  }
  return LITERAL_EXP(NUMBER_VAL(value), token.line);
}

static Expression *nil(Parser *parser, Token token, Expression *left) {
  return LITERAL_EXP(NIL_VAL, token.line);
}

static Expression *boolean_value(Parser *parser, Token token, Expression *left) {
  return LITERAL_EXP(BOOL_VAL(token.type == TOKEN_TRUE ? true : false), token.line);
}

static Expression *this_(Parser *parser, Token token, Expression *left) {
  if (parser->class_depth == 0) {
    error(parser, "Can't use 'this' out of class.\n");
    return NULL;
  }
  if (parser->static_method_depth >= parser->class_depth) {
    error(parser, "Can't use 'this' in static method.");
    return NULL;
  }
  return THIS_EXP(token);
}

static Expression *super_(Parser *parser, Token token, Expression *left) {
  if (parser->class_depth == 0) {
    error(parser, "Can't use 'super' out of class.\n");
    return NULL;
  }
  if (!parser->class_has_super[parser->class_depth - 1]) {
    error(parser, "Can't use 'super' in class without super class.\n");
    return NULL;
  }
  if (parser->static_method_depth >= parser->class_depth) {
    error(parser, "Can't use 'super' in static method.\n");
    return NULL;
  }
  consume(parser, TOKEN_DOT, "Expect '.' after 'super'\n");
  consume(parser, TOKEN_IDENTIFIER, "Expect superclass method name.\n");
  Expression *super = SUPER_EXP(token);
  return GET_EXP(super, parser->prev);
}

static Expression *string(Parser *parser, Token token, Expression *left) {
  // string is escaped and stored in this char array in the token
  StringArrayList *s = token.escaped_string;
  size_t size = s->size;
  char *str = malloc(size + 1);
  memcpy(str, s->data, size);
  str[size] = '\0';
  Stringfree_arraylist(s);
#ifdef DEBUG_TRACE_EXECUTION
  printf("   parsing string:[%s] - (%lu)\n", str, strlen(str));
#endif
  ObjString *os = malloc(sizeof(ObjString));
  os->obj = (Obj) {.type=OBJ_STRING};
  os->length = size;
  os->string = str;
  //printf("%s",os->string);
  // TODO string escape and utf8 support
  return LITERAL_EXP(OBJ_VAL(os), token.line);
}

static Expression *grouping(Parser *parser, Token token, Expression *left) {
  /**
   * Parses parentheses used to group an expression, like "a * (b + c)".
   */
  Expression *e = expression(parser);
  consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
  return e;
}

static Expression *unary(Parser *parser, Token token, Expression *left) {
  /**
   * Generic prefix parse function for an unary arithmetic operator.
   * Parses prefix unary "-", "+", "~", and "!" expressions.
   */
  Expression *right = parse_expression(parser, PREC_PREFIX);
  return PREFIX_EXP(token.type, right, token.line);
}

static Expression *binary(Parser *parser, Token token, Expression *left) {
  /**
   *  Generic infix parse function for a binary arithmetic operator.
   *  The only difference when parsing, "+", "-", "*", "/", and "^" is precedence and
   *  associativity, so we can use a single parse function for all of those.
   */
  Precedence p = precedence_of(parser, token.type);
  bool right_associative = is_right_associative(parser, token.type);
  // left-associative: precedence + 1
  Expression *right = parse_expression(parser, right_associative ? p : (Precedence) (p + 1));
  return OPERATOR_EXP(left, token.type, right, token.line);
}

static Expression *conditional(Parser *parser, Token token, Expression *left) {
  /**
   * Parse function for the condition or ternary operator, like "a ? b : c".
   */
  Expression *then_exp = expression(parser);
  consume(parser, TOKEN_COLON, "Expect ':' after expression.");
  // right-associative
  Expression *else_exp = parse_expression(parser, PREC_CONDITIONAL);
  return CONDITIONAL_EXP(left, then_exp, else_exp);
}

static Expression *identifier(Parser *parser, Token token, Expression *left) {
  /**
   * Simple parse function for a named variable like "abc".
   */
  return ID_EXP(token);
}

static Expression *fun_or_call(Parser *parser, Token token, Expression *left) {
  // if left is not identifier, then this expression must be call expression.
//  if (left->type != VARIABLE_EXPR) {
//    Expression *e = call(parser, token, left);
//    if (e->call.arg_num > 255) {
//      error(parser, "Can't call with more than 255 arguments");
//    }
//    return e;
//  }
  // assume this expression a call
  Expression *call_expr = call(parser, token, left);

  // if next token is '{', then convert it to function expression, otherwise return call
  if (!match(parser, TOKEN_LEFT_BRACE)) {
    if (call_expr->call.arg_num > 255) {
      error(parser, "Can't call with more than 255 arguments");
    }
    return call_expr;
  }

  size_t pn = call_expr->call.arg_num;
  if (call_expr->call.arg_num > 255) {
    error(parser, "Can't have more than 255 parameters");
  }
  Token *params = NULL;
  if (pn != 0) {
    params = malloc(sizeof(Token) * pn);
    for (int i = 0; i < call_expr->call.arg_num; ++i) {
      Expression *ex = call_expr->call.args[i];
      if (ex->type != VARIABLE_EXPR) {
        error_at(parser, &parser->prev, "Function parameter must be identifier.");
        params[i] = (Token) {.type=TOKEN_ERROR};
      } else {
        params[i] = ex->variable.name;
      }
    }
  }
  parser->func_depth++;
  Statement *body = block(parser);
  parser->func_depth--;
  Expression *func = FUNC_EXP(pn, params, body);
  free_expression(parser, call_expr);
  return func;
}

static Expression *lambda(Parser *parser, Token token, Expression *left) {
  consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after lambda.");
  TokenArrayList *params = Tokennew_arraylist(10);
  if (!match(parser, TOKEN_RIGHT_PAREN)) {
    for (;;) {
      consume(parser, TOKEN_IDENTIFIER, "Expect identifier after lambda.");
      Tokenappend_arraylist(params, parser->prev);
      if (!match(parser, TOKEN_COMMA)) {
        break;
      }
    }
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after lambda.");
  }
  size_t params_num = params->size;
  Token *parameters = NULL;
  if (params_num > 0) {
    parameters = malloc(sizeof(Token) * params_num);
    memcpy(parameters, params->data, sizeof(Token) * params_num);
  }
  consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before lambda body.");
  parser->func_depth++;
  Statement *body = block(parser);
  parser->func_depth--;
  Expression *func = FUNC_EXP(params_num, parameters, body);
  func->line = token.line;
  Tokenfree_arraylist(params);
  return func;
}

static Expression *call(Parser *parser, Token token, Expression *left) {
  /**
   * Parse function to parse a function call like "a(b, c, d)".
   */
  // Parse the comma-separated arguments until hit, ")".
  LinkedList *args = new_linked_list();

  // There may be no arguments at all.
  if (!match(parser, TOKEN_RIGHT_PAREN)) {
    for (;;) {
      append_list(args, expression(parser));
      if (!match(parser, TOKEN_COMMA)) {
        break;
      }
    }
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after call.");
  }

  if (list_size(args) == 0) {
    free_linked_list(args, NULL);
    return CALL_EXP(left, 0, NULL, token.line);
  } else {
    int size = list_size(args);
    Expression **arg_list = malloc(sizeof(Expression *) * size);
    LinkedListNode *list_node = head_of_list(args);
    LinkedListNode *head = list_node;
    int i = 0;
    do {
      arg_list[i++] = data_of_node_linked_list(list_node);
      list_node = next_node_linked_list(list_node);
    } while (list_node != head);
    free_linked_list(args, NULL);
    return CALL_EXP(left, size, arg_list, token.line);
  }
}

static Expression *assign(Parser *parser, Token token, Expression *left) {
  /**
   * Parses assignment expressions like "a = b".
   * The left side of an assignment expression must be a simple name like "a",
   * and expressions are right-associative. (In other words, "a = b = c" is parsed as "a = (b = c)").
   */
  Expression *right = parse_expression(parser, PREC_ASSIGNMENT);
  if (left->type == VARIABLE_EXPR) {
    Token t = left->variable.name;
    free(left);
    return ASSIGN_EXP(t, right);
  } else if (left->type == GET_EXPR) {
    Expression *obj = left->get.object;
    Token t = left->get.name;
    if (obj->type == SUPER_EXPR) {
      error_at(parser, &t, "Can't assign super class method.");
      return NULL;
    }
    free(left);
    return SET_EXP(obj, t, right);
  } else if (left->type == INDEXING_EXPR) {
    Expression *object = left->indexing.object;
    Expression *index = left->indexing.index;
    free(left);
    return ELEMENT_ASSIGN_EXP(object, index, right, token.line);
  }
  error_at(parser,
           &token,
           "The left-hand side of an assignment must be a variable or get expression or indexing expression.");
  return NULL;
}

static Expression *postfix(Parser *parser, Token token, Expression *left) {
  /**
   * Generic infix parse function for an unary arithmetic operator.
   */
  return POSTFIX_EXP(token.type, left, token.line);
}

static Expression *getter(Parser *parser, Token token, Expression *left) {
  advance(parser);
  return GET_EXP(left, parser->prev);
}

static Expression *or(Parser *parser, Token token, Expression *left) {
  Precedence p = precedence_of(parser, token.type);
  // left-associative: precedence + 1
  Expression *right = parse_expression(parser, (Precedence) (p + 1));
  return OR_EXP(left, right, token.line);
}

static Expression *and(Parser *parser, Token token, Expression *left) {
  Precedence p = precedence_of(parser, token.type);
  // left-associative: precedence + 1
  Expression *right = parse_expression(parser, (Precedence) (p + 1));
  return AND_EXP(left, right, token.line);
}

static Expression *array(Parser *parser, Token token, Expression *left) {
  // "[" arguments? "]";
  // arguments -> expression ( "," expression )* ;
  LinkedList *elements = new_linked_list();
  if (!match(parser, TOKEN_RIGHT_BRACKET)) {
    for (;;) {
      append_list(elements, expression(parser));
      if (!match(parser, TOKEN_COMMA)) {
        break;
      }
    }
    consume(parser, TOKEN_RIGHT_BRACKET, "Expect ']' after array.");
  }

  if (list_size(elements) == 0) {
    free_linked_list(elements, NULL);
    return ARR_EXP(0, NULL, token.line);
  } else {
    int size = list_size(elements);
    Expression **arg_list = malloc(sizeof(Expression *) * size);
    LinkedListNode *list_node = head_of_list(elements);
    LinkedListNode *head = list_node;
    int i = 0;
    do {
      arg_list[i++] = data_of_node_linked_list(list_node);
      list_node = next_node_linked_list(list_node);
    } while (list_node != head);
    free_linked_list(elements, NULL);
    return ARR_EXP(size, arg_list, token.line);
  }
}

static Expression *indexing_or_slicing(Parser *parser, Token token, Expression *left) {
  // [i], [:i], [:], [i:], [i:j]

  if (match(parser, TOKEN_RIGHT_BRACKET)) {
    error(parser, "Expect index or slice expression after '['.");
    return NULL;
  }

  if (match(parser, TOKEN_COLON)) {
    // [:i], [:] ,slicing
    if (match(parser, TOKEN_RIGHT_BRACKET)) {
      // [:]
      return SLICING_EXP(left, NULL, NULL, token.line);
    }
    Expression *index1 = expression(parser);
    consume(parser, TOKEN_RIGHT_BRACKET, "Expect ']' after slicing.");
    return SLICING_EXP(left, NULL, index1, token.line);
  } else {
    //[i
    Expression *index0 = expression(parser);
    if (match(parser, TOKEN_RIGHT_BRACKET)) {
      // [i]
      return INDEXING_EXP(left, index0, token.line);
    }
    consume(parser, TOKEN_COLON, "Expect ':' in slicing before ']'.");
    //[i:
    if (match(parser, TOKEN_RIGHT_BRACKET)) {
      // [i:]
      return SLICING_EXP(left, index0, NULL, token.line);
    } else {
      // [i:j]
      Expression *index1 = expression(parser);
      consume(parser, TOKEN_RIGHT_BRACKET, "Expect ']' after slicing.");
      return SLICING_EXP(left, index0, index1, token.line);
    }
  }
}

static Statement *declaration(Parser *parser) {
  /**
   *
   * declaration -> classDecl
   *             | varDecl
   *             | funDecl
   *             | statement ;
   *
   * statement -> exprStmt
   *           | printStmt
   *           | ifStmt
   *           | whileStmt
   *           | forStmt
   *           | continueStmt
   *           | breakStmt
   *           | returnStmt
   *           | block
   *           | importStmt
   *           | tryStmt
   *           | throwStmt;
   *
   * block -> "{" declaration* "}" ;
   * funDecl -> "fun" function ;
   * function -> IDENTIFIER "(" parameters? ")" block ;
   * classDecl -> "class" IDENTIFIER ( "<" IDENTIFIER )? "{" methodDecl* "}" ;
   * tryStmt -> "try" block ( "catch" "(" IDENTIFIER (as IDENTIFIER)? ")" block )* ( "finally" block )? ;
   *
   */
  Statement *stmt = NULL;
  if (match(parser, TOKEN_CLASS)) {
    stmt = class_declaration(parser);
  } else if (match(parser, TOKEN_VAR)) {
    stmt = var_declaration(parser);
  } else if (match(parser, TOKEN_FUN)) {
    stmt = fun_declaration(parser, false, false);
  } else {
    stmt = statement(parser);
  }
  if (parser->panic_mode) {
    synchronize(parser);
  }
  return stmt;
}

static Statement *statement(Parser *parser) {
  if (match(parser, TOKEN_PRINT)) {
    return print_statement(parser);
  } else if (match(parser, TOKEN_LEFT_BRACE)) {
    return block(parser);
  } else if (match(parser, TOKEN_IF)) {
    return if_statement(parser);
  } else if (match(parser, TOKEN_WHILE)) {
    return while_statement(parser);
  } else if (match(parser, TOKEN_FOR)) {
    return for_statement(parser);
  } else if (match(parser, TOKEN_CONTINUE)) {
    return continue_statement(parser);
  } else if (match(parser, TOKEN_BREAK)) {
    return break_statement(parser);
  } else if (match(parser, TOKEN_RETURN)) {
    return return_statement(parser);
  } else if (match(parser, TOKEN_IMPORT)) {
    return import_statement(parser);
  } else if (match(parser, TOKEN_TRY)) {
    return try_statement(parser);
  } else if (match(parser, TOKEN_THROW)) {
    return throw_statement(parser);
  } else {
    return expression_statement(parser);
  }
}

static Statement *block(Parser *parser) {
  StatementArrayList *stmts = Statementnew_arraylist(256);
  while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
    Statementappend_arraylist(stmts, declaration(parser));
  }
  consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after block.");
  size_t line = parser->prev.line;
  size_t size = stmts->size;
  Statement **ret = malloc(sizeof(Statement *) * size);
  for (size_t i = 0; i < size; ++i) {
    ret[i] = Statementget_data_arraylist(stmts, i);
  }
  Statementfree_arraylist(stmts);
  return BLOCK_STMTS(ret, size, line);
}

static Statement *if_statement(Parser *parser) {
  consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  Expression *condition = expression(parser);
  consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
  Statement *then_statement = statement(parser);
  Statement *else_stmt = NULL;
  if (match(parser, TOKEN_ELSE)) {
    else_stmt = statement(parser);
  }
  return IF_STMTS(condition, then_statement, else_stmt);
}

static Statement *while_statement(Parser *parser) {
  consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  Expression *condition = expression(parser);
  consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after condition.");
  parser->loop_depth++;
  Statement *body = statement(parser);
  Statement *while_stmt = WHILE_STMTS(condition, body);
  parser->loop_depth--;
  return while_stmt;
}

static Statement *for_statement(Parser *parser) {
  /**
   * TODO THIS is what I want. Consider supporting it.
   *
   * for outer ( i = 0; i < M; ++i )
   * for ( j = 0; j < N; ++j )
   *   if ( l1[ i ] == l2[ j ] )
   *     break outer;
   */
  consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  Statement *initializer = NULL;
  if (match(parser, TOKEN_SEMICOLON)) {
    // no initializer
  } else if (match(parser, TOKEN_VAR)) {
    initializer = var_declaration(parser);
  } else {
    initializer = expression_statement(parser);
  }
  Expression *condition = NULL;
  if (match(parser, TOKEN_SEMICOLON)) {
    // no condition
  } else {
    condition = expression(parser);
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after condition.");
  }

  Expression *increment = NULL;
  if (match(parser, TOKEN_RIGHT_PAREN)) {
    // no increment
  } else {
    increment = expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after for increment.");
  }
  parser->loop_depth++;
  Statement *body = statement(parser);
  Statement *for_stmt = FOR_STMTS(initializer, condition, increment, body);
  parser->loop_depth--;
  return for_stmt;
}

static Statement *continue_statement(Parser *parser) {
  if (parser->loop_depth == 0) error(parser, "'continue' must be inside a loop.");
  Token prev = parser->prev;
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'continue'");
  return CONTINUE_STMTS(prev);
}

static Statement *break_statement(Parser *parser) {
  if (parser->loop_depth == 0) error(parser, "'break' must be inside a loop.");
  Token prev = parser->prev;
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'break'");
  return BREAK_STMTS(prev);
}

static Statement *return_statement(Parser *parser) {
  if (parser->func_depth == 0) {
    error(parser, "'return' must be inside a function.");
  }
  Token keyword = parser->prev;
  if (match(parser, TOKEN_SEMICOLON)) {
    return RETURN_STMTS(keyword, NULL);
  } else {
    Statement *return_stmt = RETURN_STMTS(parser->prev, expression(parser));
    consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'return'");
    return return_stmt;
  }
}

static Statement *import_statement(Parser *parser) {
  consume(parser, TOKEN_STRING, "Expect want lib path.");
  Token lib = parser->prev;
  Expression *as = NULL;
  consume(parser, TOKEN_AS, "Expect 'as' after 'want'.");
  consume(parser, TOKEN_IDENTIFIER, "Expect as alias.");
  as = ID_EXP(parser->prev);
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after 'want'.");
  return IMPORT_STMTS(lib, as);
}

static Statement *throw_statement(Parser *parser) {
  Token kw = parser->prev;
  Expression *throwable = expression(parser);
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after throw.");
  return THROW_STMTS(kw, throwable);
}

static Statement *try_statement(Parser *parser) {
  consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after try.");
  Statement *try_block = block(parser);

  consume(parser, TOKEN_CATCH, "Expect 'catch' after try block.");
  StatementArrayList *catch_stmts = Statementnew_arraylist(8);
  Statement *catch_stmt = catch_statement(parser);
  Statementappend_arraylist(catch_stmts, catch_stmt);

  while (match(parser, TOKEN_CATCH)) {
    Statementappend_arraylist(catch_stmts, catch_statement(parser));
  }

  Statement *finally_block = NULL;
  if (match(parser, TOKEN_FINALLY)) {
    consume(parser, TOKEN_LEFT_BRACE, "Expect '{' after finally.");
    finally_block = block(parser);
  }
  size_t catch_nums = catch_stmts->size;
  Statement **catches = malloc(sizeof(Statement *) * catch_nums);
  for (int i = 0; i < catch_nums; i++) {
    catches[i] = catch_stmts->data[i];
  }
  Statementfree_arraylist(catch_stmts);
  return TRY_STMTS(try_block, catches, finally_block, catch_nums);
}

static Statement *catch_statement(Parser *parser) {
  consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after catch.");
  consume(parser, TOKEN_IDENTIFIER, "Expect exception identifier after '(',");
  Expression *id = NULL;
  Expression *as_ = NULL;
  Token t1 = parser->prev;
  if (match(parser, TOKEN_AS)) {
    consume(parser, TOKEN_IDENTIFIER, "Expect exception identifier after 'as',");
    id = ID_EXP(t1);
    as_ = ID_EXP(parser->prev);
  } else {
    as_ = ID_EXP(t1);
  }
  consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after catch.");
  consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before catch block.");
  Statement *catch_block = block(parser);
  return CATCH_STMTS(catch_block, id, as_);

}

static Statement *class_declaration(Parser *parser) {
  consume(parser, TOKEN_IDENTIFIER, "Expect class name.");
  Token name = parser->prev;
  Expression *super_class = NULL;
  bool class_nesting_overflow = false;
  if (parser->class_depth >= MAX_CLASS_NESTING) {
    class_nesting_overflow = true;
    char msg[50];
    sprintf(msg, "Can't nesting class over %d", MAX_CLASS_NESTING);
    error(parser, msg);
  } else {
    parser->class_depth++;
  }

  if (match(parser, TOKEN_LESS)) {
    // "< superclassName"
    consume(parser, TOKEN_IDENTIFIER, "Expect super class name.");
    if (parser->prev.length == name.length && memcmp(parser->prev.start, name.start, name.length) == 0) {
      error(parser, "Unable to inherit its own class.");
    }

    super_class = ID_EXP(parser->prev);
    if (!class_nesting_overflow) {
      parser->class_has_super[parser->class_depth - 1] = true;
    }
  } else {
    if (!class_nesting_overflow) {
      parser->class_has_super[parser->class_depth - 1] = false;
    }
  }

  consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before class body.");

  StatementArrayList *methods = Statementnew_arraylist(8);
  StatementArrayList *static_methods = Statementnew_arraylist(8);
  while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)) {
    bool is_static_method = match(parser, TOKEN_STATIC);
    Statement *method = fun_declaration(parser, true, is_static_method);
    if (is_static_method) {
      Statementappend_arraylist(static_methods, method);
    } else {
      Statementappend_arraylist(methods, method);
    }
  }
  consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
  if (!class_nesting_overflow) {
    parser->class_depth--;
  }

  size_t methods_num = methods->size;
  Statement **ms = methods_num == 0 ? NULL : malloc(sizeof(Statement *) * methods_num);
  size_t static_methods_num = static_methods->size;
  Statement **sms = static_methods_num == 0 ? NULL : malloc(sizeof(Statement *) * static_methods_num);
  for (size_t i = 0; i < methods_num; ++i) {
    ms[i] = Statementget_data_arraylist(methods, i);
  }

  for (size_t i = 0; i < static_methods_num; ++i) {
    sms[i] = Statementget_data_arraylist(static_methods, i);
  }

  Statementfree_arraylist(methods);
  Statementfree_arraylist(static_methods);

  return CLASS_DECL_STMTS(name, super_class, ms, methods_num, sms, static_methods_num, name.line);
}

static Statement *var_declaration(Parser *parser) {
  consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
  Token name = parser->prev;
  Expression *initializer = NULL;
  if (match(parser, TOKEN_EQUAL)) {
    initializer = expression(parser);
  }
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
  return VAR_DECL_STMTS(name, initializer, name.line);
}

static Statement *fun_declaration(Parser *parser, bool is_method, bool is_static) {
  consume(parser, TOKEN_IDENTIFIER, is_method ? "Expect method name." : "Expect function name.");
  Token name = parser->prev;
  consume(parser, TOKEN_LEFT_PAREN, is_method ? "Expect '(' after method name." : "Expect '(' after function name.");

  size_t params_num = 0;
  // parameters
  TokenArrayList *params = Tokennew_arraylist(10);
  if (!match(parser, TOKEN_RIGHT_PAREN)) {
    for (;;) {
      consume(parser, TOKEN_IDENTIFIER, is_method ? "Expect method parameter." : "Expect function parameter.");
      Token p = parser->prev;
      Tokenappend_arraylist(params, p);
      if (!match(parser, TOKEN_COMMA)) {
        break;
      }
    }
    consume(parser,
            TOKEN_RIGHT_PAREN,
            is_method ? "Expect ')' after method parameters." : "Expect ')' after function parameters.");
  }
  consume(parser, TOKEN_LEFT_BRACE, is_method ? "Expect '{' before method body." : "Expect '{' before function body.");
  parser->func_depth++;
  if (is_static) parser->static_method_depth++;
  Statement *body = block(parser);
  parser->func_depth--;
  if (is_static) parser->static_method_depth--;
  params_num = params->size;
  Token *ps = NULL;
  if (params_num != 0) {
    ps = malloc(sizeof(Token) * params_num);
    memcpy(ps, params->data, params_num * sizeof(Token));
  }
  Tokenfree_arraylist(params);

  Expression *func = FUNC_EXP(params_num, ps, body);

  return FUNC_STMTS(name, func);
}

static Statement *print_statement(Parser *parser) {
  Expression *e = expression(parser);
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after value.");
  return PRINT_STMTS(e, parser->prev.line;);
}

static Statement *expression_statement(Parser *parser) {
  Expression *e = expression(parser);
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
  return EXPR_STMTS(e, parser->prev.line;);
}

static Expression *expression(Parser *parser) {
  return parse_expression(parser, PREC_ASSIGNMENT);
}

static void consume(Parser *parser, TokenType type, const char *msg) {
  if (parser->current.type == type) {
    if (type != TOKEN_EOF) {
      advance(parser);
    }
    return;
  }
  error_at_current(parser, msg);
}

static bool check(Parser *parser, TokenType type) {
  return parser->current.type == type;
}

static bool match(Parser *parser, TokenType type) {
  if (!check(parser, type)) return false;
  if (type != TOKEN_EOF) {
    advance(parser);
  }
  return true;
}

static void synchronize(Parser *parser) {
  parser->panic_mode = false;
  // Skip tokens util reach something like a statement boundary.

  while (parser->prev.type != TOKEN_EOF) {
    if (parser->prev.type == TOKEN_SEMICOLON)return;
    switch (parser->current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:return;
      default:;
    }
    advance(parser);
  }
}

static json_value *init_json_obj_with(char *type) {
  json_value *e = malloc(sizeof(json_value));
  init_as_object(e);
  set_json_str_sk(set_json_object_sk(e, "type"), type);
  return e;
}

static json_value *statement_printer(Parser *parser, Statement *statement) {
  if (statement == NULL) return new_null();
  ParseType type = statement->type;
  switch (type) {
    case PRINT_STMT: {
      json_value *e = init_json_obj_with("print_stmt");
      json_value *exp = set_json_object_sk(e, "expression");
      *exp = *expression_printer(parser, statement->print_expr);
      return e;
    }
    case EXPRESSION_STMT: {
      json_value *e = init_json_obj_with("expression_stmt");
      json_value *exp = set_json_object_sk(e, "expression");
      *exp = *expression_printer(parser, statement->print_expr);
      return e;
    }
    case VAR_DECL_STMT: {
      json_value *e = init_json_obj_with("var_decl_stmt");

      char name[statement->var_decl.name.length + 1];
      memcpy(name, statement->var_decl.name.start, statement->var_decl.name.length);
      name[statement->var_decl.name.length] = '\0';
      set_json_str_sk(set_json_object_sk(e, "name"), name);

      json_value *exp = set_json_object_sk(e, "initializer");
      *exp = *expression_printer(parser, statement->var_decl.initializer);
      return e;
    }
    case BLOCK_STMT: {
      json_value *e = init_json_obj_with("block_stmt");

      json_value *arr = new_null();
      init_as_array(arr, 0);

      for (int i = 0; i < statement->block.stmt_nums; ++i) {
        Statement *s = statement->block.statements[i];
        json_value *a = insert_json_array(arr, i);
        *a = *statement_printer(parser, s);
      }
      *set_json_object_sk(e, "statements") = *arr;
      return e;
    }
    case TRY_STMT: {
      json_value *e = init_json_obj_with("try_stmt");

      json_value *try_block = set_json_object_sk(e, "try_block");
      *try_block = *statement_printer(parser, statement->try_stmt.try_block);

      json_value *arr = new_null();
      init_as_array(arr, 0);

      for (int i = 0; i < statement->try_stmt.catch_nums; ++i) {
        Statement *ca = statement->try_stmt.catch_stmt[i];
        json_value *v = insert_json_array(arr, i);
        *v = *statement_printer(parser, ca);
      }
      *set_json_object_sk(e, "catches") = *arr;
      json_value *finally_block = set_json_object_sk(e, "finally_block");
      *finally_block = *statement_printer(parser, statement->try_stmt.finally_block);

      return e;
    }
    case CATCH_STMT: {
      json_value *e = init_json_obj_with("catch_stmt");
      json_value *catch_exception = set_json_object_sk(e, "catch_exception");
      *catch_exception = *expression_printer(parser, statement->catch_stmt.catch_exception);
      json_value *as = set_json_object_sk(e, "as");
      *as = *expression_printer(parser, statement->catch_stmt.as);
      json_value *catch_block = set_json_object_sk(e, "catch_block");
      *catch_block = *statement_printer(parser, statement->catch_stmt.catch_block);
      return e;
    }
    case THROW_STMT: {
      json_value *e = init_json_obj_with("throw_stmt");
      json_value *throwable = set_json_object_sk(e, "throwable");
      *throwable = *expression_printer(parser, statement->throw_stmt.throwable);
      return e;
    }
    case IF_STMT: {
      json_value *e = init_json_obj_with("if_stmt");
      json_value *condition = set_json_object_sk(e, "condition");
      *condition = *expression_printer(parser, statement->if_stmt.condition);
      json_value *then_stmt = set_json_object_sk(e, "then_stmt");
      *then_stmt = *statement_printer(parser, statement->if_stmt.then_stmt);
      json_value *else_stmt = set_json_object_sk(e, "else_stmt");
      *else_stmt = *statement_printer(parser, statement->if_stmt.else_stmt);
      return e;
    }
    case WHILE_STMT: {
      json_value *e = init_json_obj_with("while_stmt");
      json_value *condition = set_json_object_sk(e, "condition");
      *condition = *expression_printer(parser, statement->while_stmt.condition);

      json_value *body = set_json_object_sk(e, "body");
      *body = *statement_printer(parser, statement->while_stmt.body);
      return e;
    }
    case FOR_STMT: {
      json_value *e = init_json_obj_with("for_stmt");
      json_value *initializer = set_json_object_sk(e, "initialzer");
      *initializer = *statement_printer(parser, statement->for_stmt.initializer);

      json_value *condition = set_json_object_sk(e, "condition");
      *condition = *expression_printer(parser, statement->for_stmt.condition);

      json_value *increment = set_json_object_sk(e, "increment");
      *increment = *expression_printer(parser, statement->for_stmt.increment);

      json_value *body = set_json_object_sk(e, "body");
      *body = *statement_printer(parser, statement->for_stmt.body);

      return e;
    }
    case BREAK_STMT: {
      json_value *e = init_json_obj_with("break_stmt");
      return e;
    }
    case CONTINUE_STMT: {
      json_value *e = init_json_obj_with("continue_stmt");
      return e;
    }
    case FUNC_STMT: {
      json_value *e = init_json_obj_with("func_stmt");

      json_value *n = set_json_object_sk(e, "name");
      char name[statement->function_stmt.name.length + 1];
      memcpy(name, statement->function_stmt.name.start, statement->function_stmt.name.length);
      name[statement->function_stmt.name.length] = '\0';
      set_json_str_sk(n, name);

      json_value *func = set_json_object_sk(e, "function");
      *func = *expression_printer(parser, statement->function_stmt.function);
      return e;
    }
    case RETURN_STMT: {
      json_value *e = init_json_obj_with("return_stmt");
      json_value *exp = set_json_object_sk(e, "value");
      *exp = *expression_printer(parser, statement->return_stmt.value);
      return e;
    }
    case IMPORT_STMT: {
      json_value *e = init_json_obj_with("import_stmt");
      json_value *lib = set_json_object_sk(e, "lib");
      char name[statement->import_stmt.lib.length + 1 - 2];
      memcpy(name, statement->import_stmt.lib.start + 1, statement->import_stmt.lib.length - 1);
      name[statement->import_stmt.lib.length - 2] = '\0';
      set_json_str_sk(lib, name);

      json_value *as = set_json_object_sk(e, "as");
      *as = *expression_printer(parser, statement->import_stmt.as);
      return e;
    }
    case CLASS_STMT: {
      json_value *e = init_json_obj_with("class_decl_stmt");
      json_value *n = set_json_object_sk(e, "name");
      char name[statement->class_stmt.name.length + 1];
      memcpy(name, statement->class_stmt.name.start, statement->class_stmt.name.length);
      name[statement->class_stmt.name.length] = '\0';
      set_json_str_sk(n, name);

      json_value *super = set_json_object_sk(e, "super_class");
      *super = *expression_printer(parser, statement->class_stmt.super_class);

      json_value *arr = new_null();
      init_as_array(arr, 0);

      for (size_t i = 0; i < statement->class_stmt.methods_num; ++i) {
        Statement *s = statement->class_stmt.methods[i];
        json_value *a = insert_json_array(arr, i);
        *a = *statement_printer(parser, s);
      }
      *set_json_object_sk(e, "methods") = *arr;

      json_value *sarr = new_null();
      init_as_array(sarr, 0);

      for (size_t i = 0; i < statement->class_stmt.static_methods_num; ++i) {
        Statement *s = statement->class_stmt.static_methods[i];
        json_value *a = insert_json_array(sarr, i);
        *a = *statement_printer(parser, s);
      }
      *set_json_object_sk(e, "static_methods") = *sarr;

      return e;
    }
    default:return new_null();
  }
}

static json_value *expression_printer(Parser *parser, Expression *expression) {
  if (expression == NULL) return new_null();
  ParseType type = expression->type;
  switch (type) {
    case ASSIGN_EXPR: {
      json_value *e = init_json_obj_with("assign");
      json_value *n = set_json_object_sk(e, "name");
      char name[expression->assign.name.length + 1];
      memcpy(name, expression->assign.name.start, expression->assign.name.length);
      name[expression->assign.name.length] = '\0';
      set_json_str_sk(n, name);

      json_value *r = set_json_object_sk(e, "right");
      json_value *right = expression_printer(parser, expression->assign.right);
      *r = right != NULL ? *right : *new_null();
      return e;
    }
    case ARRAY_EXPR: {
      json_value *e = init_json_obj_with("array");
      json_value *r = set_json_object_sk(e, "elements");
      if (expression->array.size == 0) {
        *r = *new_null();
      } else {
        json_value *args = new_null();
        init_as_array(args, 0);
        for (int i = 0; i < expression->array.size; ++i) {
          *insert_json_array_last(args) = *expression_printer(parser, expression->array.elements[i]);
        }
        *r = *args;
      }
      return e;
    }
    case INDEXING_EXPR: {
      json_value *e = init_json_obj_with("indexing");
      json_value *o = set_json_object_sk(e, "object");
      *o = *expression_printer(parser, expression->indexing.object);
      json_value *i = set_json_object_sk(e, "index");
      *i = *expression_printer(parser, expression->indexing.index);
      return e;
    }
    case ELEMENT_ASSIGN_EXPR: {
      json_value *e = init_json_obj_with("element_assign");
      json_value *o = set_json_object_sk(e, "object");
      *o = *expression_printer(parser, expression->element_assign.object);
      json_value *i = set_json_object_sk(e, "index");
      *i = *expression_printer(parser, expression->element_assign.index);
      json_value *r = set_json_object_sk(e, "right");
      *r = *expression_printer(parser, expression->element_assign.right);
      return e;
    }
    case SLICING_EXPR: {
      json_value *e = init_json_obj_with("slicing");
      json_value *o = set_json_object_sk(e, "object");
      *o = *expression_printer(parser, expression->slicing.object);

      json_value *index0 = set_json_object_sk(e, "index0");
      *index0 = *expression_printer(parser, expression->slicing.index0);

      json_value *index1 = set_json_object_sk(e, "index1");
      *index1 = *expression_printer(parser, expression->slicing.index1);

      return e;
    }
    case CALL_EXPR: {
      json_value *e = init_json_obj_with("call");
      json_value *f = set_json_object_sk(e, "callable");
      json_value *func = expression_printer(parser, expression->call.function);
      *f = func != NULL ? *func : *new_null();

      json_value *r = set_json_object_sk(e, "args");
      if (expression->call.arg_num == 0) {
        *r = *new_null();
      } else {
        json_value *args = new_null();
        init_as_array(args, 0);
        for (int i = 0; i < expression->call.arg_num; ++i) {
          *insert_json_array_last(args) = *expression_printer(parser, expression->call.args[i]);
        }
        *r = *args;
      }
      return e;
    }
    case FUNC_EXPR: {
      json_value *e = init_json_obj_with("function");
      if (expression->function.parameters != NULL) {
        json_value *params = new_null();
        init_as_array(params, 0);
        for (int i = 0; i < expression->function.param_num; ++i) {
          char name[expression->function.parameters[i].length + 1];
          memcpy(name, expression->function.parameters[i].start, expression->function.parameters[i].length);
          name[expression->function.parameters[i].length] = '\0';
          json_value *p = new_null();
          set_json_str_sk(p, name);
          *insert_json_array_last(params) = *p;
        }
        json_value *r = set_json_object_sk(e, "parameters");
        *r = *params;
      } else {
        json_value *r = set_json_object_sk(e, "parameters");
        *r = *new_null();
      }
      json_value *b = set_json_object_sk(e, "body");
      *b = *statement_printer(parser, expression->function.body);
      return e;
    }
    case GET_EXPR: {
      json_value *e = init_json_obj_with("get");
      json_value *n = set_json_object_sk(e, "name");
      char name[expression->get.name.length + 1];
      memcpy(name, expression->get.name.start, expression->get.name.length);
      name[expression->get.name.length] = '\0';
      set_json_str_sk(n, name);

      json_value *o = set_json_object_sk(e, "object");
      json_value *obj = expression_printer(parser, expression->get.object);
      *o = *obj;
      return e;
    }

    case SET_EXPR: {
      json_value *e = init_json_obj_with("set");
      json_value *n = set_json_object_sk(e, "name");
      char name[expression->set.name.length + 1];
      memcpy(name, expression->set.name.start, expression->set.name.length);
      name[expression->set.name.length] = '\0';
      set_json_str_sk(n, name);

      json_value *o = set_json_object_sk(e, "object");
      json_value *obj = expression_printer(parser, expression->set.object);
      *o = *obj;

      json_value *v = set_json_object_sk(e, "value");
      json_value *val = expression_printer(parser, expression->set.value);
      *v = *val;
      return e;
    }

    case CONDITIONAL_EXPR: {
      json_value *e = init_json_obj_with("conditional");

      json_value *c = set_json_object_sk(e, "condition");
      json_value *con = expression_printer(parser, expression->conditional.condition);
      *c = *con;

      json_value *t = set_json_object_sk(e, "then");
      json_value *th = expression_printer(parser, expression->conditional.then_exp);
      *t = *th;

      json_value *es = set_json_object_sk(e, "else");
      json_value *el = expression_printer(parser, expression->conditional.else_exp);
      *es = *el;

      return e;
    }
    case VARIABLE_EXPR: {
      json_value *e = init_json_obj_with("variable");
      char name[expression->variable.name.length + 1];
      memcpy(name, expression->variable.name.start, expression->variable.name.length);
      name[expression->variable.name.length] = '\0';
      set_json_str_sk(set_json_object_sk(e, "name"), name);
      return e;
    }

    case THIS_EXPR: {
      json_value *e = init_json_obj_with("this");
      set_json_str_sk(set_json_object_sk(e, "name"), "this");
      return e;
    }

    case SUPER_EXPR: {
      json_value *e = init_json_obj_with("super");
      set_json_str_sk(set_json_object_sk(e, "name"), "super");
      return e;
    }
    case OR_EXPR: {
      json_value *e = init_json_obj_with("or");

      json_value *r = set_json_object_sk(e, "right");
      json_value *right = expression_printer(parser, expression->or.right);
      *r = *right;

      json_value *l = set_json_object_sk(e, "left");
      json_value *left = expression_printer(parser, expression->or.left);
      *l = *left;

      return e;
    }

    case AND_EXPR: {
      json_value *e = init_json_obj_with("and");

      json_value *r = set_json_object_sk(e, "right");
      json_value *right = expression_printer(parser, expression->and.right);
      *r = *right;

      json_value *l = set_json_object_sk(e, "left");
      json_value *left = expression_printer(parser, expression->and.left);
      *l = *left;

      return e;
    }

    case OPERATOR_EXPR: {
      json_value *e = malloc(sizeof(json_value));
      init_as_object(e);
      char *t = "unknown-opt";
      switch (expression->operator.operator) {
        case TOKEN_PLUS: {
          t = "plus";
          break;
        }
        case TOKEN_MINUS: {
          t = "minus";
          break;
        }
        case TOKEN_STAR: {
          t = "multiply";
          break;
        }
        case TOKEN_SLASH: {
          t = "divide";
          break;
        }
        case TOKEN_EXPONENT: {
          t = "power";
          break;
        }
        case TOKEN_GREATER: {
          t = "greater";
          break;
        }
        case TOKEN_GREATER_EQUAL: {
          t = "greater_equal";
          break;
        }
        case TOKEN_LESS: {
          t = "less";
          break;
        }
        case TOKEN_LESS_EQUAL: {
          t = "less_equal";
          break;
        }
        case TOKEN_EQUAL_EQUAL: {
          t = "equal";
          break;
        }
        case TOKEN_BANG_EQUAL: {
          t = "not_equal";
          break;
        }
        default: {}
      }
      set_json_str_sk(set_json_object_sk(e, "type"), t);

      json_value *l = set_json_object_sk(e, "left");
      json_value *left = expression_printer(parser, expression->operator.left);
      *l = left != NULL ? *left : *new_null();

      json_value *r = set_json_object_sk(e, "right");
      json_value *right = expression_printer(parser, expression->operator.right);
      *r = right != NULL ? *right : *new_null();

      return e;
    }
    case PREFIX_EXPR: {
      json_value *e = malloc(sizeof(json_value));
      init_as_object(e);
      switch (expression->prefix.operator) {
        case TOKEN_MINUS: {
          set_json_str_sk(set_json_object_sk(e, "type"), "negate");
          break;
        }
        case TOKEN_BANG: {
          set_json_str_sk(set_json_object_sk(e, "type"), "not");
          break;
        }
        default:set_json_str_sk(set_json_object_sk(e, "type"), "unknown-prefix");
      }
      json_value *r = set_json_object_sk(e, "right");
      json_value *right = expression_printer(parser, expression->prefix.right);
      *r = right != NULL ? *right : *new_null();
      return e;
    }
    case POSTFIX_EXPR: {
      json_value *e = malloc(sizeof(json_value));
      init_as_object(e);
      switch (expression->postfix.operator) {
        case TOKEN_BANG: {
          set_json_str_sk(set_json_object_sk(e, "type"), "factorial");
          break;
        }
        default:set_json_str_sk(set_json_object_sk(e, "type"), "unknown-postfix");
      }
      json_value *l = set_json_object_sk(e, "left");
      json_value *left = expression_printer(parser, expression->postfix.left);
      *l = left != NULL ? *left : *new_null();

      return e;
    }

    case LITERAL_EXPR: {
      json_value *e = malloc(sizeof(json_value));
      init_as_object(e);

      switch (expression->value.type) {
        case VAL_NUMBER: {
          set_json_str_sk(set_json_object_sk(e, "type"), "number");
          set_json_number(set_json_object_sk(e, "value"), AS_NUMBER(expression->value));
          break;
        }
        case VAL_NIL: {
          set_json_str_sk(set_json_object_sk(e, "type"), "nil");
          break;
        }
        case VAL_BOOL: {
          set_json_str_sk(set_json_object_sk(e, "type"), "boolean");
          set_json_bool(set_json_object_sk(e, "value"), AS_BOOL(expression->value));
          break;
        }
        case VAL_OBJ: {
          // TODO heap object
          if (IS_STRING(expression->value)) {
            set_json_str_sk(set_json_object_sk(e, "type"), "string");
            set_json_str(set_json_object_sk(e, "value"),
                         AS_STRING_STRING(expression->value),
                         AS_STRING(expression->value)->length);
          } else {
            // TODO other object types
          }
          break;
        }

      }

      return e;
    }

    default:return new_null();
  }
}