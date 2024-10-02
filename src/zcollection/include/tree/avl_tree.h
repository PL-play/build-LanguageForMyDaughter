//
// Created by zhang on 2024/1/5.
//

#ifndef ZCOLLECTION_TREE_AVL_TREE_H_
#define ZCOLLECTION_TREE_AVL_TREE_H_
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
#define AVL_TREE_PUT_NEW_ADD 1
#define AVL_TREE_PUT_UNCHANGED 0
#define AVL_TREE_OPT_ON_NULL_ERROR (-1)
#define AVL_TREE_REMOVED_SUCCESS 0
#define AVL_TREE_REMOVED_UNCHANGED 1
typedef enum {
  AVL_PRE, AVL_MID, AVL_POST
} Aord;

typedef void *AVLValue;

typedef void *AVLKey;

typedef struct AVLNode AVLNode;

typedef struct AVLTree AVLTree;

/**
 * Function to handle the data when traversal the data.
 */
typedef void (*AVLHandler)(AVLNode *node, void *);

typedef int (*AVLTreeComparator)(AVLKey key1, AVLKey key2);

AVLTree *new_avl_tree(AVLTreeComparator comparator);

void free_avl_tree(AVLTree *avl_tree);

int avl_tree_put(AVLTree *avl_tree, AVLKey key, AVLValue value);

int avl_tree_remove(AVLTree *avl_tree, AVLKey key);

int avl_tree_remove_hold(AVLTree *avl_tree, AVLKey key, AVLValue *removed);

void avl_tree_remove_node(AVLTree *avl_tree, AVLNode *node);

AVLNode *avl_tree_get_node(AVLTree *avl_tree, AVLKey key);

AVLValue avl_tree_get(AVLTree *avl_tree, AVLKey key);

AVLKey avl_tree_node_key(AVLNode *node);

AVLValue avl_tree_node_value(AVLNode *node);

AVLKey *avl_tree_keys(AVLTree *avl_tree);

AVLValue *avl_tree_values(AVLTree *avl_tree);

unsigned int avl_tree_size(AVLTree *avl_tree);

/**
 * Traversal the avl tree.
 *
 * @param root      root pointer of the avl.
 * @param order     pre, mid or post order.
 * @param handler   the handler to handle data.
 */
void traversal_avl(AVLNode *root, Aord order, AVLHandler handler, void *param);


// functions for tests
typedef enum {
  AVL_TREE_NODE_LEFT = 0,
  AVL_TREE_NODE_RIGHT = 1
} AVLTreeNodeSide;

AVLNode* avl_tree_node_child(AVLNode* root,AVLTreeNodeSide);
int avl_tree_subtree_height(AVLNode* root);
AVLNode* avl_tree_root_node(AVLTree* avl_tree);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_TREE_AVL_TREE_H_
