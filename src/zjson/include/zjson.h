//
// Created by ran on 2024/3/1.
//

#ifndef ZJSON_JSON_H_
#define ZJSON_JSON_H_
#include <stdlib.h>
#include <stdint.h>
#include "hashtable/hash_table_m.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef char json_value_type;
#define JSON_NULL (json_value_type)1
#define JSON_FALSE (json_value_type)2
#define JSON_TRUE (json_value_type)3
#define JSON_NUMBER (json_value_type)4
#define JSON_STRING (json_value_type)5
#define JSON_ARRAY (json_value_type)6
#define JSON_OBJECT (json_value_type)7

#define JSON_PARSE_OK 0
#define JSON_PARSE_EXPECT_VALUE 1
#define JSON_PARSE_INVALID_VALUE 2
#define JSON_PARSE_DUPLICATE_ROOT 3
#define JSON_PARSE_NUMBER_TO_BIG 4
#define JSON_PARSE_MISS_QUOTATION_MARK 5
#define JSON_PARSE_INVALID_STRING_ESCAPE 6
#define JSON_PARSE_INVALID_STRING_CHAR 7
#define JSON_PARSE_INVALID_UNICODE_HEX 8
#define JSON_PARSE_INVALID_UNICODE_SURROGATE 9
#define JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET 10
#define JSON_PARSE_MISS_COLON 11
#define JSON_PARSE_MISS_COMMA_OR_BRACE 12
#define JSON_PARSE_MISS_KEY 13

#define JSON_DUMP_OK 0
#define JSON_DUMP_FILE_ERROR 20
#define JSON_LOAD_FILE_ERROR 30

typedef struct json_string {
  char *s;   // Pointer to the string data
  size_t len; // Length of the string
} json_string;

typedef struct json_array {
  struct json_value *e;
  size_t size;
  size_t capacity;
} json_array; //array

DECLARE_HASHTABLE(json_string*, struct json_value*, JSON)

//typedef Hashtable *json_object; // key is json_string, value is json_value pointer
typedef JSONHashtable *json_obj;
typedef double json_number;

typedef struct json_value {
  json_value_type type;
  union {
    json_number n; //number
    json_string s; //string
    json_array a; //array
    json_obj o; //object
  } val;
} json_value;

#define init_as_null(v) do{ (v)->type = JSON_NULL;}while(0)
#define init_as_object(v) do{ (v)->type = JSON_OBJECT; (v)->val.o=NULL;}while(0)
#define init_as_array(v, c) do{       free_json_value((v)); \
                                      (v)->type = JSON_ARRAY; \
                                      (v)->val.a.size = 0;\
                                      (v)->val.a.capacity = (c);\
                                      (v)->val.a.e = (c) > 0 ? malloc((c) * sizeof(json_value)) : NULL;}while(0)

json_value *new_null();

/**
 * Parse json string
 *
 * @param json
 * @return
 */
int parse_json_string(json_value *, const char *json);
json_value_type get_json_type(json_value *);
void free_json_value(json_value *);

void set_json_null(json_value *);

void set_json_number(json_value *, double n);
json_number get_json_number(json_value *);

#define set_json_str_sk(v, k) set_json_str((v), (k), strlen(k))

void set_json_str(json_value *, const char *, size_t);
const char *get_json_str(json_value *);
size_t get_json_str_len(json_value *);

void set_json_bool(json_value *, int);
int get_json_bool(json_value *);

size_t get_json_array_size(json_value *);
json_value *get_json_array_element(json_value *, size_t);
size_t get_json_array_capacity(json_value *value);
json_value *insert_json_array(json_value *value, size_t index);
void remove_json_array_element(json_value *value, size_t start, size_t count);
void clear_json_array(json_value *value);
#define insert_json_array_last(v) insert_json_array((v),get_json_array_size(v))
#define insert_json_array_first(v)  insert_json_array((v),0)
#define remove_json_array_last(v) do{ \
                                  if(get_json_array_size((v))>0) remove_json_array_element((v),get_json_array_size((v))-1,1);} \
                                  while(0)
#define remove_json_array_first(v) do{ \
                                  if(get_json_array_size((v))>0) remove_json_array_element((v),0,1);} \
                                  while(0)
void shrink_json_array(json_value *value);

size_t get_json_object_size(const json_value *);
json_value *get_json_object_value(const json_value *, char *key, size_t key_len);
int json_contains_key(const json_value *, char *key, size_t key_len);

#define set_json_object_sk(v, k)  (set_json_object((v), (k), strlen(k), NULL))

/**
 * set json object. return the pointer of the value. If the key is not exist, create a new key and value.
 * otherwise return the existing value.
 *
 * @param v
 * @param key
 * @param key_len
 * @param is_new
 * @return
 */
json_value *set_json_object(json_value *v, char *key, size_t key_len, int *is_new);
/**
 * remove and free a json value by key
 *
 * @param v
 * @param key
 * @param key_len
 * @return
 */
int remove_json_object(json_value *v, char *key, size_t key_len);

/**
 * stringify a json value.
 *
 * @param value
 * @param json
 * @param len
 * @return
 */
char *json_stringify(const json_value *value, size_t *len);

/**
 * dump json value to file.
 *
 * @param value
 * @param fp
 * @return
 */
int json_dump(const json_value *value, char *fp);

/**
 * load json from file
 *
 * @param value
 * @param fp
 * @return
 */
int json_load(json_value *value, char *fp);

int json_compare(const json_value *json1, const json_value *json2);

void json_copy(json_value *dest, const json_value *src);

/**
 * move the src to dest. This will free dest before move begin and set src to JSON_NULL after move is finished.
 *
 * @param dest
 * @param src
 */
void json_move(json_value *dest, json_value *src);

void json_swap(json_value *v1, json_value *v2);
#ifdef __cplusplus
}
#endif
#endif //ZJSON_JSON_H_
