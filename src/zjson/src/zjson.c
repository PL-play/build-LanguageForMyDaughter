//
// Created by ran on 2024/3/1.
//
#include "zjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef JSON_PARSE_STACK_INIT_SIZE
#define JSON_PARSE_STACK_INIT_SIZE 256
#endif
#ifndef JSON_STRINGIFY_STACK_INIT_SIZE
#define JSON_STRINGIFY_STACK_INIT_SIZE 256
#endif

#ifndef JSON_ARRAY_INIT_CAPACITY
#define JSON_ARRAY_INIT_CAPACITY 8
#endif

#define IS_DIGIT(c) ((c)>='0'&& (c)<='9')
#define IS_DIGIT1_9(c) ((c)>='1' && (c)<='9')

typedef struct {
  const char *json;
  char *stack;
  size_t size, top;
} json_context;

typedef struct {
  json_string key;
  json_value *value;
} object_member;

DEFINE_HASHTABLE(json_value*, NULL, JSON)

static void parse_ws(json_context *c);
static int parse_literal(json_context *c, json_value *value, const char *literal, json_value_type type);
static int parse_value(json_context *c, json_value *value);
static int parse_number(json_context *c, json_value *value);
static int parse_array(json_context *c, json_value *value);
static int consume(json_context *c, char expected);
static int parse_string(json_context *c, json_value *value);
static int parse_string_raw(json_context *c, char **s, size_t *len);
static char *context_stack_push(json_context *c, size_t size);
static char *context_stack_pop(json_context *c, size_t size);
static void put_char(json_context *c, char ch);
static const char *parse_hex4(const char *p, unsigned int *u);
static void encode_utf8(json_context *c, unsigned int u);
static int parse_object(json_context *c, json_value *value);
static unsigned int json_string_hash(json_string *);
static int json_string_equals(json_string *, json_string *);
static void json_stringify_value(json_context *c, const json_value *v);
static void stringify_string(json_context *c, const json_string *v);
void free_json_string(json_string *s);
void grow_array(json_value *value, size_t capacity);
void shrink_array(json_value *value);

// for debug
static void debug_print_value(const json_value *v);
json_value *new_null() {
  json_value *e = malloc(sizeof(json_value));
  init_as_null(e);
  return e;
}
int parse_json_string(json_value *value, const char *json) {
  json_context c = {.json=json, .stack=NULL, .top=0, .size=0};
  assert(value != NULL);
  value->type = JSON_NULL;
  parse_ws(&c);
  int ret = parse_value(&c, value);
  if (ret != JSON_PARSE_OK) {
    free(c.stack);
    free_json_value(value);
    return ret;
  }
  parse_ws(&c);
  if (*c.json != '\0') {
    free(c.stack);
    free_json_value(value);
    return JSON_PARSE_DUPLICATE_ROOT;
  }
  assert(c.top == 0);
  free(c.stack);
  return JSON_PARSE_OK;
}
json_value_type get_json_type(json_value *value) {
  assert(value != NULL);
  return value->type;
}
void free_json_value(json_value *value) {
  assert(value != NULL);
  if (value->type == JSON_STRING) {
    free(value->val.s.s);
  } else if (value->type == JSON_ARRAY) {
    for (int i = 0; i < value->val.a.size; ++i) {
      free_json_value(&value->val.a.e[i]);
    }
    free(value->val.a.e);
  } else if (value->type == JSON_OBJECT) {
    if (value->val.o != NULL) {
      JSONHashtableIterator *iter = JSONhashtable_iterator(value->val.o);
      while (JSONhashtable_iter_has_next(iter)) {
        JSONKVEntry *e = JSONhashtable_next_entry(iter);
        json_string *key = JSONtable_entry_key(e);
        json_value *v = JSONtable_entry_value(e);
        free(key->s);
        free(key);

        free_json_value(v);
        free(v);
      }
      JSONfree_hashtable_iter(iter);
      JSONfree_hash_table(value->val.o);
    }
  }
  value->type = JSON_NULL;
}
static void parse_ws(json_context *c) {
  const char *p = c->json;
  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
    p++;
  }
  c->json = p;
}

static int parse_value(json_context *c, json_value *value) {
  switch (*c->json) {
    case 'n': return parse_literal(c, value, "null", JSON_NULL);
    case 't': return parse_literal(c, value, "true", JSON_TRUE);
    case 'f': return parse_literal(c, value, "false", JSON_FALSE);
    case '"': return parse_string(c, value);
    case '[': return parse_array(c, value);
    case '{': return parse_object(c, value);
    case '\0': return JSON_PARSE_EXPECT_VALUE;
    default: return parse_number(c, value);
  }
}

static int parse_number(json_context *c, json_value *value) {
  /**
   * number -> "-"? int frac? exp?
   * int -> "0" | (digit1_9)+ digit
   * frac -> "." digit+
   * exp -> ("e" | "E") ("-" | "+")? digit+
   */
  const char *p = c->json;
  if (*p == '-') p++;
  if (*p == '0') {
    p++;
  } else {
    if (!IS_DIGIT1_9(*p)) {
      return JSON_PARSE_INVALID_VALUE;
    }
    p++;
    while (IS_DIGIT(*p)) {
      p++;
    }
  }
  if (*p == '.') {
    p++;
    if (!IS_DIGIT(*p)) {
      return JSON_PARSE_INVALID_VALUE;
    }
    p++;
    while (IS_DIGIT(*p)) {
      p++;
    }
  }
  if (*p == 'e' || *p == 'E') {
    p++;
    if (*p == '-' || *p == '+') p++;
    if (!IS_DIGIT(*p)) {
      return JSON_PARSE_INVALID_VALUE;
    }
    p++;
    while (IS_DIGIT(*p)) {
      p++;
    }
  }

  errno = 0;
  value->val.n = strtod(c->json, NULL);
  if (errno == ERANGE && (value->val.n == HUGE_VAL) || value->val.n == -HUGE_VAL) {
    return JSON_PARSE_NUMBER_TO_BIG;
  }
  value->type = JSON_NUMBER;
  c->json = p;
  return JSON_PARSE_OK;
}

static int parse_literal(json_context *c, json_value *value, const char *literal, json_value_type type) {
  consume(c, literal[0]);
  size_t i;
  for (i = 0; literal[i + 1]; ++i) {
    if (c->json[i] != literal[i + 1]) {
      return JSON_PARSE_INVALID_VALUE;
    }
  }
  c->json += i;
  value->type = type;
  return JSON_PARSE_OK;
}

static int parse_string_raw(json_context *c, char **s, size_t *len) {
  consume(c, '\"');
  size_t head = c->top;
  const char *p = c->json;

  while (1) {
    char ch = *p++;
    switch (ch) {
      case '\"': {
        *len = c->top - head;
        *s = context_stack_pop(c, *len);
        c->json = p;
        return JSON_PARSE_OK;
      }
      case '\0': {
        c->top = head;
        return JSON_PARSE_MISS_QUOTATION_MARK;
      }
      case '\\': {
        switch (*p++) {
          case '\"': {
            put_char(c, '\"');
            break;
          }
          case '/': {
            put_char(c, '/');
            break;
          }
          case '\\': {
            put_char(c, '\\');
            break;
          }
          case 'b': {
            put_char(c, '\b');
            break;
          }
          case 'f': {
            put_char(c, '\f');
            break;
          }
          case 'n': {
            put_char(c, '\n');
            break;
          }
          case 'r': {
            put_char(c, '\r');
            break;
          }
          case 't': {
            put_char(c, '\t');
            break;
          }
          case 'u': {
            unsigned int u;
            if (!(p = parse_hex4(p, &u))) {
              c->top = head;
              return JSON_PARSE_INVALID_UNICODE_HEX;
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
              if (!(*p == '\\' && *(p + 1) == 'u')) {
                c->top = head;
                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
              }
              p += 2;
              unsigned int u2;
              if (!(p = parse_hex4(p, &u2))) {
                c->top = head;
                return JSON_PARSE_INVALID_UNICODE_HEX;
              }
              if (u2 > 0xDFFF || u2 < 0xDC00) {
                c->top = head;
                return JSON_PARSE_INVALID_UNICODE_SURROGATE;
              }
              u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
            }
            encode_utf8(c, u);
            break;
          }
          default: {
            c->top = head;
            return JSON_PARSE_INVALID_STRING_ESCAPE;
          }
        }
        break;
      }
      default: {
        if ((unsigned char) ch < 0x20) {
          c->top = head;
          return JSON_PARSE_INVALID_STRING_CHAR;
        }
        put_char(c, ch);
      }
    }
  }
}
static int parse_string(json_context *c, json_value *value) {
  int ret;
  char *s;
  size_t len;
  if ((ret = parse_string_raw(c, &s, &len)) == JSON_PARSE_OK) {
    set_json_str(value, s, len);
  }
  return ret;
}

static int parse_array(json_context *c, json_value *value) {
  consume(c, '[');
  size_t size = 0;
  int ret;
  parse_ws(c);
  if (*c->json == ']') {
    c->json++;
    value->type = JSON_ARRAY;
    value->val.a.size = 0;
    value->val.a.capacity = JSON_ARRAY_INIT_CAPACITY;
    value->val.a.e = malloc(sizeof(json_value) * value->val.a.capacity);
    return JSON_PARSE_OK;
  }
  while (1) {
    json_value temp = {.type=JSON_NULL};
    ret = parse_value(c, &temp);
    if (ret != JSON_PARSE_OK) {
      break;
    }
    // push stack
    memcpy(context_stack_push(c, sizeof(json_value)), &temp, sizeof(json_value));
    size++;
    parse_ws(c);
    if (*c->json == ',') {
      c->json++;
      parse_ws(c);
    } else if (*c->json == ']') {
      c->json++;
      value->type = JSON_ARRAY;
      value->val.a.size = size;
      size_t s = size * sizeof(json_value);
      size_t cap = s + (s >> 1);
      value->val.a.e = malloc(cap);
      memcpy(value->val.a.e, context_stack_pop(c, s), s);
      return JSON_PARSE_OK;
    } else {
      ret = JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
  }
  // clean stack
  for (int i = 0; i < size; ++i) {
    free_json_value((json_value *) context_stack_pop(c, sizeof(json_value)));
  }
  return ret;
}

static int parse_object(json_context *c, json_value *value) {
  consume(c, '{');
  parse_ws(c);
  size_t size = 0;
  int ret;
  object_member member;
  if (*c->json == '}') {
    c->json++;
    value->type = JSON_OBJECT;
    value->val.o = NULL;
    return JSON_PARSE_OK;
  }
  member.key.s = NULL;
  json_value *v = NULL;
  while (1) {

    char *key;
    size_t key_len;
    if (*c->json != '"') {
      ret = JSON_PARSE_MISS_KEY;
      break;
    }

    ret = parse_string_raw(c, &key, &key_len);
    if (ret != JSON_PARSE_OK) {
      break;
    }
    char *mem_key = malloc(key_len + 1);
    memcpy(mem_key, key, key_len);
    mem_key[key_len] = '\0';

    parse_ws(c);
    if (!consume(c, ':')) {
      ret = JSON_PARSE_MISS_COLON;
      free(mem_key);
      break;
    }
    parse_ws(c);
    v = malloc(sizeof(json_value));
    ret = parse_value(c, v);
    if (ret != JSON_PARSE_OK) {
      free(mem_key);
      break;
    }
    // push stack
    member.key.s = mem_key;
    member.key.len = key_len;
    member.value = v;
    memcpy(context_stack_push(c, sizeof(object_member)), &member, sizeof(object_member));
    size++;
//    printf("stack===\n");
//    for (int i = 0; i < size; ++i) {
//      object_member *ms = (object_member *) (c->stack + (c->top - sizeof(object_member) * (i + 1)));
//      printf(" key: %s. ", ms->key.s);
//      fflush(stdout);
//      printf("value: ");
//      debug_print_value(ms->value);
//    }
//    printf("stack===\n");
//    fflush(stdout);
    parse_ws(c);
    if (*c->json == ',') {
      c->json++;
      parse_ws(c);
    } else if (*c->json == '}') {
      c->json++;
      value->type = JSON_OBJECT;
      JSONHashtable *table = JSONnew_hash_table((JSONHashtableHashFunc) json_string_hash,
                                        (JSONHashtableEqualsFunc) json_string_equals);
      for (size_t i = 0; i < size; ++i) {
        object_member *ms = (object_member *) context_stack_pop(c, sizeof(object_member));
        json_string *k = malloc(sizeof(json_string));

        k->s = ms->key.s;
        k->len = ms->key.len;

        JSONput_hash_table(table, k, ms->value);
      }
      value->val.o = table;
      return JSON_PARSE_OK;
    } else {
      ret = JSON_PARSE_MISS_COMMA_OR_BRACE;
      break;
    }
  }
  if (v != NULL)
    free(v);
  // clean stack
  for (int i = 0; i < size; ++i) {
    object_member *ms = (object_member *) context_stack_pop(c, sizeof(object_member));
    free(ms->key.s);
    //free_json_value(&ms->value);
  }
  value->type = JSON_NULL;
  return ret;
}

static unsigned int json_string_hash(json_string *string) {
  unsigned int hash = 5381; // Starting with a prime number
  size_t i;
  for (i = 0; i < string->len; ++i) {
    // Combine the current hash value with the current character
    // (hash << 5) + hash is equivalent to hash * 33, but more efficient
    hash = ((hash << 5) + hash) + (unsigned char) string->s[i];
  }
  return hash;
}
static int json_string_equals(json_string *string1, json_string *string2) {
  // First, check if their lengths are equal
  if (string1->len != string2->len) {
    return 1;
  }

  // Now compare each byte
  for (size_t i = 0; i < string1->len; ++i) {
    if (string1->s[i] != string2->s[i]) {
      return 1; // Found a difference
    }
  }
  return 0; // No differences found, strings are equal
}

static char *context_stack_push(json_context *c, size_t size) {
  assert(size > 0);
  if (c->size <= c->top + size) {
    if (c->size == 0) {
      c->size = JSON_PARSE_STACK_INIT_SIZE;
    }
    while (c->size <= c->top + size) {
      c->size += c->size >> 1; // size*1.5
    }
    c->stack = realloc(c->stack, c->size);
  }
  char *ret = c->stack + c->top;
  c->top += size;
  return ret;
}

static char *context_stack_pop(json_context *c, size_t size) {
  assert(c->top >= size);
  return c->stack + (c->top -= size);
}

static void put_char(json_context *c, char ch) {
  *context_stack_push(c, sizeof(char)) = ch;
}

static int consume(json_context *c, const char expected) {
  int ret = 0;
  if (*c->json == expected) {
    c->json++;
    ret = 1;
  }
  return ret;
}

static const char *parse_hex4(const char *p, unsigned int *u) {
  *u = 0;
  for (int i = 0; i < 4; ++i) {
    char ch = *p++;
    *u <<= 4;
    if (ch >= '0' && ch <= '9') *u |= ch - '0';
    else if (ch >= 'a' && ch <= 'f') *u |= ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F') *u |= ch - 'A' + 10;
    else return NULL;
  }
  return p;
}

static void encode_utf8(json_context *c, unsigned int u) {
  /**
     * Code Point Range | bits |  byte1  |   byte2   |  byte3  |  byte4
     * U+0000 ~ U+007F	  7     0xxxxxxx
     * U+0080 ~ U+07FF	  11    110xxxxx   10xxxxxx
     * U+0800 ~ U+FFFF	  16	1110xxxx   10xxxxxx	  10xxxxxx
     * U+10000 ~ U+10FFFF 21	11110xxx   10xxxxxx	  10xxxxxx	10xxxxxx
     */
  if (u <= 0x7F) {
    //  0x7F 0111 1111
    put_char(c, u & 0x7F);
  } else if (u <= 0x7FF) { // 0x7FF 0111 1111 1111
    // 0xC0  1100 0000
    // byte1 0XC0 | (u >> 6 & 0XFF)
    put_char(c, 0xC0 | (u >> 6 & 0xFF));
    // 0x80 1000 0000, 0x3F 0011 1111
    // byte2 0x80 | u & 0x3F
    put_char(c, 0x80 | (u & 0x3F));
  } else if (u <= 0xFFFF) {
    put_char(c, 0xE0 | ((u >> 12) & 0xFF));
    put_char(c, 0x80 | ((u >> 6) & 0x3F));
    put_char(c, 0x80 | (u & 0x3F));
  } else {
    assert(u <= 0x10FFFF);
    put_char(c, 0xF0 | ((u >> 18) & 0xFF));
    put_char(c, 0x80 | ((u >> 12) & 0x3F));
    put_char(c, 0x80 | ((u >> 6) & 0x3F));
    put_char(c, 0x80 | (u & 0x3F));
  }
}

void set_json_str(json_value *value, const char *string, size_t len) {
  assert(value != NULL && (string != NULL || len == 0));
  free_json_value(value);
  value->val.s.s = malloc(len + 1);
  memcpy(value->val.s.s, string, len);
  value->val.s.s[len] = '\0';
  value->val.s.len = len;
  value->type = JSON_STRING;
}

const char *get_json_str(json_value *value) {
  assert(value != NULL && value->type == JSON_STRING);
  return value->val.s.s;
}
size_t get_json_str_len(json_value *value) {
  assert(value != NULL && value->type == JSON_STRING);
  return value->val.s.len;
}

void set_json_null(json_value *value) {
  free_json_value(value);
}

void set_json_number(json_value *value, double n) {
  free_json_value(value);
  value->type = JSON_NUMBER;
  value->val.n = n;
}
double get_json_number(json_value *value) {
  assert(value != NULL && value->type == JSON_NUMBER);
  return value->val.n;
}

void set_json_bool(json_value *value, int bool) {
  free_json_value(value);
  value->type = bool ? JSON_TRUE : JSON_FALSE;
}
int get_json_bool(json_value *value) {
  assert(value != NULL && (value->type == JSON_FALSE || value->type == JSON_TRUE));
  return value->type == JSON_TRUE ? 1 : 0;
}

size_t get_json_array_size(json_value *value) {
  assert(value != NULL && value->type == JSON_ARRAY);
  return value->val.a.size;
}

json_value *get_json_array_element(json_value *value, size_t index) {
  assert(value != NULL && value->type == JSON_ARRAY);
  assert(index < value->val.a.size);
  return &value->val.a.e[index];
}

size_t get_json_array_capacity(json_value *value) {
  assert(value != NULL && value->type == JSON_ARRAY);
  return value->val.a.capacity;
}

json_value *insert_json_array(json_value *value, size_t index) {
  assert(value != NULL && value->type == JSON_ARRAY);
  assert(index <= value->val.a.size);
  if (value->val.a.size >= value->val.a.capacity) {
    grow_array(value, value->val.a.capacity == 0 ? 1 : value->val.a.capacity * 2);
  }
  if (index != value->val.a.size) {
    memmove(&value->val.a.e[index + 1],
            &value->val.a.e[index],
            (value->val.a.size - index) * sizeof(json_value));
  }
  value->val.a.size++;
  return &value->val.a.e[index];
}

void remove_json_array_element(json_value *value, size_t start, size_t count) {
  assert(value != NULL && value->type == JSON_ARRAY);
  assert(start < value->val.a.size);
  for (size_t i = start; i < start + count; ++i) {
    free_json_value(&value->val.a.e[i]);
  }
  size_t numElementsToMove = value->val.a.size - (start + count);
  if (numElementsToMove > 0) {
    memmove(&value->val.a.e[start],
            &value->val.a.e[start + count],
            numElementsToMove * sizeof(json_value));
  }

  value->val.a.size -= count;
}

void clear_json_array(json_value *value) {
  remove_json_array_element(value, 0, value->val.a.size);
}
void shrink_json_array(json_value *value) {
  shrink_array(value);
}
size_t get_json_object_size(const json_value *value) {
  assert(value != NULL && value->type == JSON_OBJECT);
  return value->val.o == NULL ? 0 : JSONsize_of_hash_table(value->val.o);
}
json_value *get_json_object_value(const json_value *value, char *key, size_t key_len) {
  assert(value != NULL && value->type == JSON_OBJECT);
  json_string k = {.s=key, .len=key_len};
  return value->val.o != NULL ? JSONget_hash_table(value->val.o, &k) : NULL;
}

int json_contains_key(const json_value *value, char *key, size_t key_len) {
  assert(value != NULL);
  if (value->type != JSON_OBJECT) return 0;
  if (value->val.o == NULL) return 0;
  json_string k = {.s=key, .len=key_len};
  return JSONcontains_in_hash_table(value->val.o, &k);
}

json_value *set_json_object(json_value *v, char *key, size_t key_len, int *is_new) {
  assert(v != NULL && v->type == JSON_OBJECT && key != NULL);
  json_value *value = NULL;
  if (v->val.o != NULL) {
    value = get_json_object_value(v, key, key_len);
  }

  if (is_new != NULL)
    *is_new = value == NULL ? 1 : 0;

  if (value == NULL) {
    json_string *new_key = malloc(sizeof(json_string));
    new_key->s = malloc(key_len);
    memcpy(new_key->s, key, key_len);
    new_key->len = key_len;

    value = malloc(sizeof(json_value));
    value->type = JSON_NULL;
    if (v->val.o == NULL)
      v->val.o = JSONnew_hash_table((JSONHashtableHashFunc) json_string_hash,
                                (JSONHashtableEqualsFunc) json_string_equals);
    JSONput_hash_table(v->val.o, new_key, value);
  }
  return value;
}

void free_json_string(json_string *s) {
  free(s->s);
  free(s);
}

void grow_array(json_value *value, size_t capacity) {
  assert(value != NULL && value->type == JSON_ARRAY);
  if (value->val.a.capacity < capacity) {
    json_value *p = realloc(value->val.a.e, capacity * sizeof(json_value));
    if (p != NULL) {
      value->val.a.e = p;
      value->val.a.capacity = capacity;
    }
  }
}

void shrink_array(json_value *value) {
  assert(value != NULL && value->type == JSON_ARRAY);
  if (value->val.a.size < value->val.a.capacity) {
    size_t newSize = value->val.a.size * sizeof(json_value);
    json_value *p = realloc(value->val.a.e, newSize);
    if (newSize == 0 || p != NULL) {
      value->val.a.e = p;
      value->val.a.capacity = value->val.a.size;
    }
  }
}

int remove_json_object(json_value *v, char *key, size_t key_len) {
  assert(v != NULL && v->type == JSON_OBJECT && key != NULL);
  json_value *value = get_json_object_value(v, key, key_len);
  if (value == NULL) return 0;
  json_string k = {key, key_len};
  JSONregister_hashtable_free_functions(v->val.o, (JSONHashtableKeyFreeFunc) free_json_string,
                                    (JSONHashtableValueFreeFunc) free_json_value);
  JSONremove_hash_table(v->val.o, &k);
  free(value);
  value = NULL;
  JSONregister_hashtable_free_functions(v->val.o, NULL, NULL);
  return 1;
}

#define PUTS(c, s, len) memcpy(context_stack_push((c),(len)),(s),(len))

static void json_stringify_value(json_context *c, const json_value *v) {
  switch (v->type) {
    case JSON_NULL: {
      PUTS(c, "null", 4);
      break;
    }
    case JSON_NUMBER: {
      char *buffer = context_stack_push(c, 32);
      int len = sprintf(buffer, "%.17g", v->val.n);
      c->top -= 32 - len;
      break;
    }
    case JSON_TRUE: {
      PUTS(c, "true", 4);
      break;
    }
    case JSON_FALSE: {
      PUTS(c, "false", 5);
      break;
    }
    case JSON_STRING: {
      stringify_string(c, &v->val.s);
      break;
    }
    case JSON_ARRAY: {
      put_char(c, '[');
      for (int i = 0; i < v->val.a.size; ++i) {
        if (i > 0) put_char(c, ',');
        json_stringify_value(c, &v->val.a.e[i]);
      }
      put_char(c, ']');
      break;
    }
    case JSON_OBJECT: {
      put_char(c, '{');
      JSONHashtable *m = v->val.o;
      if (m != NULL) {
        JSONHashtableIterator *iter = JSONhashtable_iterator(m);
        size_t i = 0;
        while (JSONhashtable_iter_has_next(iter)) {
          JSONKVEntry *e = JSONhashtable_next_entry(iter);
          json_string *key = JSONtable_entry_key(e);
          json_value *value = JSONtable_entry_value(e);
          if (i > 0) put_char(c, ',');
          stringify_string(c, key);
          put_char(c, ':');
          json_stringify_value(c, value);
          i++;
        }
        JSONfree_hashtable_iter(iter);
      }
      put_char(c, '}');
      break;
    }
  }
}

static void stringify_string(json_context *c, const json_string *v) {
  assert(v->s != NULL);
  put_char(c, '\"');
  for (int i = 0; i < v->len; ++i) {
    unsigned char ch = v->s[i];
    switch (ch) {
      case '\"': {
        PUTS(c, "\\\"", 2);
        break;
      }
      case '\\': {
        PUTS(c, "\\\\", 2);
        break;
      }
      case '\b': {
        PUTS(c, "\\b", 2);
        break;
      }
      case '\f': {
        PUTS(c, "\\f", 2);
        break;
      }
      case '\n': {
        PUTS(c, "\\n", 2);
        break;
      }
      case '\r': {
        PUTS(c, "\\r", 2);
        break;
      }
      case '\t': {
        PUTS(c, "\\t", 2);
        break;
      }
      default: {
        if (ch < 0x20) {
          char buffer[7];
          sprintf(buffer, "\\u%04X", ch);
          PUTS(c, buffer, 6);
        } else {
          put_char(c, v->s[i]);
        }
      }
    }
  }
  put_char(c, '\"');
}

char *json_stringify(const json_value *value, size_t *len) {
  assert(value != NULL);
  json_context c;
  c.stack = malloc(c.size = JSON_STRINGIFY_STACK_INIT_SIZE);
  c.top = 0;
  json_stringify_value(&c, value);
  if (len) {
    *len = c.top;
  }
  put_char(&c, '\0');
  return c.stack;
}

int json_dump(const json_value *value, char *fp) {
  char *ret = json_stringify(value, NULL);
  FILE *file = fopen(fp, "w");
  if (file == NULL) {
    return JSON_DUMP_FILE_ERROR;
  }
  fputs(ret, file);
  fflush(file);
  fclose(file);
  free(ret);
  return JSON_DUMP_OK;
}

int json_load(json_value *value, char *fp) {
  FILE *file;
  char *buffer;
  long numbytes;

  file = fopen(fp, "r");

  if (file == NULL) {
    return JSON_LOAD_FILE_ERROR;
  }

  fseek(file, 0L, SEEK_END);
  numbytes = ftell(file);

  fseek(file, 0L, SEEK_SET);

  // Allocate memory for the buffer to hold the text
  buffer = (char *) malloc(numbytes + 1);

  // Read the content into the buffer
  fread(buffer, sizeof(char), numbytes, file);
  buffer[numbytes] = '\0'; // Null-terminate the buffer
  fclose(file);

  int ret = parse_json_string(value, buffer);
  free(buffer);
  if (ret != JSON_PARSE_OK) {
    free_json_value(value);
  }
  return ret;
}

int json_compare(const json_value *json1, const json_value *json2) {
  assert(json1 != NULL && json2 != NULL);
  if (json1->type != json2->type) return 0;
  switch (json1->type) {
    case JSON_STRING: {
      int ret = json1->val.s.len == json2->val.s.len
          && memcmp(json1->val.s.s, json2->val.s.s, json1->val.s.len) == 0;
      return ret;
    }
    case JSON_NUMBER: {
      int ret = json1->val.n == json2->val.n;
      return ret;
    }
    case JSON_ARRAY: {
      if (json1->val.a.size != json2->val.a.size) return 0;
      for (size_t i = 0; i < json1->val.a.size; ++i) {
        if (!json_compare(&json1->val.a.e[i], &json2->val.a.e[i])) return 0;
      }
      return 1;
    }
    case JSON_OBJECT: {
      if (json1->val.o == NULL && json2->val.o != NULL || json2->val.o == NULL && json1->val.o != NULL) return 0;
      if (json1->val.o == NULL && json2->val.o == NULL) return 1;
      if (JSONsize_of_hash_table(json1->val.o) != JSONsize_of_hash_table(json2->val.o)) return 0;
      JSONHashtableIterator *iter1 = JSONhashtable_iterator(json1->val.o);
      int is_equal = 1;
      while (JSONhashtable_iter_has_next(iter1)) {
        JSONKVEntry *e = JSONhashtable_next_entry(iter1);
        json_string *key = JSONtable_entry_key(e);
        json_value *value = JSONtable_entry_value(e);
        if (!JSONcontains_in_hash_table(json2->val.o, key)) {
          is_equal = 0;
          break;
        }
        json_value *value2 = JSONget_hash_table(json2->val.o, key);
        if (!json_compare(value2, value)) {
          is_equal = 0;
          break;
        }
      }
      JSONfree_hashtable_iter(iter1);
      return is_equal;
    }

    default:return 1;
  }
}

void json_copy(json_value *dest, const json_value *src) {
  assert(dest != NULL && src != NULL && src != dest);
  switch (src->type) {
    case JSON_STRING: {
      set_json_str(dest, src->val.s.s, src->val.s.len);
      break;
    }
    case JSON_ARRAY: {
      dest->type = JSON_ARRAY;
      dest->val.a.size = src->val.a.size;
      dest->val.a.e = malloc(sizeof(json_value) * src->val.a.size);
      for (size_t i = 0; i < src->val.a.size; ++i) {
        json_copy(&dest->val.a.e[i], &src->val.a.e[i]);
      }
      break;
    }
    case JSON_OBJECT: {
      dest->type = JSON_OBJECT;
      if (src->val.o != NULL) {
        dest->val.o = JSONnew_hash_table((JSONHashtableHashFunc) json_string_hash,
                                     (JSONHashtableEqualsFunc) json_string_equals);
        JSONHashtableIterator *iter = JSONhashtable_iterator(src->val.o);
        while (JSONhashtable_iter_has_next(iter)) {
          JSONKVEntry *e = JSONhashtable_next_entry(iter);
          json_string *key = JSONtable_entry_key(e);
          json_value *value = JSONtable_entry_value(e);

          json_string *new_key = malloc(sizeof(json_string));
          new_key->s = malloc(sizeof(char) * key->len);
          memcpy(new_key->s, key->s, sizeof(char) * key->len);
          new_key->len = key->len;

          json_value *new_value = malloc(sizeof(json_value));
          json_copy(new_value, value);
          JSONput_hash_table(dest->val.o, new_key, new_value);
        }
        JSONfree_hashtable_iter(iter);
      }

      break;
    }
    default: {
      memcpy(dest, src, sizeof(json_value));
    }
  }
}
void json_move(json_value *dest, json_value *src) {
  assert(dest != NULL && src != NULL && src != dest);
  free_json_value(dest);
  memcpy(dest, src, sizeof(json_value));
  src->type = JSON_NULL;
}

void json_swap(json_value *v1, json_value *v2) {
  assert(v1 != NULL && v2 != NULL);
  if (v1 != v2) {
    json_value temp;
    memcpy(&temp, v1, sizeof(json_value));
    memcpy(v1, v2, sizeof(json_value));
    memcpy(v2, &temp, sizeof(json_value));
  }
}

static void debug_print_value(const json_value *v) {
  size_t len;
  char *ret = json_stringify(v, &len);
  printf("%s\n", ret);
  fflush(stdout);
  free(ret);
}