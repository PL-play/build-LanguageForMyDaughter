//
// Created by ran on 2024/1/2.
//
/**
 * Circular Doubly Linked List.
 */
#ifndef ZCOLLECTION_LINKED_LIST_H_
#define ZCOLLECTION_LINKED_LIST_H_
#ifdef __cplusplus
extern "C" {
#endif

/**
 * The value in @ref LinkedListNode.
 */
typedef void *ListValue;

/**
 * Represent an entry in a doubly-linked list.
 */
typedef struct LinkedListNode LinkedListNode;

typedef struct LinkedList LinkedList;

/**
 * Compare two data.
 * @return Returns zero if they are equal. Positive if data1 is bigger than data2, negative otherwise.
 */
typedef int (*LinkedListValueCompare)(ListValue data1, ListValue data2);

LinkedList *new_linked_list();

void free_linked_list(LinkedList *, void(*free_func)(void *));

int append_list(LinkedList *list, ListValue data);

int insert_list(LinkedList *list, int index, ListValue data);

int prepend_list(LinkedList *list, ListValue data);

int remove_list_first(LinkedList *list);

ListValue remove_list(LinkedList* list,int index);

ListValue peek_list_first(LinkedList *list);

ListValue peek_list_last(LinkedList *list);

int remove_list_last(LinkedList *list);

int remove_list_equals_to(LinkedList *list, ListValue data, LinkedListValueCompare);

ListValue get_list_data(LinkedList *list, int index);

/**
 * Retrieve the data at specific node.
 *
 * @param head  start pointer of list
 * @return      The value at specific index.
 */
ListValue data_of_node_linked_list(LinkedListNode *node);

LinkedListNode *head_of_list(LinkedList *list);

LinkedListNode *node_of_list(LinkedList *list,int index);
/**
 * Retrieve the previous entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The previous entry in the list, or NULL if this
 *                     was the first entry in the list.
 */
LinkedListNode *prev_node_linked_list(LinkedListNode *node);

/**
 * Retrieve the next entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The next entry in the list, or NULL if this was the
 *                     last entry in the list.
 */
LinkedListNode *next_node_linked_list(LinkedListNode *node);

int set_list_data(LinkedList *list, int index, ListValue value);

int list_size(LinkedList *list);

ListValue *value_array(LinkedList *list);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_LINKED_LIST_H_
