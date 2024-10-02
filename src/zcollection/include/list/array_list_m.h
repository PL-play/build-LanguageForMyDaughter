//
// Created by ran on 2024-03-30.
//
#ifndef ZCOLLECTION_ARRAY_LIST_M_H_
#define ZCOLLECTION_ARRAY_LIST_M_H_

#define M_ARRAYLIST_ERROR (-1)
#define M_ARRAYLIST_SUCCESS 0
#define M_ARRAYLIST_INVALID_POSITION 1
#define M_ARRAYLIST_POSITION_EMPTY 2

#define BASE_ARRAY_LIST(__name) \
        typedef struct {               \
           size_t size; \
        } __name;

#define DECLARE_ARRAY_LIST(__val_type, __name) \
typedef __val_type  __name##ArrayListData;\
typedef struct  {                             \
        size_t size;\
        size_t capacity;\
        __name##ArrayListData *data;\
      } __name##ArrayList;                                      \
typedef int (*__name##ArrayListDataCompare)(__name##ArrayListData data1, __name##ArrayListData data2);                                  \
__name##ArrayList *__name##new_arraylist(size_t init_capacity);\
void __name##free_arraylist(__name##ArrayList *array_list);    \
int __name##insert_arraylist(__name##ArrayList *array_list, size_t index, __name##ArrayListData data);\
int __name##append_arraylist(__name##ArrayList *array_list, __name##ArrayListData data);              \
int __name##prepend_arraylist(__name##ArrayList *array_list, __name##ArrayListData data);             \
int __name##remove_arraylist(__name##ArrayList *array_list, size_t index);                                                              \
int __name##set_arraylist_data(__name##ArrayList *array_list, size_t index, __name##ArrayListData data);                       \
int __name##index_of_arraylist(__name##ArrayList *array_list, __name##ArrayListData data, __name##ArrayListDataCompare comparator);     \
__name##ArrayListData __name##get_data_arraylist(__name##ArrayList *array_list, size_t index);                                          \
void __name##clear_arraylist(__name##ArrayList *array_list);

#define DEFINE_ARRAY_LIST(__null_data, __name) \
static int __name##grow_capacity(__name##ArrayList *array_list); \
__name##ArrayList *__name##new_arraylist(size_t init_capacity) {  \
  if (init_capacity == 0) {  \
    init_capacity = 8;  \
  }  \
  \
  __name##ArrayList *array_list = malloc(sizeof(__name##ArrayList));  \
  if (array_list == NULL) {  \
    return NULL;  \
  }  \
  \
  array_list->data = malloc(sizeof(__name##ArrayListData) * init_capacity);  \
  if (array_list->data == NULL) {  \
    free(array_list);  \
    return NULL;  \
  }  \
  \
  array_list->capacity = init_capacity;  \
  array_list->size = 0;  \
  \
  return array_list;  \
}\
void __name##free_arraylist(__name##ArrayList *array_list) {  \
  if (array_list != NULL) {  \
    free(array_list->data);  \
    free(array_list);  \
  }  \
}\
int __name##insert_arraylist(__name##ArrayList *array_list, size_t index, __name##ArrayListData data) {  \
  if (array_list->size >= array_list->capacity) {  \
    int ret = __name##grow_capacity(array_list);  \
    if (ret != M_ARRAYLIST_SUCCESS) {  \
      return ret;  \
    }  \
  }  \
  if (index > array_list->size) {  \
    return  M_ARRAYLIST_INVALID_POSITION;  \
  }  \
  if (index != array_list->size) {  \
    memmove(&array_list->data[index + 1],  \
            &array_list->data[index],  \
            (array_list->size - index) * sizeof(__name##ArrayListData));  \
  }  \
  array_list->data[index] = data;  \
  array_list->size++;  \
  return  M_ARRAYLIST_SUCCESS;  \
}  \
int __name##append_arraylist(__name##ArrayList *array_list, __name##ArrayListData data) {  \
  return __name##insert_arraylist(array_list, array_list->size, data);  \
}  \
  \
int __name##prepend_arraylist(__name##ArrayList *array_list, __name##ArrayListData data) {  \
  return __name##insert_arraylist(array_list, 0, data);  \
}\
int __name##remove_arraylist(__name##ArrayList *array_list, size_t index) { \
  if (index == array_list->size - 1) { \
    array_list->data[index] = __null_data; \
    array_list->size--; \
    return  M_ARRAYLIST_SUCCESS; \
  } \
  if (index >= array_list->size) { \
    return  M_ARRAYLIST_INVALID_POSITION; \
  } \
  memmove(&array_list->data[index], \
          &array_list->data[index + 1], \
          (array_list->size - (index + 1)) * sizeof(__name##ArrayListData)); \
  array_list->size--; \
  return  M_ARRAYLIST_SUCCESS;       \
}\
int __name##index_of_arraylist(__name##ArrayList *array_list, __name##ArrayListData data, __name##ArrayListDataCompare comparator) { \
  for (size_t i = 0; i < array_list->size; i++) { \
    if (comparator(data, array_list->data[i]) == 0) { \
      return i; \
    } \
  } \
  return -1; \
} \
 \
__name##ArrayListData __name##get_data_arraylist(__name##ArrayList *array_list, size_t index) { \
  if (index >= array_list->size) { \
    return __null_data; \
  } \
  return array_list->data[index]; \
} \
 \
int __name##set_arraylist_data(__name##ArrayList *array_list, size_t index, __name##ArrayListData data) { \
  if (index >= array_list->size) { \
    return  M_ARRAYLIST_INVALID_POSITION; \
  } \
  array_list->data[index] = data; \
  return  M_ARRAYLIST_SUCCESS; \
} \
 \
void __name##clear_arraylist(__name##ArrayList *array_list) { \
  array_list->size = 0; \
}\
static int __name##grow_capacity(__name##ArrayList *array_list) {\
  size_t new_capacity = array_list->capacity * 2;\
  __name##ArrayListData *newData = realloc(array_list->data, sizeof(__name##ArrayListData) * new_capacity);\
  if (newData == NULL) {\
    return  M_ARRAYLIST_ERROR;\
  }\
  array_list->data = newData;\
  array_list->capacity = new_capacity;\
  return  M_ARRAYLIST_SUCCESS;\
}

#endif