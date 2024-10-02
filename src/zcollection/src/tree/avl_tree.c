//
// Created by zhang on 2024/1/5.
//
#include "tree/avl_tree.h"
#include <stdlib.h>


#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

struct AVLNode {
  AVLKey key;
  AVLValue value;
  AVLNode *left, *right;
  unsigned int size;
  unsigned int height;

};

struct AVLTree {
  AVLNode *root;
  unsigned int size;
  AVLTreeComparator comparator;
  bool null_key;
  AVLValue null_key_value;
};

static void free_avl_nodes(AVLNode *node);
static AVLNode *new_avl_node(AVLKey key, AVLValue value);
static unsigned int node_height(AVLNode *node);
static int node_balance_factor(AVLNode *node);
static AVLNode *update_balance(AVLNode *node);
static AVLNode *left_rotate(AVLNode *node);
static AVLNode *right_rotate(AVLNode *node);
static AVLNode *avl_tree_remove_internal(AVLNode *node,
                                         AVLKey key,
                                         AVLValue *removed_value,
                                         AVLTreeComparator comparator,
                                         bool *removed_flag);
static AVLNode *min_of_avl(AVLNode *root);
static AVLNode *avl_tree_get_node_internal(AVLNode *root, AVLKey key, AVLTreeComparator comparator);
static unsigned int size_of_node(AVLNode *node);

/**
 * Add the <key, value> into node. Return the new node.
 *
 * @param node          current node
 * @param key           avl key
 * @param value         avl value
 * @param comparator    comparator of key
 * @param added_flag    is the <key, value> pair is added or already exists
 * @return              NEW ROOT NODE after add
 */
static AVLNode *avl_tree_add(AVLNode *node,
                             AVLKey key,
                             AVLValue value,
                             AVLTreeComparator comparator,
                             bool *added_flag);

AVLTree *new_avl_tree(AVLTreeComparator comparator) {
  AVLTree *avl_tree = malloc(sizeof(AVLTree));
  if (avl_tree == NULL) return NULL;
  avl_tree->root = NULL;
  avl_tree->size = 0;
  avl_tree->comparator = comparator;
  avl_tree->null_key = false;
  avl_tree->null_key_value = NULL;
  return avl_tree;
}

void free_avl_tree(AVLTree *avl_tree) {
  if (avl_tree == NULL) return;
  free_avl_nodes(avl_tree->root);
  free(avl_tree);
}

static void free_avl_nodes(AVLNode *node) {
  // recursively free nodes in post order
  if (node == NULL) return;
  free_avl_nodes(node->left);
  free_avl_nodes(node->right);
  free(node);
}

int avl_tree_put(AVLTree *avl_tree, AVLKey key, AVLValue value) {
  if (avl_tree == NULL) return AVL_TREE_OPT_ON_NULL_ERROR;
  if (key == NULL && !avl_tree->null_key) {
    avl_tree->null_key = true;
    avl_tree->null_key_value = value;
    return AVL_TREE_PUT_NEW_ADD;
  }
  if (key == NULL && avl_tree->null_key) {
    avl_tree->null_key_value = value;
    return AVL_TREE_PUT_UNCHANGED;
  }
  bool added = false;
  AVLNode *root = avl_tree_add(avl_tree->root, key, value, avl_tree->comparator, &added);
  avl_tree->root = root;
  avl_tree->size = root == NULL ? 0 : root->size;
  return added ? AVL_TREE_PUT_NEW_ADD : AVL_TREE_PUT_UNCHANGED;
}

/**
 * add key value and return new root
 *
 * @param node
 * @param parent
 * @param key
 * @param value
 * @param comparator
 * @param added_flag
 * @return
 */
static AVLNode *avl_tree_add(AVLNode *node,
                             AVLKey key,
                             AVLValue value,
                             AVLTreeComparator comparator,
                             bool *added_flag) {
  if (node == NULL) {
    // current node is null. need to make new node to store key, value
    *added_flag = true;
    node = new_avl_node(key, value);
    if (node == NULL) {
      // if malloc failed, reset flag and return
      *added_flag = false;
      return NULL;
    }
    return node;
  }

  if (comparator(key, node->key) > 0) {
    // add to right subtree
    bool added = false;
    node->right = avl_tree_add(node->right, key, value, comparator, &added);
    if (added) {
      // increase the size of this node.
      *added_flag = true;
      node->size++;
    }
  } else if (comparator(key, node->key) < 0) {
    // add to left subtree
    bool added = false;
    node->left = avl_tree_add(node->left, key, value, comparator, &added);
    if (added) {
      // increase the size of this node.
      *added_flag = true;
      node->size++;
    }
  } else {
    // update key value
    node->value = value;
    *added_flag = false;
  }
  if (*added_flag == false) {
    // no node added
    return node;
  } else {
    node->height = 1 + max(node_height(node->left), node_height(node->right));
    node = update_balance(node);
    return node;
  }
}

/**
 *
 * @param node
 * @return
 */
static AVLNode *update_balance(AVLNode *node) {
  int balance_factor = node_balance_factor(node);
  if (balance_factor > 1 && node_balance_factor(node->left) >= 0) {
    return right_rotate(node);
  }
  if (balance_factor < -1 && node_balance_factor(node->right) <= 0) {
    return left_rotate(node);
  }
  if (balance_factor > 1 && node_balance_factor(node->left) < 0) {
    node->left = left_rotate(node->left);
    return right_rotate(node);
  }
  if (balance_factor < -1 && node_balance_factor(node->right) > 0) {
    node->right = right_rotate(node->right);
    return left_rotate(node);
  }
  return node;
}

/**
 *
 * Left rotation:                Rotated to:
 *
 *      Y                              X
 *     / \                           /   \
 *   T1   X                        Y      Z
 *       / \                     /  \    / \
 *      T2  Z                   T1  T2  T3  T4
 *         / \
 *       T3   T4
 *
 *
 * @param node
 * @return      return x
 */
static AVLNode *left_rotate(AVLNode *y) {
  AVLNode *x = y->right;
  AVLNode *t2 = x->left;

  x->left = y;
  y->right = t2;

  y->height = 1 + max(node_height(y->left), node_height(y->right));
  x->height = 1 + max(node_height(x->left), node_height(x->right));

  y->size = 1 + size_of_node(y->left) + size_of_node(y->right);
  x->size = 1 + size_of_node(x->left) + size_of_node(x->right);

  return x;
}

static unsigned int size_of_node(AVLNode *node) {
  if (node == NULL) return 0;
  return node->size;
}
/**
 *
 * Right rotation:                Rotated to:
 *
 *         Y                              X
 *        / \                           /  \
 *      X   T4                        Z     Y
 *     / \                          / \    /  \
 *    Z   T3                      T1  T2  T3  T4
 *   / \
 * T1   T2
 *
 *
 * @param node
 * @return      return x
 */
static AVLNode *right_rotate(AVLNode *y) {
  AVLNode *x = y->left;
  AVLNode *t3 = x->right;
  x->right = y;
  y->left = t3;

  y->height = 1 + max(node_height(y->left), node_height(y->right));
  x->height = 1 + max(node_height(x->left), node_height(x->right));

  y->size = 1 + size_of_node(y->left) + size_of_node(y->right);
  x->size = 1 + size_of_node(x->left) + size_of_node(x->right);

  return x;
}

/**
 * get height of node
 *
 * @param node
 * @return      height of node, 0 if node is NULL
 */
static unsigned int node_height(AVLNode *node) {
  return node == NULL ? 0 : node->height;
}

/**
 * balance factor of node
 *
 * @param node
 * @return
 */
static int node_balance_factor(AVLNode *node) {
  if (node == NULL) return 0;
  return node_height(node->left) - node_height(node->right);
}

static AVLNode *new_avl_node(AVLKey key, AVLValue value) {
  AVLNode *node = malloc(sizeof(AVLNode));
  if (node == NULL) return NULL;
  node->key = key;
  node->value = value;
  node->size = 1;
  node->height = 1;
  node->left = NULL, node->right = NULL;
  return node;
}

int avl_tree_remove(AVLTree *avl_tree, AVLKey key) {
  return avl_tree_remove_hold(avl_tree, key, NULL);
}

int avl_tree_remove_hold(AVLTree *avl_tree, AVLKey key, AVLValue *removed) {
  if (avl_tree == NULL) return AVL_TREE_OPT_ON_NULL_ERROR;
  if (key == NULL) {
    if (avl_tree->null_key) {
      *removed = avl_tree->null_key_value;
      avl_tree->null_key = false;
      avl_tree->null_key_value = NULL;
      return AVL_TREE_REMOVED_SUCCESS;
    }
    if (!avl_tree->null_key) {
      removed = NULL;
      return AVL_TREE_REMOVED_UNCHANGED;
    }
  }
  bool flag = false;
  //removed = NULL;
  avl_tree->root = avl_tree_remove_internal(avl_tree->root, key, removed, avl_tree->comparator, &flag);
  avl_tree->size = avl_tree->root == NULL ? 0 : avl_tree->root->size;
  if (flag) {
    return AVL_TREE_REMOVED_SUCCESS;
  } else {
    return AVL_TREE_REMOVED_UNCHANGED;
  }
}

/**
 * Remove by key from node. Return the now root node.
 *
 * @param node
 * @param parent
 * @param key
 * @param comparator
 * @param removed_flag
 * @return
 */
static AVLNode *avl_tree_remove_internal(AVLNode *node,
                                         AVLKey key,
                                         AVLValue *removed_value,
                                         AVLTreeComparator comparator,
                                         bool *removed_flag) {
  if (node == NULL) {
    *removed_flag = false;
    return NULL;
  }
  AVLNode *new_root;
  if (comparator(key, node->key) > 0) {
    // remove from the right subtree
    bool removed = false;
    node->right = avl_tree_remove_internal(node->right, key, removed_value, comparator, &removed);
    new_root = node;
    if (removed) {
      *removed_flag = true;
      new_root->size--;
    }
  } else if (comparator(key, node->key) < 0) {
    // remove from the left subtree
    bool removed = false;
    node->left = avl_tree_remove_internal(node->left, key, removed_value, comparator, &removed);
    new_root = node;
    if (removed) {
      *removed_flag = true;
      new_root->size--;
    }
  } else {
    // current node is to remove
    *removed_flag = true;
    if (node->left == NULL) {
      AVLNode *right = node->right;
      new_root = right;
      if (removed_value != NULL) *removed_value = node->value;
      free(node);
    } else if (node->right == NULL) {
      AVLNode *left = node->left;
      new_root = left;
      if (removed_value != NULL) *removed_value = node->value;
      free(node);
    } else {
      // find the successor node
      AVLNode *successor = min_of_avl(node->right);
      if (removed_value != NULL) *removed_value = successor->value;
      AVLKey successor_key = successor->key;
      AVLKey successor_value = successor->value;
      node->key = successor_key;
      node->value = successor_value;
      bool remove = false;
      // remove the successor
      node->right = avl_tree_remove_internal(node->right, successor_key, NULL, comparator, &remove);
      node->size--;
      new_root = node;
    }
  }
  if (new_root == NULL) return NULL;
  new_root->height = 1 + max(node_height(new_root->left), node_height(new_root->right));
  if (*removed_flag) {
    new_root = update_balance(new_root);
  }
  return new_root;
}

/**
 * Find the node containing the minimum value.
 *
 * @param root  root pointer of the avl.
 * @return      the node of in which the minimum value belong.
 */
static AVLNode *min_of_avl(AVLNode *root) {
  if (root == NULL || root->left == NULL) return root;
  return min_of_avl(root->left);
}

/**
 * remove the given node
 *
 * @param avl_tree
 * @param node
 */
void avl_tree_remove_node(AVLTree *avl_tree, AVLNode *node) {
  avl_tree_remove(avl_tree, node->key);
}

AVLNode *avl_tree_get_node(AVLTree *avl_tree, AVLKey key) {
  if (avl_tree == NULL) NULL;
  return avl_tree_get_node_internal(avl_tree->root, key, avl_tree->comparator);
}

static AVLNode *avl_tree_get_node_internal(AVLNode *root, AVLKey key, AVLTreeComparator comparator) {
  if (root == NULL) {
    return NULL;
  }
  if (comparator(key, root->key) > 0) {
    return avl_tree_get_node_internal(root->right, key, comparator);
  } else if (comparator(key, root->key) < 0) {
    return avl_tree_get_node_internal(root->left, key, comparator);
  } else {
    return root;
  }
}

AVLValue avl_tree_get(AVLTree *avl_tree, AVLKey key) {
  AVLNode *node = avl_tree_get_node(avl_tree, key);
  if (node == NULL) return NULL;
  return node->value;
}

AVLKey avl_tree_node_key(AVLNode *node) {
  return node->key;
}

AVLValue avl_tree_node_value(AVLNode *node) {
  return node->value;
}

AVLKey *avl_tree_keys(AVLTree *avl_tree) {
  // TODO

}

AVLValue *avl_tree_values(AVLTree *avl_tree) {
  //TODO
}

unsigned int avl_tree_size(AVLTree *avl_tree) {
  if (avl_tree == NULL) {
    return 0;
  }
  return avl_tree->size + (avl_tree->null_key ? 1 : 0);
}

void traversal_avl(AVLNode *root, Aord order, AVLHandler handler, void *param) {
  if (root == NULL) return;
  if (order == AVL_PRE) {
    handler(root->value, param);
  }
  traversal_avl(root->left, order, handler, param);
  if (order == AVL_MID) {
    handler(root->value, param);
  }
  traversal_avl(root->right, order, handler, param);
  if (order == AVL_POST) {
    handler(root->value, param);
  }
}

// functions for test
AVLNode *avl_tree_node_child(AVLNode *root, AVLTreeNodeSide side) {
  if (root == NULL) return NULL;
  if (side == AVL_TREE_NODE_LEFT) {
    return root->left;
  } else if (side == AVL_TREE_NODE_RIGHT) {
    return root->right;
  } else {
    return NULL;
  }
}

int avl_tree_subtree_height(AVLNode *root) {
  return root == NULL ? 0 : root->height;
}

AVLNode *avl_tree_root_node(AVLTree *avl_tree) {
  return avl_tree->root;
}