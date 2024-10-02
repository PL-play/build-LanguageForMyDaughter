//
// Created by ran on 2024-03-16.
//
#include <stdlib.h>
#include "compiler/scanner.h"
#include "string.h"
#include <assert.h>
#ifdef ALLOC_TESTING
#include "common/alloc-testing.h"
#endif

DEFINE_ARRAY_LIST(0, String)

#define IS_DIGIT(c) ((c)>='0'&& (c)<='9')
#define IS_DIGIT1_9(c) ((c)>='1' && (c)<='9')
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') ||((c) >= 'A' && (c) <= 'Z') ||(c) == '_')

const char *err_msg[] = {
    "Unexpected character.",
    "Unterminated string.",
    "Invalid number.",
    "Unterminated block comment.",
    "Interpolation may only nest 8 levels deep.",
    "Invalid unicode hex.",
    "Invalid unicode surrogate.",
    "Invalid string escape.",
    "Invalid string char."
};
#define ERR_UC 0
#define ERR_US 1
#define ERR_IN 2
#define ERR_UB 3
#define ERR_ID 4
#define ERR_IU 5
#define ERR_IUS 6
#define ERR_ISE 7
#define ERR_ISC 8

static int is_end(Scanner *);

static Token make_token(Scanner *, TokenType type);

static Token make_token_with_offset(Scanner *, TokenType type, int start_offset, int end_offset);

static Token error_token(Scanner *, const char *msg);

static Token string(Scanner *);

static Token raw_string(Scanner *);

static Token number(Scanner *);

static Token identifier(Scanner *);

static TokenType identifier_type(Scanner *);

static TokenType check_keyword(Scanner *, int start, int length, const char *rest, TokenType type);

static char advance(Scanner *vv);

static int match(Scanner *, char expected);

static void skip_whitespace(Scanner *);

static int skip_comment(Scanner *);

static char peek(Scanner *);

static char peek_next(Scanner *);

static int parse_hex4(const char **p, unsigned int *u);

static void encode_utf8(StringArrayList *s, unsigned int u);

void init_scanner(Scanner *scanner, const char *source) {
    scanner->line = 1;
    scanner->start = source;
    scanner->current = source;
    scanner->num_braces = 0;
}

Token scan_token(Scanner *scanner) {
    skip_whitespace(scanner);
    int ret = skip_comment(scanner);
    if (ret != -1) return error_token(scanner, err_msg[ret]);
    skip_whitespace(scanner);
    scanner->start = scanner->current;
    if (is_end(scanner)) return make_token(scanner, TOKEN_EOF);

    char c = advance(scanner);
    if (IS_ALPHA(c)) return identifier(scanner);
    if (IS_DIGIT(c)) return number(scanner);

    switch (c) {
        // single character tokens
        case '(': return make_token(scanner, TOKEN_LEFT_PAREN);
        case ')': return make_token(scanner, TOKEN_RIGHT_PAREN);
        case '[': return make_token(scanner, TOKEN_LEFT_BRACKET);
        case ']': return make_token(scanner, TOKEN_RIGHT_BRACKET);
        case '{': {
            // If we are inside an interpolated expression, count the unmatched "{".
            if (scanner->num_braces > 0) {
                scanner->braces[scanner->num_braces - 1]++;
                advance(scanner);
                return scan_token(scanner);
            }
            return make_token(scanner, TOKEN_LEFT_BRACE);
        }
        case '}': {
            // If we are inside an interpolated expression, count the "}".
            if (scanner->num_braces > 0 && --scanner->braces[scanner->num_braces - 1] == 0) {
                // This is the final "}", so the interpolation expression has ended.
                // This "}" now begins the next section of the template string.
                scanner->num_braces--;
                //advance(scanner);
                scanner->start = scanner->current;
                return string(scanner);
            }
            return make_token(scanner, TOKEN_RIGHT_BRACE);
        }
        case ';': return make_token(scanner, TOKEN_SEMICOLON);
        case ',': return make_token(scanner, TOKEN_COMMA);
        case '.': return make_token(scanner, TOKEN_DOT);
        case '-': return make_token(scanner, TOKEN_MINUS);
        case '+': return make_token(scanner, TOKEN_PLUS);
        case '/': return make_token(scanner, TOKEN_SLASH);
        case '*': return make_token(scanner, TOKEN_STAR);
        case '?': return make_token(scanner, TOKEN_QUESTION);
        case '^': return make_token(scanner, TOKEN_EXPONENT);
        case ':': return make_token(scanner, TOKEN_COLON);
        case '%': return make_token(scanner, TOKEN_PERCENT);
        // 2-characters tokens
        case '!': return make_token(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return make_token(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return make_token(scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return make_token(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        // string
        case '"': {
            if (peek(scanner) == '"' && peek_next(scanner) == '"') {
                return raw_string(scanner);
            }
            return string(scanner);
        }
        case '&': {
            if (match(scanner, '&')) {
                return make_token(scanner, TOKEN_AND);
            }
        }
        case '|': {
            if (match(scanner, '|')) {
                return make_token(scanner, TOKEN_OR);
            }
        }
    }

    return error_token(scanner, err_msg[ERR_UC]);
}

static int is_end(Scanner *scanner) {
    return *scanner->current == '\0';
}

static Token make_token(Scanner *scanner, TokenType type) {
    return make_token_with_offset(scanner, type, 0, 0);
}

static Token make_token_with_offset(Scanner *scanner, TokenType type, int start_offset, int end_offset) {
    Token token;
    token.type = type;
    token.start = scanner->start + start_offset;
    token.length = (int) (scanner->current + end_offset - token.start);
    token.line = scanner->line;
    return token;
}

static Token error_token(Scanner *scanner, const char *msg) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = msg;
    token.length = (int) strlen(msg);
    token.line = scanner->line;
    return token;
}

static int parse_hex4(const char **p, unsigned int *u) {
    *u = 0;
    for (int i = 0; i < 4; ++i) {
        char ch = *((*p)++);
        *u <<= 4;
        if (ch >= '0' && ch <= '9') *u |= ch - '0';
        else if (ch >= 'a' && ch <= 'f') *u |= ch - 'a' + 10;
        else if (ch >= 'A' && ch <= 'F') *u |= ch - 'A' + 10;
        else return 0;
    }
    return 1;
}

static void encode_utf8(StringArrayList *s, unsigned int u) {
    /**
       * Code Point Range | bits |  byte1  |   byte2   |  byte3  |  byte4
       * U+0000 ~ U+007F	  7     0xxxxxxx
       * U+0080 ~ U+07FF	  11    110xxxxx   10xxxxxx
       * U+0800 ~ U+FFFF	  16	1110xxxx   10xxxxxx	  10xxxxxx
       * U+10000 ~ U+10FFFF 21	11110xxx   10xxxxxx	  10xxxxxx	10xxxxxx
       */
    if (u <= 0x7F) {
        //  0x7F 0111 1111
        Stringappend_arraylist(s, u & 0x7F);
    } else if (u <= 0x7FF) {
        // 0x7FF 0111 1111 1111
        // 0xC0  1100 0000
        // byte1 0XC0 | (u >> 6 & 0XFF)
        Stringappend_arraylist(s, 0xC0 | (u >> 6 & 0xFF));
        // 0x80 1000 0000, 0x3F 0011 1111
        // byte2 0x80 | u & 0x3F
        Stringappend_arraylist(s, 0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {
        Stringappend_arraylist(s, 0xE0 | ((u >> 12) & 0xFF));
        Stringappend_arraylist(s, 0x80 | ((u >> 6) & 0x3F));
        Stringappend_arraylist(s, 0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        Stringappend_arraylist(s, 0xF0 | ((u >> 18) & 0xFF));
        Stringappend_arraylist(s, 0x80 | ((u >> 12) & 0x3F));
        Stringappend_arraylist(s, 0x80 | ((u >> 6) & 0x3F));
        Stringappend_arraylist(s, 0x80 | (u & 0x3F));
    }
}

static Token string(Scanner *scanner) {
    /**
     * TOKEN_STRING is for uninterpolated string literals, and the last segment of an interpolated string.
     * Every piece of a string literal that precedes an interpolated expression uses a different TOKEN_INTERPOLATION
     * type.
     *
     * This:
     *
     *   "Tea will be ready in ${steep + cool} minutes."
     *   Gets scanned like:
     *
     *   TOKEN_INTERPOLATION "Tea will be ready in"
     *   TOKEN_IDENTIFIER    "steep"
     *   TOKEN_PLUS          "+"
     *   TOKEN_IDENTIFIER    "cool"
     *   TOKEN_STRING        "minutes."
     *   (The interpolation delimiters themselves are discarded.)
     *
     * And this:
     *
     *   "Nested ${"interpolation?! Are you ${"mad?!"}"}"
     *   Scans as:
     *
     *   TOKEN_INTERPOLATION "Nested "
     *   TOKEN_INTERPOLATION "interpolation?! Are you "
     *   TOKEN_STRING        "mad?!"
     *   TOKEN_STRING        ""
     *   TOKEN_STRING        ""
     *
     * The two empty TOKEN_STRING tokens are because the interpolation appears at the very end of the string.
     * They tell the parser that they've reached the end of the interpolated expression.
     *
     */
    TokenType type = TOKEN_STRING;
    StringArrayList *s = Stringnew_arraylist(8);
    for (;;) {
        char c = advance(scanner);
        if (c == '\"') {
            break;
        } else if (c == '\n') {
            scanner->line++;
            Stringappend_arraylist(s, '\n');
        } else if (c == '\\') {
            // escape string
            char e = advance(scanner);
            switch (e) {
                case '\"': {
                    Stringappend_arraylist(s, '\"');
                    break;
                }
                case '/': {
                    Stringappend_arraylist(s, '/');
                    break;
                }
                case '\\': {
                    Stringappend_arraylist(s, '\\');
                    break;
                }
                case 'b': {
                    Stringappend_arraylist(s, '\b');
                    break;
                }
                case 'f': {
                    Stringappend_arraylist(s, '\f');
                    break;
                }
                case 'n': {
                    Stringappend_arraylist(s, '\n');
                    break;
                }
                case 'r': {
                    Stringappend_arraylist(s, '\r');
                    break;
                }
                case 't': {
                    Stringappend_arraylist(s, '\t');
                    break;
                }
                case 'u': {
                    unsigned int u;
                    if (!parse_hex4(&scanner->current, &u)) {
                        Stringfree_arraylist(s);
                        return error_token(scanner, err_msg[ERR_IU]);
                    }
                    /**
                       * surrogate handling
                       * \uXXXX\uYYYY
                       *
                       * \uXXXX: high surrogate:[U+D800, U+DBFF]
                       * \uYYYY: low surrogate:[U+DC00, U+DFFF]
                       *
                       * codepoint = 0x10000 + (H − 0xD800) × 0x400 + (L − 0xDC00)
                       */
                    if (u >= 0xD800 && u <= 0xDBFF) {
                        if (!(peek(scanner) == '\\' && peek_next(scanner) == 'u')) {
                            Stringfree_arraylist(s);
                            return error_token(scanner, err_msg[ERR_IUS]);
                        }
                        advance(scanner);
                        advance(scanner);
                        unsigned int u2;
                        if (!parse_hex4(&scanner->current, &u2)) {
                            Stringfree_arraylist(s);
                            return error_token(scanner, err_msg[ERR_IU]);
                        }
                        if (u2 > 0xDFFF || u2 < 0xDC00) {
                            Stringfree_arraylist(s);
                            return error_token(scanner, err_msg[ERR_IUS]);
                        }
                        u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                    }
                    encode_utf8(s, u);
                    break;
                }
                default: {
                    Stringfree_arraylist(s);
                    return error_token(scanner, err_msg[ERR_ISE]);;
                }
            }
        } else if (c == '\0') {
            Stringfree_arraylist(s);
            return error_token(scanner, err_msg[ERR_US]);
        } else if (c == '$' && peek(scanner) == '{') {
            advance(scanner);
            if (scanner->num_braces >= MAX_INTERPOLATION_NESTING) {
                Stringfree_arraylist(s);
                return error_token(scanner, err_msg[ERR_ID]);
            }
            scanner->braces[scanner->num_braces++] = 1;
            type = TOKEN_INTERPOLATION;
            return make_token_with_offset(scanner, type, 0, -2);
        } else {
            if ((unsigned char) c < 0x20) {
                Stringfree_arraylist(s);
                return error_token(scanner, err_msg[ERR_ISC]);;
            }
            Stringappend_arraylist(s, c);
        }
    }
    Token t = make_token(scanner, type);
    t.escaped_string = s;
    return t;
}

static Token raw_string(Scanner *scanner) {
    // consume the second and third "
    advance(scanner);
    advance(scanner);
    for (;;) {
        char c = advance(scanner);
        char c1 = peek(scanner);
        char c2 = peek_next(scanner);

        if (c == '\0' || c1 == '\0' || c2 == '\0') {
            return error_token(scanner, err_msg[ERR_US]);
        }

        if (c == '\n') {
            scanner->line++;
        }

        if (c == '"' && c1 == '"' && c2 == '"') break;

        //    advance(scanner);
    }
    // consume the second and third "
    advance(scanner);
    advance(scanner);
    return make_token_with_offset(scanner, TOKEN_STRING, 2, -2);;
}

static Token number(Scanner *scanner) {
    while (IS_DIGIT(peek(scanner))) advance(scanner);
    if (peek(scanner) == '.' && IS_DIGIT(peek_next(scanner))) {
        // consume '.'
        advance(scanner);
        while (IS_DIGIT(peek(scanner))) advance(scanner);
    }
    if (peek(scanner) == 'e' || peek(scanner) == 'E') {
        // consume 'e' or 'E'
        advance(scanner);
        if (peek(scanner) == '-' || peek(scanner) == '+') advance(scanner);
        if (!IS_DIGIT(peek(scanner))) return error_token(scanner, err_msg[ERR_IN]);
        while (IS_DIGIT(peek(scanner))) advance(scanner);
    }
    return make_token(scanner, TOKEN_NUMBER);
}

static Token identifier(Scanner *scanner) {
    while (IS_ALPHA(peek(scanner)) || IS_DIGIT(peek(scanner))) advance(scanner);
    return make_token(scanner, identifier_type(scanner));
}

/**
 * fun ->  magic,表示创建函数的行为像施展魔法
 * class -> castle,类可以像童话故事中的城堡一样
 * var -> waa,像变量突然出现的感觉
 * print -> puff,轻柔地发出，像一团烟雾飘散到屏幕上
 * if -> wish,表示通过愿望控制逻辑分支
 * else -> dream,表示如果愿望未能实现，进入梦境
 * for -> loop,像小孩子玩旋转游戏一样
 * while -> wloop, 无穷的loop直到
 * continue -> skip，像跳绳一样跳过
 * break
 * return -> home,返回温暖的家
 * true -> aow, 吼啊
 * false -> emm, 不要
 * try -> adventure，表示尝试未知的旅程
 * catch -> rescue，仿佛从危险中解救
 * throw -> toss，像丢玩具一样抛出异常
 * import -> want，表示想要某个模块
 * super -> hero，让继承感觉像超级英雄一样强大
 * this
 * trait -> color，表示类的特质像颜色一样丰富
 * lambda -> shadow，让匿名函数像影子一样灵活且神秘
 *
 *
 * @param scanner
 * @return
 */
static TokenType identifier_type(Scanner *scanner) {
    switch (*scanner->start) {
        case 'a':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'n': return check_keyword(scanner, 2, 1, "d", TOKEN_AND);
                    case 's': return check_keyword(scanner, 2, 0, "", TOKEN_AS);
                    case 'd': return check_keyword(scanner, 2, 7, "venture", TOKEN_TRY);
                    case 'o': return check_keyword(scanner, 2, 1, "w", TOKEN_TRUE);
                }
            }
            break;
        case 'b':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'r': return check_keyword(scanner, 2, 3, "eak", TOKEN_BREAK);
                }
            }
            break;
        case 'c':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    // case 'l': return check_keyword(scanner, 2, 3, "ass", TOKEN_CLASS);
                    // case 'o': return check_keyword(scanner, 2, 6, "ntinue", TOKEN_CONTINUE);
                    // case 'a': return check_keyword(scanner, 2, 3, "tch", TOKEN_CATCH);
                    case 'a': return check_keyword(scanner, 2, 4, "stle", TOKEN_CLASS);
                    case 'o': return check_keyword(scanner, 2, 3, "lor", TOKEN_TRAIT);
                }
            }
            break;
        case 'd': return check_keyword(scanner, 1, 4, "ream", TOKEN_ELSE);
        case 'e':
            // return check_keyword(scanner, 1, 3, "lse", TOKEN_ELSE);
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'm': return check_keyword(scanner, 2, 1, "m", TOKEN_FALSE);
                }
            }
            break;

        case 'f':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    // case 'a': return check_keyword(scanner, 2, 3, "lse", TOKEN_FALSE);
                    case 'i': return check_keyword(scanner, 2, 5, "nally", TOKEN_FINALLY);
                    // case 'o': return check_keyword(scanner, 2, 1, "r", TOKEN_FOR);
                    // case 'u': return check_keyword(scanner, 2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'h':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'o': return check_keyword(scanner, 2, 2, "me", TOKEN_RETURN);
                    case 'e': return check_keyword(scanner, 2, 2, "ro", TOKEN_SUPER);
                }
            }
            break;

        // case 'i': return check_keyword(scanner, 1, 1, "f", TOKEN_IF);
        case 'l': return check_keyword(scanner, 1, 3, "oop", TOKEN_FOR);
        case 'm': return check_keyword(scanner, 1, 4, "agic", TOKEN_FUN);
        case 'n': return check_keyword(scanner, 1, 2, "il", TOKEN_NIL);
        case 'o': return check_keyword(scanner, 1, 1, "r", TOKEN_OR);
        case 'p': return check_keyword(scanner, 1, 4, "rint", TOKEN_PRINT);
        // case 'r': return check_keyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
        case 'r': return check_keyword(scanner, 1, 5, "escue", TOKEN_CATCH);
        case 's':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    // case 'u': return check_keyword(scanner, 2, 3, "per", TOKEN_SUPER);
                    case 't': return check_keyword(scanner, 2, 4, "atic", TOKEN_STATIC);
                    case 'k': return check_keyword(scanner, 2, 2, "ip", TOKEN_CONTINUE);
                    case 'h': return check_keyword(scanner, 2, 4, "adow", TOKEN_LAMBDA);
                }
            }
            break;
        case 't':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    // case 'h':
                    //     if (scanner->current - scanner->start > 2) {
                    //         switch (scanner->start[2]) {
                    //             case 'i': return check_keyword(scanner, 3, 1, "s", TOKEN_THIS);
                    //             // case 'r': return check_keyword(scanner, 3, 2, "ow", TOKEN_THROW);
                    //         }
                    //     }
                    case 'h': return check_keyword(scanner, 2, 2, "is", TOKEN_THIS);
                    case 'o': return check_keyword(scanner, 2, 2, "ss", TOKEN_THROW);
                    // case 'r':
                    //     if (scanner->current - scanner->start > 2) {
                    //         switch (scanner->start[2]) {
                    //             case 'u': return check_keyword(scanner, 3, 1, "e", TOKEN_TRUE);
                    //             case 'a': return check_keyword(scanner, 3, 2, "it", TOKEN_TRAIT);
                    //             case 'y': return check_keyword(scanner, 3, 0, "", TOKEN_TRY);
                    //         }
                    //     }
                }
            }
            break;
        // case 'v': return check_keyword(scanner, 1, 2, "ar", TOKEN_VAR);

        case 'w':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    // case 'h': return check_keyword(scanner, 2, 3, "ile", TOKEN_WHILE);
                    // case 'a': return check_keyword(scanner, 2, 2, "nt", TOKEN_IMPORT);
                    case 'i': return check_keyword(scanner, 2, 2, "sh", TOKEN_IF);
                    case 'l': return check_keyword(scanner, 2, 3, "oop", TOKEN_WHILE);
                    case 'a':
                        if (scanner->current - scanner->start > 2) {
                            switch (scanner->start[2]) {
                                case 'n': return check_keyword(scanner, 3, 1, "t", TOKEN_IMPORT);
                                case 'a': return check_keyword(scanner, 3, 0, "", TOKEN_VAR);
                            }
                        }
                }
            }
            break;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType check_keyword(Scanner *scanner, int start, int length,
                               const char *rest, TokenType type) {
    if (scanner->current - scanner->start == start + length &&
        (length == 0 || memcmp(scanner->start + start, rest, length) == 0)) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static char advance(Scanner *scanner) {
    return *scanner->current++;
}

static int match(Scanner *scanner, char expected) {
    if (is_end(scanner)) return 0;
    if (*scanner->current != expected) return 0;
    scanner->current++;
    return 1;
}

static void skip_whitespace(Scanner *scanner) {
    for (;;) {
        switch (peek(scanner)) {
            case ' ':
            case '\r':
            case '\t': advance(scanner);
                break;
            case '\n': scanner->line++;
                advance(scanner);
                break;
            default: return;
        }
    }
}

static int skip_comment(Scanner *scanner) {
    for (;;) {
        skip_whitespace(scanner);
        if (peek(scanner) == '/') {
            if (peek_next(scanner) == '/') {
                // skip line comment "//"
                advance(scanner);
                advance(scanner);
                while (!is_end(scanner)) {
                    char c = peek(scanner);
                    //          if (c == ' ' || c == '\r' || c == '\t') {
                    //            advance(scanner);
                    //            continue;
                    //          }
                    if (c == '\n') {
                        advance(scanner);
                        scanner->line++;
                        break;
                    }
                    advance(scanner);
                }
            } else if (peek_next(scanner) == '*') {
                // block comment
                advance(scanner);
                advance(scanner);
                int nesting = 1;
                while (nesting > 0) {
                    if (peek(scanner) == '\0') return ERR_UB;
                    if (peek(scanner) == '\n') scanner->line++;

                    if (peek(scanner) == '/' && peek_next(scanner) == '*') {
                        advance(scanner);
                        advance(scanner);
                        nesting++;
                        continue;
                    }

                    if (peek(scanner) == '*' && peek_next(scanner) == '/') {
                        advance(scanner);
                        advance(scanner);
                        nesting--;
                        continue;
                    }
                    advance(scanner);
                }
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }
}

static char peek(Scanner *scanner) {
    return *scanner->current;
}

static char peek_next(Scanner *scanner) {
    if (is_end(scanner)) return '\0';
    return *(scanner->current + 1);
}
