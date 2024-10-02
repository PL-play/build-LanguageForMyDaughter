//
// Created by ran on 2024/1/16.
//
#include "tree/b_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <queue/dqueue_ll.h>

struct BTreeNode {
  // array of keys
  BTreeKey *key;
  // current number of keys, the capacity of key array is 2t-1
  int n;
  // array of child pointers. the capacity of children array is 2t
  BTreeNode **c;
  // current number of children
  int cn;

};

/**
 * for split node to hold temporary node.
 * only one key and its left and right children to pop up
 */
typedef struct temp_node {
  BTreeKey key;
  BTreeNode *left;
  BTreeNode *right;
  int err;
  int pop;
} temp_node;

struct BTree {
  // degree of btree. maximum m children
  int m;
  // minimum degree, at least t children. t = cell(m/2)
  int t;

  int size;
  BTreeKeyCompareFunc compare_func;

  // root of b-tree
  BTreeNode *root;

  BTreeKeyFreeFunc free_func;

};

static void free_node(BTreeNode *node, BTreeKeyFreeFunc func);
static BTreeNode *search_node(BTreeNode *node, BTreeKey key, int *idx, BTreeKeyCompareFunc compare_func);
static void levelOrderTraversalNode(BTreeNode *node);
static void insert_key_left_side(BTreeNode *node, BTreeKey key, int i, int middle);
static void insert_key_right_side(BTreeNode *node, BTreeKey key, int i, int middle);
static temp_node split_node(BTree *tree, BTreeNode *node, BTreeKey key, int i);
static temp_node insert_key_internal(BTree *tree, BTreeNode *node, BTreeKey key, int *inserted);
static void balance_tree(BTree *tree, BTreeNode *node, BTreeKey key);
static BTreeNode *inorder_predecessor(BTree *tree, BTreeNode *node, int index);
static void btree_node_remove(BTree *tree, BTreeNode *node, BTreeKey key, int *removed);
static void free_node_shallow(BTreeNode *node);

BTree *new_btree(int m, BTreeKeyCompareFunc compare_func) {
  if (m < BTREE_MIN_M) {
    m = BTREE_MIN_M;
  }
  BTree *b_tree = malloc(sizeof(BTree));
  if (b_tree == NULL) return NULL;
  b_tree->m = m;
  b_tree->t = m / 2 + (m % 2 != 0 ? 1 : 0);
  b_tree->compare_func = compare_func;
  b_tree->root = NULL;
  b_tree->free_func = NULL;
  b_tree->size = 0;
  return b_tree;
}

static BTreeNode *allocate_node(int m) {
  BTreeNode *node = malloc(sizeof(BTreeNode));
  if (node == NULL) return NULL;
  node->cn = 0;
  node->n = 0;
  BTreeKey *keys = calloc(m - 1, sizeof(BTreeKey));
  if (keys == NULL) {
    free(node);
    return NULL;
  }
  node->key = keys;
  BTreeNode **c = calloc(m, sizeof(BTreeNode *));
  if (c == NULL) {
    free(keys);
    free(node);
    return NULL;
  }
  node->c = c;
  // disk write(node)
  return node;
}

void free_btree(BTree *b_tree) {
  if (b_tree != NULL) {
    BTreeNode *root = b_tree->root;
    free_node(root, b_tree->free_func);
    free(b_tree);
  }
}
int sizeof_btree(BTree *b_tree) {
  return b_tree->size;
}
static void free_node(BTreeNode *node, BTreeKeyFreeFunc func) {
  if (node == NULL) return;

  if (node->cn != 0) {
    for (int i = 0; i < node->n + 1; i++) {
      free_node(node->c[i], func);
    }
  }
  if (func != NULL) {
    for (int i = 0; i < node->n; ++i) {
      func(node->key[i]);
    }
  }
  free(node->key);
  free(node->c);
  free(node);
}

BTreeNode *btree_search(BTree *b_tree, BTreeKey key) {
  int idx;
  return search_node(b_tree->root, key, &idx, b_tree->compare_func);
}

static BTreeNode *search_node(BTreeNode *node, BTreeKey key, int *idx, BTreeKeyCompareFunc compare_func) {
  if (node == NULL) return NULL;
  for (*idx = 0; *idx < node->n && compare_func(key, node->key[*idx]) >= 0; ++(*idx)) {
    if (compare_func(key, node->key[*idx]) == 0) {
      return node;
    }
  }
  return node->cn == 0 ? NULL : search_node(node->c[*idx], key, idx, compare_func);
}

static void insert_key_left_side(BTreeNode *node, BTreeKey key, int i, int middle) {
  // insert the key into the left side, replace the middle
  for (int j = middle; j > i; j--) {
    node->key[j] = node->key[j - 1];
  }
  node->key[i] = key;
}

static void insert_key_right_side(BTreeNode *node, BTreeKey key, int i, int middle) {
  // insert the key into the right side, replace the middle
  for (int j = middle; j < i && j < node->n - 1; ++j) {
    node->key[j] = node->key[j + 1];
  }
  node->key[i - 1] = key;
}

/**
 * split the node and return the new root node.
 *
 * @param tree      pointer of b-tree
 * @param node      the node to be split
 * @param key       the key to be inserted
 * @param i         the index of in the node that the key to be inserted
 * @return          the new temp root node
 */
static temp_node split_node(BTree *tree, BTreeNode *node, BTreeKey key, int i) {
  // ret is the temp node to return
  temp_node temp_node = {.key=NULL, .left=NULL, .right=NULL, .err=1, .pop=1};
  // child is the node to hold the half of node when it's split
  // the new_node will become the right node, this node become the left node
  BTreeNode *right_node = allocate_node(tree->m);
  if (right_node == NULL) return temp_node;

  int middle = tree->t - 1;

  if (i < middle) {
    // the middle key to be will be the key of new root. but the new key is inserted before middle, so the middle-1
    // is the root key to pop
    BTreeKey root_key = node->key[middle - 1];
    insert_key_left_side(node, key, i, middle - 1);
    temp_node.key = root_key;

  } else if (i > middle) {
    // the new_node will become the right node, this node become the left node
    BTreeKey root_key = node->key[middle];
    insert_key_right_side(node, key, i, middle);
    temp_node.key = root_key;

  } else {
    //the key itself is the middle key to pop up
    temp_node.key = key;
  }
  // populate children
  temp_node.left = node;
  temp_node.right = right_node;

  for (int j = middle; j < node->n; ++j) {
    // split keys to right child
    right_node->key[j - middle] = node->key[j];
    // set null
    node->key[j] = NULL;
  }

  if (node->cn != 0) {
    // node has children
    if (i < middle) {
      // move children to right child
      for (int j = middle; j < tree->m; ++j) {
        right_node->c[j - middle] = node->c[j];
        node->c[j] = NULL;
      }
      // right shift children of left child for children of the key to be inserted
      for (int j = middle; j > i + 1; j--) {
        node->c[j] = node->c[j - 1];
      }
      // leave an empty slot on the left child. i+1
    } else {
      // move children to right child
      for (int j = tree->t; j < tree->m; ++j) {
        right_node->c[j - tree->t] = node->c[j];
        node->c[j] = NULL;
      }

      // right shift children of left child for children of the key to be inserted
      for (int j = middle + (tree->m % 2 == 0 ? 1 : 0); j > i - middle; j--) {
        right_node->c[j] = right_node->c[j - 1];
      }
      // leave an empty slot on the right child. i-t+1
    }
    node->cn = tree->t;
    right_node->cn = tree->m - tree->t + 1;
  }
  // update counts
  node->n = tree->t - 1;
  right_node->n = tree->m - tree->t;

  temp_node.err = 0;
  return temp_node;
}

static temp_node insert_key_internal(BTree *tree, BTreeNode *node, BTreeKey key, int *inserted) {
  *inserted = 0;
  temp_node t = {.key=NULL, .left=NULL, .right=NULL, .err=0, .pop=0};
  int i = 0;
  while (i < node->n && tree->compare_func(key, node->key[i]) > 0) {
    i++;
  }
  if (i < node->n && tree->compare_func(key, node->key[i]) == 0) {
    // key already in the node
    // todo update value
    return t;
  }
  // not found in this node
  if (node->cn == 0) {
    // if this node is leaf, insert key into this node
    // don't need to split this node
    if (node->n < tree->m - 1) {
      for (int j = node->n; j > i; j--) {
        // right move keys
        node->key[j] = node->key[j - 1];
      }
      // insert the key
      node->key[i] = key;
      node->n++;
      t.pop = 0;
      *inserted = 1;
      return t;
    } else {
      // n == m-1, will overflow
      // need to split this node
      *inserted = 1;
      return split_node(tree, node, key, i);
    }
  } else {
    // if node is not leaf, insert key to its child
    temp_node ret = insert_key_internal(tree, node->c[i], key, inserted);
    if (ret.pop == 1 && ret.err == 0) {
      // new root popped up, merge in this node
      if (node->n == tree->m - 1) {
        // need split
        temp_node tep = split_node(tree, node, ret.key, i);
        if (tep.err == 1) {
          t.err = 1;
          return t;
        }
        // only right child is the new created node. left child is the reference of the current node.
        if (i < tree->t - 1) {
          tep.left->c[i + 1] = ret.right;
        } else {
          tep.right->c[i - tree->t + 1] = ret.right;
        }
        return tep;
      } else {
        // shift keys in this node to insert popped node
        for (int j = node->n; j > i; j--) {
          node->key[j] = node->key[j - 1];
        }
        // insert the popped key
        node->key[i] = ret.key;
        // shift children
        for (int j = node->n + 1; j > i + 1; j--) {
          node->c[j] = node->c[j - 1];
        }
        //node->c[i] = ret.left;
        node->c[i + 1] = ret.right;

        node->n++;
        node->cn++;
        // handled, no need to pop up
        ret.pop = 0;
        return ret;
      }
    } else {
      // no new root popped up or failed
      return ret;
    }
  }
}

int btree_insert_key(BTree *tree, BTreeKey key) {
  if (tree->root == NULL) {
    // 1. if the root node is null, make a root node and insert the key in root node
    BTreeNode *node = allocate_node(tree->m);
    if (node == NULL) return 0;
    node->key[0] = key;
    node->n++;
    node->cn = 0;
    tree->root = node;
    tree->size = 1;
    return 1;
  }
  int inserted = 0;
  temp_node t = insert_key_internal(tree, tree->root, key, &inserted);
  if (inserted == 1) {
    tree->size++;
  }
  if (t.err == 0 && t.pop == 1) {
    BTreeNode *new_node = allocate_node(tree->m);
    new_node->key[0] = t.key;
    new_node->cn = 0;
    new_node->c[0] = t.left;
    new_node->c[1] = t.right;
    new_node->n = 1;
    new_node->cn = 2;
    tree->root = new_node;
  }
  return inserted;
}

void register_btree_keyfree(BTree *b_tree, BTreeKeyFreeFunc free_func) {
  b_tree->free_func = free_func;
}

int btree_remove(BTree *tree, BTreeKey key) {
  if (tree->root == NULL) {
    return 0;
  }
  int removed = 0;
  btree_node_remove(tree, tree->root, key, &removed);
  if (removed == 1) {
    tree->size--;
  }
  BTreeNode *root = tree->root;
  if (removed && root->n == 0) {
    // no keys in root, tree will shrink
    BTreeNode *c = root->c[0];
    tree->root = c;
    free_node_shallow(root);
  }
  return removed;
}

static void btree_node_remove(BTree *tree, BTreeNode *node, BTreeKey key, int *removed) {
  // TODO return value
  // 1. find the key to remove
  int i = 0;
  while (i < node->n && tree->compare_func(node->key[i], key) < 0) {
    i++;
  }
  if (i < node->n && tree->compare_func(key, node->key[i]) == 0) {
    // found target key to remove
    *removed = 1;
    if (node->cn == 0) {
      // no children, left shift keys
      BTreeKey k = node->key[i];
      for (int j = i; j < node->n - 1; ++j) {
        node->key[j] = node->key[j + 1];
      }
      node->key[node->n - 1] = NULL;
      node->n--;
      if (tree->free_func != NULL) {
        tree->free_func(k);
      }
      if (node->n < tree->t - 1) {
        // rebalance tree from target leaf to root
        balance_tree(tree, tree->root, key);
      }
    } else {
      // internal node, remove its predecessor
      BTreeNode *pred = inorder_predecessor(tree, node, i);
      // replace the target with the predecessor
      BTreeKey k = node->key[i];
      BTreeKey rk = pred->key[0];
      node->key[i] = pred->key[pred->n - 1];
      pred->key[pred->n - 1] = NULL;
      pred->n--;
      if (tree->free_func != NULL) {
        tree->free_func(k);
      }
      if (pred->n < tree->t - 1) {
        balance_tree(tree, tree->root, rk);
      }
    }
  } else {
    // keep find in children
    if (node->cn != 0) {
      btree_node_remove(tree, node->c[i], key, removed);
    } else {
      // key not found
      *removed = 0;
    }
  }
}

static void balance_tree(BTree *tree, BTreeNode *node, BTreeKey key) {
  // rebalance take place in parent
  if (node->cn == 0) {
    return;
  }
  // go deep into where the re-balance starts
  int i = 0;
  while (i < node->n && tree->compare_func(key, node->key[i]) > 0) {
    i++;
  }
  // starts from child
  balance_tree(tree, node->c[i], key);

  if (node->c[i]->n < tree->t - 1) {
    // need re-balance
    if (i < node->cn - 1 && node->c[i + 1]->n > tree->t - 1) {
      // case 1 deficient node has right sibling and right sibling has > t-1 keys
      // spare key from right sibling. rotate left
      /**
       *        x | S | x                                   x | y | x
       *         /    \               rotate left            /    \
       *    x | x     y | z | x | x     ----->      x | x | S     z | x | x
       *             /                                     /
       *            c                                     c
       *
       */
      BTreeNode *deficient = node->c[i];
      BTreeKey s = node->key[i];
      BTreeNode *right = node->c[i + 1];
      deficient->key[deficient->n] = s;
      deficient->n++;

      node->key[i] = right->key[0];

      // left shift right keys
      for (int j = 0; j < right->n - 1; ++j) {
        right->key[j] = right->key[j + 1];
      }
      right->key[right->n - 1] = NULL;
      right->n--;

      if (deficient->cn != 0) {
        // move left most child of right to end of deficient node
        deficient->c[deficient->cn] = right->c[0];
        deficient->cn++;

        // left shift right children
        for (int j = 0; j < right->cn - 1; ++j) {
          right->c[j] = right->c[j + 1];
        }
        right->c[right->cn - 1] = NULL;
        right->cn--;
      }
    } else if (i > 0 && node->c[i - 1]->n > tree->t - 1) {
      // case 2 deficient node has left sibling and left sibling has > t-1 keys
      // spare key from left sibling. rotate right
      /**
       *            x | S | x                                x | y | x
       *             /    \          rotate right             /    \
       * x | x | b | y     x | x       ----->        x | x | b      S | x | x
       *              \                                            /
       *               c                                          c
       *
       */
      BTreeNode *deficient = node->c[i];
      BTreeNode *left = node->c[i - 1];
      // right shift deficient node
      for (int j = deficient->n; j > 0; j--) {
        deficient->key[j] = deficient->key[j - 1];
      }
      deficient->key[0] = node->key[i - 1];
      deficient->n++;

      node->key[i - 1] = left->key[left->n - 1];
      left->n--;

      if (deficient->cn != 0) {
        // right shift children of deficient node
        for (int j = deficient->cn; j > 0; j--) {
          deficient->c[j] = deficient->c[j - 1];
        }
        deficient->cn++;

        // move the rightmost child of left to deficit node's child[0]
        deficient->c[0] = left->c[left->cn - 1];
        left->c[left->cn - 1] = NULL;
        left->cn--;
      }
    } else {
      // case 3: neither of siblings is sufficient
      // merge with siblings and parent separator
      /**
      *            x | S | x                          x | x
      *             /    \        merge                |
      *         x | x     x      ------>        x | x | S | x
      *        /  \  \   / \                   /  /   /  /  \
      *      c0  c1 c2  c3  c4               c0  c1  c2 c3  c4
      */
      // if has left sibling
      int leftIdx = i > 0 ? i - 1 : i;
      BTreeNode *left = node->c[leftIdx];
      BTreeNode *right = node->c[leftIdx + 1];
      // left children right most index
      // move s to left node's end
      left->key[left->n] = node->key[leftIdx];
      left->n++;

      // move right to left
      for (int j = 0; j < right->n; ++j) {
        left->key[left->n] = right->key[j];
        left->n++;
      }
      // copy children of right to left
      for (int j = 0; j < right->cn; ++j) {
        left->c[left->cn] = right->c[j];
        left->cn++;
      }
      // free right node
      free_node_shallow(right);

      // left shift node
      for (int j = leftIdx; j < node->n - 1; ++j) {
        node->key[j] = node->key[j + 1];
      }
      node->n--;
      // left shift node's children
      for (int j = leftIdx + 1; j < node->cn - 1; ++j) {
        node->c[j] = node->c[j + 1];
      }
      node->cn--;
    }
  }
}

static BTreeNode *inorder_predecessor(BTree *tree, BTreeNode *node, int index) {
  BTreeNode *p = node->c[index];
  while (p->cn != 0) {
    p = p->c[p->n];
  }
  return p;
}

void print_btree(BTree *tree) {
  BTreeNode *root = tree->root;
  levelOrderTraversalNode(root);
}

static void levelOrderTraversalNode(BTreeNode *node) {
  Dqueue *q = new_dqueue();
  dqueue_push_tail(q, node);
  dqueue_push_tail(q, NULL); // Marker for end of level

  while (!dqueue_is_empty(q)) {
    BTreeNode *current = dqueue_pop_head(q);

    if (current == NULL) {
      // End of a level
      printf("\n");
      if (!dqueue_is_empty(q)) {
        dqueue_push_tail(q, NULL); // Add marker for next level
      }
    } else {
      // Print the keys in the current node with brackets
      printf("[");
      for (int i = 0; i < current->n; ++i) {
        printf("%d", *(int *) (current->key[i]));
        if (i < current->n - 1) {
          printf(" "); // Space between keys
        }
      }
      printf("] "); // Close bracket and space after the node
      fflush(stdout);
      if (current->cn != 0) {
        for (int i = 0; i <= current->n; i++) {
          if (current->c[i] != NULL) {
            dqueue_push_tail(q, current->c[i]);
          }
        }
      }
    }
  }

  free_dqueue(q);
}

static void free_node_shallow(BTreeNode *node) {
  if (node) {
    free(node->key);
    free(node->c);
    free(node);
  }
}

int isLeaf(BTreeNode *node) {
  return (node->cn == 0);
}
int validateNode(BTreeNode *node,
                 int minKeys,
                 int maxKeys,
                 int minChildren,
                 int maxChildren,
                 int level,
                 int *leafLevel,
                 int isroot,
                 BTreeKeyCompareFunc compare_func) {
  // Check number of keys
  if (!isroot) {
    if (node->n < minKeys || node->n > maxKeys) {
      return 0;
    }

    // Check number of children
    if (!isLeaf(node) && (node->cn < minChildren || node->cn > maxChildren)) {
      return 0;
    }
  }


  // Check if keys are in sorted order
  for (int i = 1; i < node->n; i++) {
    if (compare_func(node->key[i - 1], node->key[i]) >= 0) {
      return 0;
    }
  }

  // Check children
  if (!isLeaf(node)) {
    for (int i = 0; i < node->cn; i++) {
      if (!validateNode(node->c[i],
                        minKeys,
                        maxKeys,
                        minChildren,
                        maxChildren,
                        level + 1,
                        leafLevel,
                        0,
                        compare_func)) {
        return 0;
      }
    }
  } else {
    // Check if all leaves are at the same level
    if (*leafLevel == 0) {
      *leafLevel = level;
    } else if (level != *leafLevel) {
      return 0;
    }
  }

  return 1;
}

int btree_validate(BTree *tree) {
  if (!tree || !tree->root) {
    return 1; // An empty tree is a valid B-tree
  }

  int minKeys = tree->t - 1;
  int maxKeys = tree->m - 1;
  int minChildren = tree->t;
  int maxChildren = tree->m;
  int leafLevel = 0; // Start with leafLevel as 0

  // Special case for root
  if (isLeaf(tree->root)) {
    if (tree->root->n < 1 || tree->root->n > maxKeys) {
      return 0;
    }
  } else {
    if (tree->root->cn < 2 || tree->root->cn > maxChildren) {
      return 0;
    }
  }

  // Start validation from level 1, as root is at level 1
  return validateNode(tree->root, minKeys, maxKeys, minChildren, maxChildren, 1, &leafLevel, 1, tree->compare_func);
}