//
// Created by ran on 2024/3/1.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/framework.h"
#include "zjson.h"

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
      do {                                            \
          if(!(equality)){                                 \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
          }                                                \
      }while(0)

#define EXPECT_INT(expect, actual) EXPECT_EQ_BASE((expect)==(actual),expect,actual,"%d")
#define EXPECT_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect)==(actual),expect,actual,"%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")

#define TEST_NUMBER(expect, json_str) \
  do{                            \
      json_value json;           \
      EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, json_str)); \
      EXPECT_INT(JSON_NUMBER, get_json_type(&json));                 \
      EXPECT_DOUBLE(expect,get_json_number(&json));\
  }while(0)

#define TEST_ERROR(error, json)\
    do {\
        json_value v;\
        v.type = JSON_FALSE;\
        EXPECT_INT(error, parse_json_string(&v, json));\
        EXPECT_INT(JSON_NULL, get_json_type(&v));\
    } while(0)

static void test_parse_null() {
  json_value json;
  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, "null"));
  EXPECT_INT(JSON_NULL, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, " \n null"));
  EXPECT_INT(JSON_NULL, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, "\r  null  \n\r "));
  EXPECT_INT(JSON_NULL, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_DUPLICATE_ROOT, parse_json_string(&json, "\r  \tnull false \n\r "));
}

static void test_parse_true() {
  json_value json;
  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, "true"));
  EXPECT_INT(JSON_TRUE, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, " \n true"));
  EXPECT_INT(JSON_TRUE, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, "\r  \ttrue  \n\r "));
  EXPECT_INT(JSON_TRUE, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_DUPLICATE_ROOT, parse_json_string(&json, "\r  \ttrue false \n\r "));
}

static void test_parse_false() {
  json_value json;
  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, "false"));
  EXPECT_INT(JSON_FALSE, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, " \n false"));
  EXPECT_INT(JSON_FALSE, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&json, "\r  \tfalse  \n\r "));
  EXPECT_INT(JSON_FALSE, get_json_type(&json));

  EXPECT_INT(JSON_PARSE_DUPLICATE_ROOT, parse_json_string(&json, "\r  \tfalse false \n\r "));
}

static void test_duplicate_root() {
  TEST_ERROR(JSON_PARSE_DUPLICATE_ROOT, "null x");

  /* invalid number */
  TEST_ERROR(JSON_PARSE_DUPLICATE_ROOT, "0123"); /* after zero should be '.' or nothing */
  TEST_ERROR(JSON_PARSE_DUPLICATE_ROOT, "0x0");
  TEST_ERROR(JSON_PARSE_DUPLICATE_ROOT, "0x123");
}

static void test_number() {
  TEST_NUMBER(0.0, "0");
  TEST_NUMBER(0.0, "-0");
  TEST_NUMBER(0.0, "-0.0");
  TEST_NUMBER(1.0, "1");
  TEST_NUMBER(-1.0, "-1");
  TEST_NUMBER(1.5, "1.5");
  TEST_NUMBER(-1.5, "-1.5");
  TEST_NUMBER(3.1416, "3.1416");
  TEST_NUMBER(1E10, "1E10");
  TEST_NUMBER(1e10, "1e10");
  TEST_NUMBER(1E+10, "1E+10");
  TEST_NUMBER(1E-10, "1E-10");
  TEST_NUMBER(-1E10, "-1E10");
  TEST_NUMBER(-1e10, "-1e10");
  TEST_NUMBER(-1E+10, "-1E+10");
  TEST_NUMBER(-1E-10, "-1E-10");
  TEST_NUMBER(1.234E+10, "1.234E+10");
  TEST_NUMBER(1.234E-10, "1.234E-10");
  TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

  TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
  TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
  TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
  TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
  TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
  TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
  TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
  TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
  TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_invalid_value() {
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nul");
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "?");

  /* invalid number */
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+0");
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+1");
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "INF");
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "inf");
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "NAN");
  TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nan");
}
static void test_parse_number_too_big() {
  TEST_ERROR(JSON_PARSE_NUMBER_TO_BIG, "1e309");
  TEST_ERROR(JSON_PARSE_NUMBER_TO_BIG, "-1e309");
}
#define TEST_STRING(expect, json)\
    do {\
        json_value v;  \
        v.type=JSON_NULL;\
        EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v, json));\
        EXPECT_INT(JSON_STRING, get_json_type(&v));\
        EXPECT_EQ_STRING(expect, get_json_str(&v), get_json_str_len(&v));\
        free_json_value(&v);\
    } while(0)

static void test_parse_string() {
  TEST_STRING("", "\"\"");
  TEST_STRING("Hello", "\"Hello\"");
  TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
  TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
  TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
  TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
  TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
  TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
  TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
  TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}
static void test_parse_expect_value() {
  TEST_ERROR(JSON_PARSE_EXPECT_VALUE, "");
  TEST_ERROR(JSON_PARSE_EXPECT_VALUE, " ");
}
static void test_parse_missing_quotation_mark() {
  TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"");
  TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
  TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
  TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
  TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
  TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
  TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
  TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
  TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_access_null() {
  json_value v;
  init_as_null(&v);
  set_json_str(&v, "a", 1);
  set_json_null(&v);
  EXPECT_INT(JSON_NULL, get_json_type(&v));
  free_json_value(&v);
}

static void test_access_boolean() {
  json_value v;
  init_as_null(&v);
  set_json_str(&v, "a", 1);
  set_json_bool(&v, 1);
  EXPECT_TRUE(get_json_bool(&v));
  set_json_bool(&v, 0);
  EXPECT_FALSE(get_json_bool(&v));
  free_json_value(&v);
}

static void test_access_number() {
  json_value v;
  init_as_null(&v);
  set_json_str(&v, "a", 1);
  set_json_number(&v, 1234.5);
  EXPECT_DOUBLE(1234.5, get_json_number(&v));
  free_json_value(&v);
}

static void test_access_string() {
  json_value v;
  init_as_null(&v);
  set_json_str(&v, "", 0);
  EXPECT_EQ_STRING("", get_json_str(&v), get_json_str_len(&v));
  set_json_str(&v, "Hello", 5);
  EXPECT_EQ_STRING("Hello", get_json_str(&v), get_json_str_len(&v));
  free_json_value(&v);
}

static void test_parse_array() {
  size_t i, j;
  json_value v;
  v.type = JSON_NULL;
  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v, "[ ]"));
  EXPECT_INT(JSON_ARRAY, get_json_type(&v));
  EXPECT_EQ_SIZE_T(0, get_json_array_size(&v));
  free_json_value(&v);

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v, "[ null , false , true , 123 , \"abc\" ]"));
  EXPECT_INT(JSON_ARRAY, get_json_type(&v));
  EXPECT_EQ_SIZE_T(5, get_json_array_size(&v));
  EXPECT_INT(JSON_NULL, get_json_type(get_json_array_element(&v, 0)));
  EXPECT_INT(JSON_FALSE, get_json_type(get_json_array_element(&v, 1)));
  EXPECT_INT(JSON_TRUE, get_json_type(get_json_array_element(&v, 2)));
  EXPECT_INT(JSON_NUMBER, get_json_type(get_json_array_element(&v, 3)));
  EXPECT_INT(JSON_STRING, get_json_type(get_json_array_element(&v, 4)));
  EXPECT_DOUBLE(123.0, get_json_number(get_json_array_element(&v, 3)));
  EXPECT_EQ_STRING("abc", get_json_str(get_json_array_element(&v, 4)), get_json_str_len(get_json_array_element(&v, 4)));
  free_json_value(&v);

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
  EXPECT_INT(JSON_ARRAY, get_json_type(&v));
  EXPECT_EQ_SIZE_T(4, get_json_array_size(&v));
  for (i = 0; i < 4; i++) {
    json_value *a = get_json_array_element(&v, i);
    EXPECT_INT(JSON_ARRAY, get_json_type(a));
    EXPECT_EQ_SIZE_T(i, get_json_array_size(a));
    for (j = 0; j < i; j++) {
      json_value *e = get_json_array_element(a, j);
      EXPECT_INT(JSON_NUMBER, get_json_type(e));
      EXPECT_DOUBLE((double) j, get_json_number(e));
    }
  }
  free_json_value(&v);
}
static void test_parse_miss_comma_or_square_bracket() {
  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
//  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
//  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
//  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_object() {
  json_value v;
  size_t i;

  v.type = JSON_NULL;
  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v, " { } "));
  EXPECT_INT(JSON_OBJECT, get_json_type(&v));
  EXPECT_EQ_SIZE_T(0, get_json_object_size(&v));
  free_json_value(&v);

  EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v,
                                              " { "
                                              "\"n\" : null , "
                                              "\"f\" : false , "
                                              "\"t\" : true , "
                                              "\"i\" : 123 , "
                                              "\"s\" : \"abc\", "
                                              "\"a\" : [ 1, 2, 3 ],"
                                              "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                              " } "
  ));
  EXPECT_INT(JSON_OBJECT, get_json_type(&v));
  EXPECT_EQ_SIZE_T(7, get_json_object_size(&v));

  EXPECT_INT(JSON_NULL, get_json_object_value(&v, "n", 1)->type);
  EXPECT_INT(JSON_FALSE, get_json_object_value(&v, "f", 1)->type);
  EXPECT_INT(JSON_TRUE, get_json_object_value(&v, "t", 1)->type);
  EXPECT_INT(JSON_NUMBER, get_json_object_value(&v, "i", 1)->type);
  EXPECT_DOUBLE(123.0, get_json_object_value(&v, "i", 1)->val.n);
  EXPECT_INT(JSON_STRING, get_json_object_value(&v, "s", 1)->type);
  EXPECT_EQ_STRING("abc", get_json_object_value(&v, "s", 1)->val.s.s, 3);

  EXPECT_INT(JSON_ARRAY, get_json_object_value(&v, "a", 1)->type);
  json_value *a = get_json_object_value(&v, "a", 1);
  EXPECT_EQ_SIZE_T(3, a->val.a.size);

  for (i = 0; i < 3; i++) {
    json_value *e = get_json_array_element(a, i);
    EXPECT_INT(JSON_NUMBER, get_json_type(e));
    EXPECT_DOUBLE(i + 1.0, get_json_number(e));
  }

  EXPECT_INT(JSON_OBJECT, get_json_object_value(&v, "o", 1)->type);
  json_value *o = get_json_object_value(&v, "o", 1);
  EXPECT_EQ_SIZE_T(3, get_json_object_size(o));
  for (int j = 0; j < 3; ++j) {
    char k[2];
    k[0] = '1' + j;
    k[1] = '\0';
    json_value *m = get_json_object_value(o, k, 1);
    EXPECT_INT(JSON_NUMBER, m->type);
    EXPECT_DOUBLE(j + 1.0, get_json_number(m));
  }
  free_json_value(&v);
}

static void test_parse_miss_key() {
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{:1,");
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{1:1,");
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{true:1,");
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{false:1,");
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{null:1,");
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{[]:1,");
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{{}:1,");
  TEST_ERROR(JSON_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
  TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\"}");
  TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}
static void test_parse_miss_comma_or_curly_bracket() {
  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_BRACE, "{\"a\":1");
  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_BRACE, "{\"a\":1]");
  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_BRACE, "{\"a\":1 \"b\"");
  TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_BRACE, "{\"a\":{}");
}

#define TEST_ROUNDTRIP(json)\
    do {                    \
        json_value v,v2;\
        char* json2;\
        size_t length;\
        v.type=JSON_NULL;   \
        v2.type=JSON_NULL;\
        EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v, json));\
        json2 = json_stringify(&v, &length);                   \
        EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v2, json2));\
        EXPECT_INT(1, json_compare(&v, &v2));      \
        free_json_value(&v);\
        free_json_value(&v2);\
        free(json2);        \
    } while(0)

static void test_stringify_number() {
  TEST_ROUNDTRIP("0");
  TEST_ROUNDTRIP("-0");
  TEST_ROUNDTRIP("1");
  TEST_ROUNDTRIP("-1");
  TEST_ROUNDTRIP("1.5");
  TEST_ROUNDTRIP("-1.5");
  TEST_ROUNDTRIP("3.25");
  TEST_ROUNDTRIP("1e+20");
  TEST_ROUNDTRIP("1.234e+20");
  TEST_ROUNDTRIP("1.234e-20");

  TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
  TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
  TEST_ROUNDTRIP("-4.9406564584124654e-324");
  TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
  TEST_ROUNDTRIP("-2.2250738585072009e-308");
  TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
  TEST_ROUNDTRIP("-2.2250738585072014e-308");
  TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
  TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
  TEST_ROUNDTRIP("\"\"");
  TEST_ROUNDTRIP("\"Hello\"");
  TEST_ROUNDTRIP("\"Hello\\nWorld\"");
  TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
  TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
  TEST_ROUNDTRIP("[]");
  TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
  TEST_ROUNDTRIP("{}");
  TEST_ROUNDTRIP(
      "{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"p\":{\"1\":1,\"2\":2,\"3\":3},"
      "\"s\":\"abc\",\"a\":[1,2,3,{\"1\":1,\"2\":2,\"3\":3}],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
  TEST_ROUNDTRIP("null");
  TEST_ROUNDTRIP("false");
  TEST_ROUNDTRIP("true");
  test_stringify_number();
  test_stringify_string();
  test_stringify_array();
  test_stringify_object();
}

#define TEST_EQUAL(json1, json2, equality) \
    do {\
        json_value v1, v2;\
        v1.type=JSON_NULL; \
        v2.type=JSON_NULL;\
        EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v1, json1));\
        EXPECT_INT(JSON_PARSE_OK, parse_json_string(&v2, json2));\
        EXPECT_INT(equality, json_compare(&v1, &v2));\
        free_json_value(&v1);\
        free_json_value(&v2);\
    } while(0)

static void test_equal() {
  TEST_EQUAL("true", "true", 1);
  TEST_EQUAL("true", "false", 0);
  TEST_EQUAL("false", "false", 1);
  TEST_EQUAL("null", "null", 1);
  TEST_EQUAL("null", "0", 0);
  TEST_EQUAL("123", "123", 1);
  TEST_EQUAL("123", "456", 0);
  TEST_EQUAL("\"abc\"", "\"abc\"", 1);
  TEST_EQUAL("\"abc\"", "\"abcd\"", 0);
  TEST_EQUAL("[]", "[]", 1);
  TEST_EQUAL("[]", "null", 0);
  TEST_EQUAL("[1,2,3]", "[1,2,3]", 1);
  TEST_EQUAL("[1,2,3]", "[1,2,3,4]", 0);
  TEST_EQUAL("[[]]", "[[]]", 1);
  TEST_EQUAL("{}", "{}", 1);
  TEST_EQUAL("{}", "null", 0);
  TEST_EQUAL("{}", "[]", 0);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
  TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
  TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
  TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

static void test_copy() {
  json_value v1, v2;
  v1.type = JSON_NULL;

  parse_json_string(&v1, "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3],\"s\":{\"i\":true,\"j\":1.6,"
                         "\"k\":[4,5,6],\"q\":{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}}}");
  v2.type = JSON_NULL;
  json_copy(&v2, &v1);
  EXPECT_TRUE(json_compare(&v2, &v1));
  free_json_value(&v1);
  free_json_value(&v2);
}

static void test_move() {
  json_value v1, v2, v3;
  v1.type = JSON_NULL;
  parse_json_string(&v1, "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
  v2.type = JSON_NULL;
  json_copy(&v2, &v1);
  v3.type = JSON_NULL;
  json_move(&v3, &v2);
  EXPECT_INT(JSON_NULL, get_json_type(&v2));
  EXPECT_TRUE(json_compare(&v3, &v1));
  free_json_value(&v1);
  free_json_value(&v2);
  free_json_value(&v3);
}

static void test_swap() {
  json_value v1, v2;
  v1.type = JSON_NULL;
  v2.type = JSON_NULL;
  set_json_str(&v1, "Hello", 5);
  set_json_str(&v2, "World!", 6);
  json_swap(&v1, &v2);
  EXPECT_EQ_STRING("World!", get_json_str(&v1), get_json_str_len(&v1));
  EXPECT_EQ_STRING("Hello", get_json_str(&v2), get_json_str_len(&v2));
  free_json_value(&v1);
  free_json_value(&v2);
}

static void test_access_object() {
  json_value o, v, *pv;
  size_t i;
  init_as_null(&v);
  init_as_object(&o);

  for (i = 0; i < 10; i++) {
    char key[2] = "a";
    key[0] += i;
    init_as_null(&v);
    set_json_number(&v, i);
    int is_new;
    json_value *ov = set_json_object(&o, key, 1, &is_new);
    assert(ov != NULL);
    assert(is_new == 1);
    json_move(ov, &v);
    free_json_value(&v);
  }
  EXPECT_EQ_SIZE_T(10, get_json_object_size(&o));

  for (i = 0; i < 10; i++) {
    char key[] = "a";
    key[0] += i;
    EXPECT_TRUE(json_contains_key(&o, key, 1));
    pv = get_json_object_value(&o, key, 1);
    EXPECT_DOUBLE((double) i, get_json_number(pv));
  }

  EXPECT_TRUE(json_contains_key(&o, "j", 1));
  EXPECT_TRUE(remove_json_object(&o, "j", 1));
  EXPECT_FALSE(json_contains_key(&o, "j", 1));
  EXPECT_EQ_SIZE_T(9, get_json_object_size(&o));

  EXPECT_TRUE(json_contains_key(&o, "a", 1));
  EXPECT_TRUE(remove_json_object(&o, "a", 1));
  EXPECT_FALSE(json_contains_key(&o, "a", 1));
  EXPECT_EQ_SIZE_T(8, get_json_object_size(&o));

  for (i = 0; i < 8; i++) {
    char key[] = "a";
    key[0] += i + 1;
    EXPECT_DOUBLE((double) i + 1, get_json_number(get_json_object_value(&o, key, 1)));
  }

  set_json_str(&v, "Hello", 5);
  json_move(set_json_object(&o, "World", 5, NULL), &v); /* Test if element is freed */
  free_json_value(&v);

  pv = get_json_object_value(&o, "World", 5);
  EXPECT_TRUE(pv != NULL);
  EXPECT_EQ_STRING("Hello", get_json_str(pv), get_json_str_len(pv));

  free_json_value(&o);
}

static void test_access_array() {
  json_value a, e;
  size_t i, j;

  init_as_null(&a);

  for (j = 0; j <= 5; j += 5) {
    init_as_array(&a, j);
    EXPECT_EQ_SIZE_T(0, get_json_array_size(&a));
    EXPECT_EQ_SIZE_T(j, get_json_array_capacity(&a));
    for (i = 0; i < 10; i++) {
      init_as_null(&e);
      set_json_number(&e, i);
      json_move(insert_json_array_last(&a), &e);
      free_json_value(&e);
    }

    EXPECT_EQ_SIZE_T(10, get_json_array_size(&a));
    for (i = 0; i < 10; i++)
      EXPECT_DOUBLE((double) i, get_json_number(get_json_array_element(&a, i)));

  }

  remove_json_array_last(&a);
  EXPECT_EQ_SIZE_T(9, get_json_array_size(&a));
  for (i = 0; i < 9; i++)
    EXPECT_DOUBLE((double) i, get_json_number(get_json_array_element(&a, i)));

  remove_json_array_element(&a, 4, 0);
  EXPECT_EQ_SIZE_T(9, get_json_array_size(&a));
  for (i = 0; i < 9; i++)
    EXPECT_DOUBLE((double) i, get_json_number(get_json_array_element(&a, i)));

  remove_json_array_element(&a, 8, 1);
  EXPECT_EQ_SIZE_T(8, get_json_array_size(&a));
  for (i = 0; i < 8; i++)
    EXPECT_DOUBLE((double) i, get_json_number(get_json_array_element(&a, i)));

  remove_json_array_element(&a, 0, 2);
  EXPECT_EQ_SIZE_T(6, get_json_array_size(&a));
  for (i = 0; i < 6; i++)
    EXPECT_DOUBLE((double) i + 2, get_json_number(get_json_array_element(&a, i)));

  for (i = 0; i < 2; i++) {
    init_as_null(&e);
    set_json_number(&e, i);
    json_move(insert_json_array(&a, i), &e);
    free_json_value(&e);
  }

  EXPECT_EQ_SIZE_T(8, get_json_array_size(&a));
  for (i = 0; i < 8; i++)
    EXPECT_DOUBLE((double) i, get_json_number(get_json_array_element(&a, i)));

  EXPECT_TRUE(get_json_array_capacity(&a) > 8);
  shrink_json_array(&a);
  EXPECT_EQ_SIZE_T(8, get_json_array_capacity(&a));
  EXPECT_EQ_SIZE_T(8, get_json_array_size(&a));
  for (i = 0; i < 8; i++)
    EXPECT_DOUBLE((double) i, get_json_number(get_json_array_element(&a, i)));

  set_json_str(&e, "Hello", 5);
  json_move(insert_json_array_last(&a), &e);     /* Test if element is freed */
  free_json_value(&e);

  i = get_json_array_capacity(&a);
  clear_json_array(&a);
  EXPECT_EQ_SIZE_T(0, get_json_array_size(&a));
  EXPECT_EQ_SIZE_T(i, get_json_array_capacity(&a));   /* capacity remains unchanged */
  shrink_json_array(&a);
  EXPECT_EQ_SIZE_T(0, get_json_array_capacity(&a));

  free_json_value(&a);
}

static void test_dump_file() {
  json_value v1;
  v1.type = JSON_NULL;

  parse_json_string(&v1, "{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3],\"s\":{\"i\":true,\"j\":1.6,"
                         "\"k\":[4,5,6],\"q\":{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}},\"ss\":\""
                         "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssseeeeeeeeeee"
                         "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeettttttttttttttttttttttttttt"
                         "tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttrrrrrrrrr"
                         "rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr"
                         "rrrrrrrrrrrnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn"
                         "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
                         "ooooooooooooo\"}");
  EXPECT_INT(JSON_DUMP_OK, json_dump(&v1, "./test.json"));

  free_json_value(&v1);

  EXPECT_INT(JSON_PARSE_OK, json_load(&v1, "./test.json"));
  char *s = json_stringify(&v1, NULL);
  printf("%s\n", s);
  free(s);
}

static UnitTestFunction tests[] = {
    test_parse_null,
    test_parse_true,
    test_parse_false,
    test_number,
    test_parse_invalid_value,
    test_duplicate_root,
    test_parse_number_too_big,
    test_parse_string,
    test_parse_expect_value,
    test_parse_missing_quotation_mark,
    test_parse_invalid_string_escape,
    test_parse_invalid_string_char,
    test_access_null,
    test_access_boolean,
    test_access_number,
    test_access_string,
    test_parse_invalid_unicode_hex,
    test_parse_invalid_unicode_surrogate,
    test_parse_array,
    test_parse_miss_comma_or_square_bracket,
    test_parse_object,
    test_parse_miss_key,
    test_parse_miss_colon,
    test_parse_miss_comma_or_curly_bracket,
    test_stringify,
    test_equal,
    test_copy,
    test_move,
    test_swap,
    test_access_object,
    test_access_array,
    test_dump_file,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);
  return 0;
}