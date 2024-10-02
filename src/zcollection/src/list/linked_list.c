//
// Created by ran on 2024/1/2.
//
#include "list/linked_list.h"
#include <stdlib.h>


#define LINKED_LIST_SUCCESS 0
#define LINKED_LIST_OPT_ON_NULL_ERROR (-1)

struct LinkedListNode {
  ListValue data;
  struct LinkedListNode *prev;
  struct LinkedListNode *next;
};

struct LinkedList {
  LinkedListNode *head;
  int size;
};

/**
 * Create a linked list and return the node.
 *
 * @return  LinkedList *    Pointer to the newly created list node.
 */
static LinkedListNode *new_linked_list_node(ListValue data);

/**
 * Free the linked list.
 */
static void free_linked_list_node(LinkedListNode *head);
/**
 * Free the linked list.
 */
static void free_linked_list_node(LinkedListNode *head);

/**
 * Free the linked list and its data value
 */
static void free_linked_list_data(LinkedListNode *head, void(*free_func)(void *));
/**
 * Append a value to the end of list.
 *
 * @param head  start pointer of list
 * @param data  the data
 * @return      The new entry in the list, or NULL if it was not
 *              possible to allocate the memory for the new entry.
 */
static LinkedListNode *append_linked_list_node(LinkedListNode **head, ListValue data);

/**
 * Append a value to the start of list.
 *
 * @param head  start pointer of list
 * @param data  the data
 * @return      The new entry in the list, or NULL if it was not
 *              possible to allocate the memory for the new entry.
 */
static LinkedListNode *prepend_linked_list_node(LinkedListNode **head, ListValue data);

/**
 * Delete the first node in list and change the head to the new list.
 *
 * @param head  start pointer of list
 * @return      zero if successful
 */
static int remove_head_linked_list(LinkedListNode **head);

/**
 * Delete the last node in list.
 *
 * @param head  start pointer of list
 * @return      zero if successful
 */
static int remove_tail_linked_list(LinkedListNode **head);
/**
 * Remove nodes from list if any data of that node equals to the given data.
 *
 * @param head start pointer of list
 * @param data data to
 * @return
 */
static unsigned int remove_data_linked_list(LinkedListNode **head, ListValue data, LinkedListValueCompare);
/**
 * Remove an entry from a list.
 *
 * @param head  start pointer of list
 * @param node  entry to remove
 * @return      Returns zero if successful, non-zero if unsuccessful
 */
static int remove_node_linked_list(LinkedListNode **head, LinkedListNode *node);
/**
 * Retrieve the data at specific index.
 *
 * @param head  start pointer of list
 * @param index index of list
 * @return      The value at specific index. NULL if unsuccessful or index is wrong.
 */
static ListValue data_of_linked_list(LinkedListNode *head, unsigned int index);

/**
 * Search the list by given data.
 *
 * @param head  start pointer of list
 * @param data  data to be searched
 * @param compare   compare function
 * @return  list of nodes in which data equals to given one.
 */
static LinkedListNode *search_linked_list(LinkedListNode *head, ListValue data, LinkedListValueCompare compare);
/**
 * Set the value at specific node of list.
 *
 * @param head  start pointer of list
 * @param index index of list
 * @param value value to be set
 * @return      zero if successful,no-zero if unsuccessful.
 */
static int set_data_of_linked_list(LinkedListNode *head, unsigned int index, ListValue value);

/**
 * Return an array copied from data of list nodes.
 *
 * @param head  start pointer of list.
 * @return      array of data, NULL if list is NULL
 */
static ListValue *linked_list_to_array(LinkedListNode *head);

static ListValue peek_list_head(LinkedListNode *head);
static ListValue peek_list_tail(LinkedListNode *head);
/**
 * Get the size of list.
 *
 * @param head  start pointer of list.
 * @return      size of the list.
 */
static unsigned int size_of_linked_list(LinkedListNode *head);

/**
 * Retrieve the node at specific index.
 *
 * @param head  start pointer of list
 * @param index index of list
 * @return      The node at specific index. NULL if unsuccessful or index is wrong.
 */
static LinkedListNode *node_of_linked_list(LinkedListNode *head, unsigned int index);
/**
 * Create a linked list and return the node.
 *
 * @return  LinkedList *    Pointer to the newly created list node.
 */
static LinkedListNode *new_linked_list_node(ListValue data) {
  LinkedListNode *new_list = malloc(sizeof(LinkedListNode));
  new_list->data = data;
  new_list->next = new_list;
  new_list->prev = new_list;
  return new_list;
}

LinkedList *new_linked_list() {
  LinkedList *list = malloc(sizeof(LinkedList));
  if (!list) return NULL;
  list->size = 0;
  list->head = NULL;
  return list;
}

/**
 * Free the linked list.
 */
static void free_linked_list_node(LinkedListNode *head) {
  free_linked_list_data(head, NULL);
}

void free_linked_list(LinkedList *list, void(*free_func)(void *)) {
  if (list) {
    if (list->head) {
      free_linked_list_data(list->head, free_func);
    }
    free(list);
  }
}

static void free_linked_list_data(LinkedListNode *head, void(*free_func)(void *)) {
  if (head == NULL) return;

  LinkedListNode *cur = head;
  do {
    LinkedListNode *next = cur->next;
    if (free_func) {
      free_func(cur->data);
    }
    free(cur);
    cur = next;
  } while (cur != head);
}

/**
 * Append a value to the end of list.
 *
 * @param head  start pointer of list
 * @param data  the data
 * @return      The new entry in the list, or NULL if it was not
 *              possible to allocate the memory for the new entry.
 */
static LinkedListNode *append_linked_list_node(LinkedListNode **head, ListValue data) {
  LinkedListNode *newNode = malloc(sizeof(LinkedListNode));
  if (newNode == NULL) {
    return NULL;
  }
  if (*head == NULL) {
    *head = newNode;
    (*head)->next = *head;
    (*head)->prev = *head;
    (*head)->data = data;
    return *head;
  }
  newNode->data = data;
  LinkedListNode *tail = (*head)->prev;
  tail->next = newNode;
  newNode->prev = tail;
  newNode->next = *head;
  (*head)->prev = newNode;
  return newNode;
}

int append_list(LinkedList *list, ListValue data) {
  if (append_linked_list_node(&list->head, data)) {
    list->size++;
    return 1;

  }
  return 0;
}

int insert_list(LinkedList *list, int index, ListValue data) {
  if (index < 0 || index > list->size) {
    return 0;
  }
  if (index == 0 || index == list->size) {
    return append_list(list, data);
  }
  LinkedListNode *cur = list->head;
  for (int i = 0; i < index - 1; i++) {
    cur = cur->next;
  }
  LinkedListNode *newNode = malloc(sizeof(LinkedListNode));
  if (newNode == NULL) {
    return 0;
  }
  newNode->data = data;
  LinkedListNode *next = cur->next;
  cur->next = newNode;
  newNode->prev = cur;
  newNode->next = next;
  next->prev = newNode;
  list->size++;
  return 1;
}
/**
 * Append a value to the start of list.
 *
 * @param head  start pointer of list
 * @param data  the data
 * @return      The new entry in the list, or NULL if it was not
 *              possible to allocate the memory for the new entry.
 */
static LinkedListNode *prepend_linked_list_node(LinkedListNode **head, ListValue data) {
  LinkedListNode *newNode = malloc(sizeof(LinkedListNode));
  if (newNode == NULL) {
    return NULL;
  }
  if (*head == NULL) {
    *head = newNode;
    (*head)->next = *head;
    (*head)->prev = *head;
    (*head)->data = data;
    return *head;
  }
  newNode->data = data;
  LinkedListNode *tail = (*head)->prev;
  tail->next = newNode;
  newNode->next = *head;
  newNode->prev = tail;
  (*head)->prev = newNode;
  *head = newNode;
  return newNode;
}

int prepend_list(LinkedList *list, ListValue data) {
  if (prepend_linked_list_node(&list->head, data)) {
    list->size++;
    return 1;
  }
  return 0;
}
/**
 * Delete the first node in list and change the head to the new list.
 *
 * @param head  start pointer of list
 * @return      the node that deleted.
 */
static int remove_head_linked_list(LinkedListNode **head) {
  if (*head == NULL) return LINKED_LIST_OPT_ON_NULL_ERROR;
  if ((*head)->next == *head) {
    free(*head);
    *head = NULL;
    return LINKED_LIST_SUCCESS;
  }
  LinkedListNode *new_head = (*head)->next;
  LinkedListNode *tail = (*head)->prev;

  new_head->prev = tail;
  tail->next = new_head;
  free(*head);

  *head = new_head;
  return LINKED_LIST_SUCCESS;
}

int remove_list_first(LinkedList *list) {
  if (LINKED_LIST_SUCCESS == remove_head_linked_list(&list->head)) {
    list->size--;
    return 1;
  }
  return 0;
}

ListValue remove_list(LinkedList *list, int index) {
  if (index < 0 || index > list->size - 1) {
    return NULL;
  }
  LinkedListNode *node = list->head;
  for (int i = 0; i < index; i++) {
    node = node->next;
  }
  ListValue value = node->data;
  if (LINKED_LIST_SUCCESS == remove_node_linked_list(&list->head, node)) {
    list->size--;
    return value;
  }
  return NULL;
}

static ListValue peek_list_head(LinkedListNode *head) {
  if (head == NULL) return NULL;
  return head->data;
}

ListValue peek_list_first(LinkedList *list) {
  return peek_list_head(list->head);
}

static ListValue peek_list_tail(LinkedListNode *head) {
  if (head == NULL) return NULL;
  return head->prev->data;
}

ListValue peek_list_last(LinkedList *list) {
  return peek_list_tail(list->head);
}
/**
 * Delete the last node in list.
 *
 * @param head  start pointer of list
 * @return      the node that deleted.
 */
static int remove_tail_linked_list(LinkedListNode **head) {
  if (*head == NULL) return LINKED_LIST_OPT_ON_NULL_ERROR;
  // only one node
  if ((*head)->next == *head) {
    free(*head);
    *head = NULL;
    return LINKED_LIST_SUCCESS;
  }

  LinkedListNode *tail = (*head)->prev;
  LinkedListNode *new_tail = tail->prev;
  new_tail->next = *head;
  (*head)->prev = new_tail;
  free(tail);
  return LINKED_LIST_SUCCESS;
}
int remove_list_last(LinkedList *list) {
  if (LINKED_LIST_SUCCESS == remove_tail_linked_list(&list->head)) {
    list->size--;
    return 1;
  }
  return 0;
}

/**
 * Remove nodes from list if any data of that node equals to the given data.
 *
 * @param head start pointer of list
 * @param data data to
 * @return
 */
static unsigned int remove_data_linked_list(LinkedListNode **head, ListValue data, LinkedListValueCompare comparator) {
  if (*head == NULL) return 0;
  LinkedListNode *cur = *head;
  LinkedListNode *next;
  unsigned int count = 0;
  char head_shifted;
  do {
    head_shifted = 0;
    next = cur->next;
    if (comparator(data, cur->data) == 0) {
      // If it's the head node, update head pointer
      if (cur == *head) {
        // This is the only node in the list.
        if (cur->next == cur) {
          *head = NULL;
          free(cur);
          count++;
          break;
        } else {
          *head = cur->next;
          head_shifted = 1;
        }
      }
      // update the links of the adjacent nodes.
      cur->prev->next = cur->next;
      cur->next->prev = cur->prev;
      free(cur);
      count++;
    }
    cur = next;
  } while (*head != NULL && (head_shifted || cur != *head));
  return count;
}

int remove_list_equals_to(LinkedList *list, ListValue data, LinkedListValueCompare comparator) {
  int count = (int) remove_data_linked_list(&list->head, data, comparator);
  list->size -= count;
  return count;
}

/**
 * Remove an entry from a list.
 *
 * @param head  start pointer of list
 * @param node  entry to remove
 * @return      Returns zero if successful, non-zero if unsuccessful
 */
static int remove_node_linked_list(LinkedListNode **head, LinkedListNode *node) {
  if (*head == NULL || node == NULL) return LINKED_LIST_OPT_ON_NULL_ERROR;
  // It's the only node
  if (node->next == node) {
    free(node);
    *head = NULL;
    return LINKED_LIST_SUCCESS;
  } else {
    // check if the node is head
    if (*head == node) {
      *head = node->next;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
    return LINKED_LIST_SUCCESS;
  }
}

/**
 * Retrieve the data at specific index.
 *
 * @param head  start pointer of list
 * @param index index of list
 * @return      The value at specific index. NULL if unsuccessful or index is wrong.
 */
static ListValue data_of_linked_list(LinkedListNode *head, unsigned int index) {
  LinkedListNode *node = node_of_linked_list(head, index);
  return node == NULL ? NULL : node->data;
}

ListValue get_list_data(LinkedList *list, int index) {
  return data_of_linked_list(list->head, index);
}

/**
 * Retrieve the data at specific node.
 *
 * @param head  start pointer of list
 * @return      The value at specific index.
 */
ListValue data_of_node_linked_list(LinkedListNode *node) {
  if (node == NULL) return NULL;
  return node->data;
}
/**
 * Retrieve the previous entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The previous entry in the list, or NULL if this
 *                     was the first entry in the list.
 */
LinkedListNode *prev_node_linked_list(LinkedListNode *node) {
  return node == NULL ? NULL : node->prev;
}

/**
 * Retrieve the next entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The next entry in the list, or NULL if this was the
 *                     last entry in the list.
 */
LinkedListNode *next_node_linked_list(LinkedListNode *node) {
  return node == NULL ? NULL : node->next;
}
/**
 * Retrieve the node at specific index.
 *
 * @param head  start pointer of list
 * @param index index of list
 * @return      The node at specific index. NULL if unsuccessful or index is wrong.
 */
static LinkedListNode *node_of_linked_list(LinkedListNode *head, unsigned int index) {
  if (head == NULL || index >= size_of_linked_list(head)) {
    return NULL;
  }
  LinkedListNode *node = head;
  for (int i = 0; i < index; i++) {
    node = node->next;
  }
  return node;
}

LinkedListNode *node_of_list(LinkedList *list, int index) {
  return node_of_linked_list(list->head, index);
}
LinkedListNode *head_of_list(LinkedList *list) {
  return list->head;
}
/**
 * Search the list by given data.
 *
 * @param head  start pointer of list
 * @param data  data to be searched
 * @param compare   compare function
 * @return  list of nodes in which data equals to given one.
 */
static LinkedListNode *search_linked_list(LinkedListNode *head, ListValue data, LinkedListValueCompare compare) {
  if (head == NULL) return NULL;
  LinkedListNode *cur = head;
  LinkedListNode *ret = NULL;
  do {
    if (compare(data, cur->data) == 0) {
      append_linked_list_node(&ret, data);
    }
    cur = cur->next;
  } while (cur != head);
  return ret;
}
/**
 * Set the value at specific node of list.
 *
 * @param head  start pointer of list
 * @param index index of list
 * @param value value to be set
 * @return      zero if successful,no-zero if unsuccessful.
 */
static int set_data_of_linked_list(LinkedListNode *head, unsigned int index, ListValue value) {
  LinkedListNode *node = node_of_linked_list(head, index);
  if (node == NULL) {
    return LINKED_LIST_OPT_ON_NULL_ERROR;
  }
  node->data = value;
  return LINKED_LIST_SUCCESS;
}
int set_list_data(LinkedList *list, int index, ListValue value) {
  if (set_data_of_linked_list(list->head, index, value) == LINKED_LIST_SUCCESS) {
    return 1;
  }
  return 0;
}
/**
 * Get the size of list.
 *
 * @param head  start pointer of list.
 * @return      size of the list.
 */
static unsigned int size_of_linked_list(LinkedListNode *head) {
  if (head == NULL) return 0;
  LinkedListNode *tail = head->prev;
  LinkedListNode *cur = head;
  unsigned int count = 0;
  while (cur != tail) {
    count++;
    cur = cur->next;
  }
  return count + 1;
}
int list_size(LinkedList *list) {
  return list->size;
}

/**
 * Return an array copied from data of list nodes.
 *
 * @param head  start pointer of list.
 * @return      array of data, NULL if list is NULL
 */
static ListValue *linked_list_to_array(LinkedListNode *head) {
  if (head == NULL) {
    return NULL;
  }
  unsigned int size = size_of_linked_list(head);
  ListValue *arr = malloc(sizeof(ListValue) * size);
  if (arr == NULL) return NULL;
  LinkedListNode *cur = head;
  for (int i = 0; i < size; i++) {
    arr[i] = cur->data;
    cur = cur->next;
  }
  return arr;
}

ListValue *value_array(LinkedList *list) {
  return linked_list_to_array(list->head);
}