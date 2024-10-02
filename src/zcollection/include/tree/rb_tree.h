//
// Created by ran on 2024/1/6.
//
/**
 * The red-black tree is a balanced binary tree. Each node has a key
 * and a value associated with it. The order is based on the order of
 * key.
 *
 * A red-black tree is "black balanced", which means any path from root
 * node to a leaf node has the same amount of black nodes.
 *
 * Several properties make a tree a read-black tree:
 *  1. The root node and external nodes are black;
 *  2. There are no consecutive 2 red nodes on the path from the root to
 *     all external nodes;
 *  3. All paths from roots to external nodes have the same number of black
 *     nodes.
 *
 */
#ifndef ZCOLLECTION_TREE_RB_TREE_H_
#define ZCOLLECTION_TREE_RB_TREE_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef void *RBTreeKey;

typedef void *RBTreeValue;

typedef struct RBTreeNode RBTreeNode;

typedef struct RBTree RBTree;

typedef void(*RBTreeKeyFreeFunc)(RBTreeKey key);

typedef void(*RBTreeValueFreeFunc)(RBTreeValue value);

typedef int (*RBTreeComparator)(RBTreeKey key1, RBTreeKey key2);

typedef struct RBNodeEntry {
  RBTreeKey key;
  RBTreeValue value;
} RBNodeEntry;

typedef struct RBTreeIterator RBTreeIterator;

/**
 * Create new node with key and value.
 *
 * @param key
 * @param value
 * @return      pointer of RBTreeNode. NULL if unsuccessful.
 */
RBTreeNode *new_rb_node(RBTreeKey key, RBTreeValue value);

/**
 * Register functions used to free the key and value when an entry is
 * removed from a red black tree.
 *
 * @param rb_tree           The rb tree.
 * @param key_free_func     Function used to free keys.
 * @param value_free_func   Function used to free values.
 */

void register_rb_tree_free_functions(RBTree *rb_tree,
                                     RBTreeKeyFreeFunc key_free_func,
                                     RBTreeValueFreeFunc value_free_func);

/**
 * Create new rb-tree with comparator.
 *
 * @param comparator    comparator of RBTreeKey;
 * @return              pointer of RBTree. NULL if unsuccessful.
 */
RBTree *new_rb_tree(RBTreeComparator comparator);

/**
 * Free rb tree
 *
 * @param rb_tree
 */
void free_rb_tree(RBTree *rb_tree);

/**
 * Insert a new key value pair to rb tree. If the key is already exist,update the node with the value.
 *
 * @param rb_tree   pointer of rb tree
 * @param key       key
 * @param value     value
 * @return          the newly create tree node. NULL if impossible to allocate memory
 */
RBTreeNode *put_rb_tree(RBTree *rb_tree, RBTreeKey key, RBTreeValue value);

/**
 * Test the given key is in the rt tree.
 *
 * @param rb_tree   pointer of rb tree
 * @param key       the key
 * @return          true or false
 */
int contains_in_rb_tree(RBTree *rb_tree, RBTreeKey key);

void get_hold_value_rb_tree(RBTree *rb_tree, RBTreeKey key, int *flag, RBTreeValue *value);

/**
 * Get node with given key from rb tree.
 *
 * @param tree  pointer of rb tree
 * @param key   the key
 * @return      the node of which key is equal to give key. NULL if not found.
 */
RBTreeNode *get_node_rb_tree(RBTree *tree, RBTreeKey key);

/**
 * Get value of the key.
 *
 * @param tree  pointer of rb tree
 * @param key   the key
 * @return      the value, NULL if not found.
 */
RBTreeValue get_value_rb_tree(RBTree *tree, RBTreeKey key);

/**
 * Remove a node with given key
 *
 * @param tree  pointer of rb tree
 * @param key
 * @return
 */
RBTreeValue remove_rb_tree(RBTree *tree, RBTreeKey key);

/**
 * Remove a node with given key
 *
 * @param tree  pointer of rb tree
 * @param key
 * @return
 */
RBTreeValue remove_with_flag_rb_tree(RBTree *tree, RBTreeKey key, int *flag);

/**
 * Remove a node
 *
 * @param tree  pointer of rb tree
 * @param node  node to remove
 * @return
 */
RBTreeValue remove_node_rb_tree(RBTree *tree, RBTreeNode *node);

/**
 * Get the size of rb tree.
 *
 * @param rb_tree
 * @return
 */
unsigned int size_of_rbt(RBTree *rb_tree);

RBTreeKey key_of_rb_node(RBTreeNode *node);

unsigned int height_of_rb_node(RBTreeNode *node);

unsigned int height_of_rbt(RBTree *rb_tree);

RBTreeValue value_of_rb_node(RBTreeNode *node);

void set_value_of_rb_node(RBTreeNode *node,RBTreeValue value);

RBTreeIterator *rb_tree_iterator(RBTree *rb_tree);

void free_rb_tree_iterator(RBTreeIterator *iterator);

int has_next_rbt_entry(RBTreeIterator *iterator);
// TODO should it return the  RBTreeNode or RBNodeEntry?
RBNodeEntry next_rbt_entry(RBTreeIterator *iterator);

RBTreeNode *next_rbt_node(RBTreeIterator *iterator);

/**
 * Only for test. the diff of height is at most 2 times.
 *
 * should be (1/2, 2)
 *
 * @param node
 * @return
 */
double rb_tree_diff_lr_height(RBTreeNode *node);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_TREE_RB_TREE_H_
