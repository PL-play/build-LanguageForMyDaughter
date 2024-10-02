//
// Created by ran on 2024-03-16.
//

#ifndef ZHI_SCANNER_SCANNER_H_
#define ZHI_SCANNER_SCANNER_H_
#include <stddef.h>
#include "common/common.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  // Single-character tokens.
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS,
  TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH,
  TOKEN_STAR, TOKEN_QUESTION, TOKEN_EXPONENT,
  TOKEN_COLON, TOKEN_PERCENT,
  // One or two character tokens.
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL,
  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_RAW_STRING, TOKEN_INTERPOLATION, TOKEN_NUMBER,
  // Keywords.
  TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
  TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
  TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_CONTINUE, TOKEN_BREAK,
  TOKEN_LAMBDA, TOKEN_STATIC, TOKEN_TRAIT, TOKEN_IMPORT, TOKEN_AS,
  TOKEN_TRY, TOKEN_CATCH, TOKEN_FINALLY, TOKEN_THROW,
  // Error, End
  TOKEN_ERROR, TOKEN_EOF
} TokenType;

DECLARE_ARRAY_LIST(char, String)

typedef struct {
  TokenType type;
  const char *start;
  int length;
  size_t line;
  StringArrayList *escaped_string; //
} Token;

typedef struct {
  const char *start;
  const char *current;
  size_t line;

  // Tracks the lexing state when tokenizing interpolated strings.
  //
  // Interpolated strings make the lexer not strictly regular: we don't know
  // whether a ")" should be treated as a RIGHT_PAREN token or as ending an
  // interpolated expression unless we know whether we are inside a string
  // interpolation and how many unmatched "(" there are. This is particularly
  // complex because interpolation can nest:
  //
  //     " ${ " %{ inner } " } "
  //
  // This tracks that state. The parser maintains a stack of ints, one for each
  // level of current interpolation nesting. Each value is the number of
  // unmatched "(" that are waiting to be closed.
  int braces[MAX_INTERPOLATION_NESTING];
  int num_braces;

} Scanner;

void init_scanner(Scanner *scanner, const char *source);
Token scan_token(Scanner *scanner);
#ifdef __cplusplus
}
#endif

#endif //ZHI_SCANNER_SCANNER_H_
