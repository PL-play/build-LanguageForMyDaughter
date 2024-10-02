//
// Created by ran on 2024-03-30.
//

#ifndef ZCOLLECTION_LIST_LINKED_LIST_M_H_
#define ZCOLLECTION_LIST_LINKED_LIST_M_H_

#define M_LINKED_LIST_SUCCESS 0
#define M_LINKED_LIST_OPT_ON_NULL_ERROR (-1)

#define DECLARE_LINKED_LIST(__val_type,__name) \
typedef __val_type  __name##ListValue;                 \
typedef struct __name##LinkedListNode __name##LinkedListNode;  \
typedef struct __name##LinkedList __name##LinkedList;  \
typedef int (*__name##LinkedListValueCompare)(__name##ListValue data1, __name##ListValue data2);\
__name##LinkedList *__name##new_linked_list();\
void __name##free_linked_list(__name##LinkedList *, void(*free_func)(__val_type));              \
int __name##append_list(__name##LinkedList *list, __name##ListValue data);                      \
int __name##insert_list(__name##LinkedList *list, size_t index, __name##ListValue data);\
int __name##prepend_list(__name##LinkedList *list, __name##ListValue data);                     \
int __name##remove_list_first(__name##LinkedList *list);       \
__name##ListValue __name##remove_list(__name##LinkedList* list,size_t index);                  \
__name##ListValue __name##peek_list_first(__name##LinkedList *list);                         \
__name##ListValue __name##peek_list_last(__name##LinkedList *list);\
int __name##remove_list_last(__name##LinkedList *list);\
int __name##remove_list_equals_to(__name##LinkedList *list, __name##ListValue data, __name##LinkedListValueCompare);\
__name##ListValue __name##get_list_data(__name##LinkedList *list, size_t index);                                               \
__name##ListValue __name##data_of_node_linked_list(__name##LinkedListNode *node);\
__name##LinkedListNode *__name##head_of_list(__name##LinkedList *list);\
__name##LinkedListNode *__name##node_of_list(__name##LinkedList *list,size_t index);            \
__name##LinkedListNode *__name##prev_node_linked_list(__name##LinkedListNode *node);                                               \
__name##LinkedListNode *__name##next_node_linked_list(__name##LinkedListNode *node);\
int __name##set_list_data(__name##LinkedList *list, size_t index, __name##ListValue value);\
size_t __name##list_size(__name##LinkedList *list);\
__name##ListValue *__name##value_array(__name##LinkedList *list);                                               \

#define DEFINE_LINKED_LIST(__val_type,__null_val,__name) \
struct __name##LinkedListNode { \
  __name##ListValue data; \
  struct __name##LinkedListNode *prev; \
  struct __name##LinkedListNode *next; \
}; \
 \
struct __name##LinkedList { \
  __name##LinkedListNode *head; \
  size_t size; \
}; \
 \
static __name##LinkedListNode *__name##new_linked_list_node(__name##ListValue data); \
 \
 \
static void __name##free_linked_list_node(__name##LinkedListNode *head); \
 \
static void __name##free_linked_list_data(__name##LinkedListNode *head, void(*free_func)(__val_type)); \
 \
static __name##LinkedListNode *__name##append_linked_list_node(__name##LinkedListNode **head, __name##ListValue data); \
 \
 \
static __name##LinkedListNode *__name##prepend_linked_list_node(__name##LinkedListNode **head, __name##ListValue data); \
 \
 \
static int __name##remove_head_linked_list(__name##LinkedListNode **head); \
 \
 \
static int __name##remove_tail_linked_list(__name##LinkedListNode **head); \
 \
static unsigned int __name##remove_data_linked_list(__name##LinkedListNode **head, __name##ListValue data, __name##LinkedListValueCompare); \
 \
static int __name##remove_node_linked_list(__name##LinkedListNode **head, __name##LinkedListNode *node); \
 \
static __name##ListValue __name##data_of_linked_list(__name##LinkedListNode *head, size_t index); \
 \
 \
static __name##LinkedListNode *__name##search_linked_list(__name##LinkedListNode *head, __name##ListValue data, __name##LinkedListValueCompare compare); \
 \
static int __name##set_data_of_linked_list(__name##LinkedListNode *head, size_t index, __name##ListValue value); \
 \
 \
static __name##ListValue *__name##linked_list_to_array(__name##LinkedListNode *head); \
 \
static __name##ListValue __name##peek_list_head(__name##LinkedListNode *head); \
static __name##ListValue __name##peek_list_tail(__name##LinkedListNode *head); \
 \
static unsigned int __name##size_of_linked_list(__name##LinkedListNode *head); \
 \
 \
static __name##LinkedListNode *__name##node_of_linked_list(__name##LinkedListNode *head, size_t index); \
 \
static __name##LinkedListNode *__name##new_linked_list_node(__name##ListValue data) { \
  __name##LinkedListNode *new_list = malloc(sizeof(__name##LinkedListNode)); \
  new_list->data = data; \
  new_list->next = new_list; \
  new_list->prev = new_list; \
  return new_list; \
} \
 \
__name##LinkedList *__name##new_linked_list() { \
  __name##LinkedList *list = malloc(sizeof(__name##LinkedList)); \
  if (!list) return NULL; \
  list->size = 0; \
  list->head = NULL; \
  return list; \
} \
 \
 \
static void __name##free_linked_list_node(__name##LinkedListNode *head) { \
  __name##free_linked_list_data(head, NULL); \
} \
 \
void __name##free_linked_list(__name##LinkedList *list, void(*free_func)(__val_type)) { \
  if (list) { \
    if (list->head) { \
      __name##free_linked_list_data(list->head, free_func); \
    } \
    free(list); \
  } \
} \
 \
static void __name##free_linked_list_data(__name##LinkedListNode *head, void(*free_func)(__val_type)) { \
  if (head == NULL) return; \
 \
  __name##LinkedListNode *cur = head; \
  do { \
    __name##LinkedListNode *next = cur->next; \
    if (free_func) { \
      free_func(cur->data); \
    } \
    free(cur); \
    cur = next; \
  } while (cur != head); \
} \
 \
 \
static __name##LinkedListNode *__name##append_linked_list_node(__name##LinkedListNode **head, __name##ListValue data) { \
  __name##LinkedListNode *newNode = malloc(sizeof(__name##LinkedListNode)); \
  if (newNode == NULL) { \
    return NULL; \
  } \
  if (*head == NULL) { \
    *head = newNode; \
    (*head)->next = *head; \
    (*head)->prev = *head; \
    (*head)->data = data; \
    return *head; \
  } \
  newNode->data = data; \
  __name##LinkedListNode *tail = (*head)->prev; \
  tail->next = newNode; \
  newNode->prev = tail; \
  newNode->next = *head; \
  (*head)->prev = newNode; \
  return newNode; \
} \
 \
int __name##append_list(__name##LinkedList *list, __name##ListValue data) { \
  if (__name##append_linked_list_node(&list->head, data)) { \
    list->size++; \
    return 1; \
 \
  } \
  return 0; \
} \
 \
int __name##insert_list(__name##LinkedList *list, size_t index, __name##ListValue data) { \
  if (index < 0 || index > list->size) { \
    return 0; \
  } \
  if (index == 0 || index == list->size) { \
    return __name##append_list(list, data); \
  } \
  __name##LinkedListNode *cur = list->head; \
  for (int i = 0; i < index - 1; i++) { \
    cur = cur->next; \
  } \
  __name##LinkedListNode *newNode = malloc(sizeof(__name##LinkedListNode)); \
  if (newNode == NULL) { \
    return 0; \
  } \
  newNode->data = data; \
  __name##LinkedListNode *next = cur->next; \
  cur->next = newNode; \
  newNode->prev = cur; \
  newNode->next = next; \
  next->prev = newNode; \
  list->size++; \
  return 1; \
} \
 \
static __name##LinkedListNode *__name##prepend_linked_list_node(__name##LinkedListNode **head, __name##ListValue data) { \
  __name##LinkedListNode *newNode = malloc(sizeof(__name##LinkedListNode)); \
  if (newNode == NULL) { \
    return NULL; \
  } \
  if (*head == NULL) { \
    *head = newNode; \
    (*head)->next = *head; \
    (*head)->prev = *head; \
    (*head)->data = data; \
    return *head; \
  } \
  newNode->data = data; \
  __name##LinkedListNode *tail = (*head)->prev; \
  tail->next = newNode; \
  newNode->next = *head; \
  newNode->prev = tail; \
  (*head)->prev = newNode; \
  *head = newNode; \
  return newNode; \
} \
 \
int __name##prepend_list(__name##LinkedList *list, __name##ListValue data) { \
  if (__name##prepend_linked_list_node(&list->head, data)) { \
    list->size++; \
    return 1; \
  } \
  return 0; \
} \
 \
static int __name##remove_head_linked_list(__name##LinkedListNode **head) { \
  if (*head == NULL) return M_LINKED_LIST_OPT_ON_NULL_ERROR; \
  if ((*head)->next == *head) { \
    free(*head); \
    *head = NULL; \
    return M_LINKED_LIST_SUCCESS; \
  } \
  __name##LinkedListNode *new_head = (*head)->next; \
  __name##LinkedListNode *tail = (*head)->prev; \
 \
  new_head->prev = tail; \
  tail->next = new_head; \
  free(*head); \
 \
  *head = new_head; \
  return M_LINKED_LIST_SUCCESS; \
} \
 \
int __name##remove_list_first(__name##LinkedList *list) { \
  if (M_LINKED_LIST_SUCCESS == __name##remove_head_linked_list(&list->head)) { \
    list->size--; \
    return 1; \
  } \
  return 0; \
} \
 \
__name##ListValue __name##remove_list(__name##LinkedList *list, size_t index) { \
  if (index < 0 || index > list->size - 1) { \
    return __null_val; \
  } \
  __name##LinkedListNode *node = list->head; \
  for (int i = 0; i < index; i++) { \
    node = node->next; \
  } \
  __name##ListValue value = node->data; \
  if (M_LINKED_LIST_SUCCESS == __name##remove_node_linked_list(&list->head, node)) { \
    list->size--; \
    return value; \
  } \
  return __null_val; \
} \
 \
static __name##ListValue __name##peek_list_head(__name##LinkedListNode *head) { \
  if (head == NULL) return __null_val; \
  return head->data; \
} \
 \
__name##ListValue __name##peek_list_first(__name##LinkedList *list) { \
  return __name##peek_list_head(list->head); \
} \
 \
static __name##ListValue __name##peek_list_tail(__name##LinkedListNode *head) { \
  if (head == NULL) return __null_val; \
  return head->prev->data; \
} \
 \
__name##ListValue __name##peek_list_last(__name##LinkedList *list) { \
  return __name##peek_list_tail(list->head); \
} \
 \
static int __name##remove_tail_linked_list(__name##LinkedListNode **head) { \
  if (*head == NULL) return M_LINKED_LIST_OPT_ON_NULL_ERROR; \
  if ((*head)->next == *head) { \
    free(*head); \
    *head = NULL; \
    return M_LINKED_LIST_SUCCESS; \
  } \
 \
  __name##LinkedListNode *tail = (*head)->prev; \
  __name##LinkedListNode *new_tail = tail->prev; \
  new_tail->next = *head; \
  (*head)->prev = new_tail; \
  free(tail); \
  return M_LINKED_LIST_SUCCESS; \
} \
int __name##remove_list_last(__name##LinkedList *list) { \
  if (M_LINKED_LIST_SUCCESS == __name##remove_tail_linked_list(&list->head)) { \
    list->size--; \
    return 1; \
  } \
  return 0; \
} \
 \
 \
static unsigned int __name##remove_data_linked_list(__name##LinkedListNode **head, __name##ListValue data, __name##LinkedListValueCompare comparator) { \
  if (*head == NULL) return 0; \
  __name##LinkedListNode *cur = *head; \
  __name##LinkedListNode *next; \
  unsigned int count = 0; \
  char head_shifted; \
  do { \
    head_shifted = 0; \
    next = cur->next; \
    if (comparator(data, cur->data) == 0) { \
      if (cur == *head) { \
        if (cur->next == cur) { \
          *head = NULL; \
          free(cur); \
          count++; \
          break; \
        } else { \
          *head = cur->next; \
          head_shifted = 1; \
        } \
      } \
      cur->prev->next = cur->next; \
      cur->next->prev = cur->prev; \
      free(cur); \
      count++; \
    } \
    cur = next; \
  } while (*head != NULL && (head_shifted || cur != *head)); \
  return count; \
} \
 \
int __name##remove_list_equals_to(__name##LinkedList *list, __name##ListValue data, __name##LinkedListValueCompare comparator) { \
  int count = (int) __name##remove_data_linked_list(&list->head, data, comparator); \
  list->size -= count; \
  return count; \
} \
 \
 \
static int __name##remove_node_linked_list(__name##LinkedListNode **head, __name##LinkedListNode *node) { \
  if (*head == NULL || node == NULL) return M_LINKED_LIST_OPT_ON_NULL_ERROR; \
  if (node->next == node) { \
    free(node); \
    *head = NULL; \
    return M_LINKED_LIST_SUCCESS; \
  } else { \
    if (*head == node) { \
      *head = node->next; \
    } \
    node->prev->next = node->next; \
    node->next->prev = node->prev; \
    free(node); \
    return M_LINKED_LIST_SUCCESS; \
  } \
} \
 \
 \
static __name##ListValue __name##data_of_linked_list(__name##LinkedListNode *head, size_t index) { \
  __name##LinkedListNode *node = __name##node_of_linked_list(head, index); \
  return node == NULL ? __null_val : node->data; \
} \
 \
__name##ListValue __name##get_list_data(__name##LinkedList *list, size_t index) { \
  return __name##data_of_linked_list(list->head, index); \
} \
 \
 \
__name##ListValue __name##data_of_node_linked_list(__name##LinkedListNode *node) { \
  if (node == NULL) return __null_val; \
  return node->data; \
} \
 \
__name##LinkedListNode *__name##prev_node_linked_list(__name##LinkedListNode *node) { \
  return node == NULL ? NULL : node->prev; \
} \
 \
 \
__name##LinkedListNode *__name##next_node_linked_list(__name##LinkedListNode *node) { \
  return node == NULL ? NULL : node->next; \
} \
 \
static __name##LinkedListNode *__name##node_of_linked_list(__name##LinkedListNode *head, size_t index) { \
  if (head == NULL || index >= __name##size_of_linked_list(head)) { \
    return NULL; \
  } \
  __name##LinkedListNode *node = head; \
  for (size_t i = 0; i < index; i++) { \
    node = node->next; \
  } \
  return node; \
} \
 \
__name##LinkedListNode *__name##node_of_list(__name##LinkedList *list, size_t index) { \
  return __name##node_of_linked_list(list->head, index); \
} \
__name##LinkedListNode *__name##head_of_list(__name##LinkedList *list) { \
  return list->head; \
} \
 \
static __name##LinkedListNode *__name##search_linked_list(__name##LinkedListNode *head, __name##ListValue data, __name##LinkedListValueCompare compare) { \
  if (head == NULL) return NULL; \
  __name##LinkedListNode *cur = head; \
  __name##LinkedListNode *ret = NULL; \
  do { \
    if (compare(data, cur->data) == 0) { \
      __name##append_linked_list_node(&ret, data); \
    } \
    cur = cur->next; \
  } while (cur != head); \
  return ret; \
} \
 \
static int __name##set_data_of_linked_list(__name##LinkedListNode *head, size_t index, __name##ListValue value) { \
  __name##LinkedListNode *node = __name##node_of_linked_list(head, index); \
  if (node == NULL) { \
    return M_LINKED_LIST_OPT_ON_NULL_ERROR; \
  } \
  node->data = value; \
  return M_LINKED_LIST_SUCCESS; \
} \
int __name##set_list_data(__name##LinkedList *list, size_t index, __name##ListValue value) { \
  if (__name##set_data_of_linked_list(list->head, index, value) == M_LINKED_LIST_SUCCESS) { \
    return 1; \
  } \
  return 0; \
} \
 \
static unsigned int __name##size_of_linked_list(__name##LinkedListNode *head) { \
  if (head == NULL) return 0; \
  __name##LinkedListNode *tail = head->prev; \
  __name##LinkedListNode *cur = head; \
  unsigned int count = 0; \
  while (cur != tail) { \
    count++; \
    cur = cur->next; \
  } \
  return count + 1; \
} \
size_t __name##list_size(__name##LinkedList *list) { \
  return list->size; \
} \
 \
 \
static __name##ListValue *__name##linked_list_to_array(__name##LinkedListNode *head) { \
  if (head == NULL) { \
    return NULL; \
  } \
  unsigned int size = __name##size_of_linked_list(head); \
  __name##ListValue *arr = malloc(sizeof(__name##ListValue) * size); \
  if (arr == NULL) return NULL; \
  __name##LinkedListNode *cur = head; \
  for (int i = 0; i < size; i++) { \
    arr[i] = cur->data; \
    cur = cur->next; \
  } \
  return arr; \
} \
 \
__name##ListValue *__name##value_array(__name##LinkedList *list) { \
  return __name##linked_list_to_array(list->head); \
}
#endif //ZCOLLECTION_LIST_LINKED_LIST_M_H_
