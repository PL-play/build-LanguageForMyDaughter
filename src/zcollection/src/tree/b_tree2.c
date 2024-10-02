//
// Created by ran on 2024/1/20.
//
#include "tree/b_tree2.h"
#include <queue/dqueue_ll.h>
#include <stdlib.h>
#include <stdio.h>

struct btree_node {
  bt_key *k;
  int size;
  int is_leaf;
  btree_node **c;
};

struct btree {
  btree_node *root;
  int t;
  int node_count;
  int size;
};

static bt_key bt_search_internal(btree_node *node, bt_key key);
static btree_node *new_node(btree *bt);
// split pa.c[idx] node, the pa node is NON-FULL
static void split_child(btree *bt, btree_node *pa, int idx);
static int insert_non_full(btree *bt, btree_node *node, bt_key key);
static void levelOrderTraversalNode(btree_node *node);
static void free_node(btree_node *node);
static int delete_key(btree *bt, btree_node *node, bt_key key);
static btree_node *predecessor_node(btree_node *node);
static btree_node *successor_node(btree_node *node);
static void merge_nodes(btree *bt, btree_node *node, int i, btree_node *left, btree_node *right);
static void free_node_shallow(btree_node *node);
static void rotate_right(btree *bt, btree_node *node, int i, btree_node *ci, btree_node *left);
static void rotate_left(btree *bt, btree_node *node, int i, btree_node *ci, btree_node *right);
static void merge_siblings(btree *bt, btree_node *node, btree_node *left, btree_node *right, int leftIdx);

btree *bt_create(int order) {
  btree *bt = malloc(sizeof(btree));
  if (!bt) return NULL;
  bt->t = order;
  bt->node_count = 0;
  bt->size = 0;
  bt->root = NULL;
  return bt;
}

bt_key bt_search(btree *bt, bt_key key) {
  if (bt->root == NULL) return -1;
  return bt_search_internal(bt->root, key);
}
int bt_size(btree *bt) {
  return bt->size;
}
void bt_free(btree *bt) {
  if (bt != NULL) {
    btree_node *root = bt->root;
    free_node(root);
    free(bt);
  }
}
int bt_insert(btree *bt, bt_key key) {
  if (bt->root == NULL) {
    bt->root = new_node(bt);
    bt->root->size++;
    bt->node_count = 1;
    bt->root->k[0] = key;
    bt->size = 1;
    bt->node_count = 1;
    return 1;
  }
  btree_node *n = bt->root;
  int ret;
  if (n->size == bt->t * 2 - 1) {
    btree_node *s = new_node(bt);
    s->c[0] = n;
    s->is_leaf = 0;
    bt->node_count++;
    bt->root = s;
    split_child(bt, s, 0);
    ret = insert_non_full(bt, s, key);
  } else {
    ret = insert_non_full(bt, n, key);
  }
  if (ret == 1) bt->size++;
  return ret;
}

static bt_key bt_search_internal(btree_node *node, bt_key key) {
  int i = 0;
  while (i < node->size && key > node->k[i]) {
    i++;
  }
  if (i < node->size && key == node->k[i]) {
    return node->k[i];
  } else if (node->is_leaf == 1) {
    return -1;
  } else {
    return bt_search_internal(node->c[i], key);
  }

}
static btree_node *new_node(btree *bt) {
  btree_node *btn = malloc(sizeof(btree_node));
  if (!btn) return NULL;
  btn->size = 0;
  btn->k = calloc(bt->t * 2 - 1, sizeof(bt_key));
  if (!btn->k) {
    free(btn);
    return NULL;
  }

  btn->c = calloc(bt->t * 2, sizeof(btree_node *));
  if (!btn->c) {
    free(btn->k);
    free(btn);
    return NULL;
  }
  btn->is_leaf = 1;
  return btn;
}
static void split_child(btree *bt, btree_node *pa, int idx) {
  btree_node *right = new_node(bt);
  btree_node *left = pa->c[idx];
  right->is_leaf = left->is_leaf;

  int mid = bt->t - 1;
  bt_key mid_key = left->k[mid];
  // move keys
  for (int i = mid + 1; i < bt->t * 2 - 1; ++i) {
    right->k[i - mid - 1] = left->k[i];
  }
  if (!left->is_leaf) {
    // move children
    for (int i = mid + 1; i < bt->t * 2; ++i) {
      right->c[i - mid - 1] = left->c[i];
    }
  }
  // right shift parent keys
  for (int i = pa->size; i > idx; i--) {
    pa->k[i] = pa->k[i - 1];
  }
  // insert mid key
  pa->k[idx] = mid_key;
  // right shift parent children
  for (int i = bt->t * 2; i > idx; i--) {
    pa->c[i] = pa->c[i - 1];
  }
  // insert new child
  pa->c[idx + 1] = right;

  // update counters
  left->size = bt->t - 1;
  right->size = bt->t - 1;
  pa->size++;
  bt->node_count++;
}
static int insert_non_full(btree *bt, btree_node *node, bt_key key) {
  if (node->is_leaf) {
    int i = 0;
    while (i < node->size && key > node->k[i]) {
      i++;
    }
    if (i < node->size && node->k[i] == key) {
      // todo update value
      return -1;
    }

    for (int j = node->size; j > i; j--) {
      node->k[j] = node->k[j - 1];
    }
    node->k[i] = key;
    node->size++;
    return 1;

  } else {
    int i = 0;
    while (i < node->size && key > node->k[i]) {
      i++;
    }
    if (i < node->size && node->k[i] == key) {
      // todo update value
      return -1;
    }
    if (node->c[i]->size == bt->t * 2 - 1) {
      split_child(bt, node, i);
      if (key == node->k[i]) {
        // todo update value
        return -1;
      }
      if (key > node->k[i]) {
        i++;
      }
    }
    return insert_non_full(bt, node->c[i], key);
  }
}

int bt_delete(btree *bt, bt_key key) {
  btree_node *root = bt->root;
  if (!root) return 0;
  int ret = delete_key(bt, root, key);
  if (root->size == 0) {
    if (bt->size != 0) {
      bt->root = root->c[0];
    } else {
      bt->root = NULL;
    }
    free_node_shallow(root);
  }
  return ret;
}

void print_bt(btree *tree) {
  btree_node *root = tree->root;
  if (root == NULL) return;
  levelOrderTraversalNode(root);
}

static void levelOrderTraversalNode(btree_node *node) {
  Dqueue *q = new_dqueue();
  dqueue_push_tail(q, node);
  dqueue_push_tail(q, NULL); // Marker for end of level

  while (!dqueue_is_empty(q)) {
    btree_node *current = dqueue_pop_head(q);

    if (current == NULL) {
      // End of a level
      printf("\n");
      if (!dqueue_is_empty(q)) {
        dqueue_push_tail(q, NULL); // Add marker for next level
      }
    } else {
      // Print the keys in the current node with brackets
      printf("[");
      for (int i = 0; i < current->size; ++i) {
        printf("%d", (current->k[i]));
        if (i < current->size - 1) {
          printf(" "); // Space between keys
        }
      }
      printf("] "); // Close bracket and space after the node
      if (!current->is_leaf) {
        for (int i = 0; i <= current->size; i++) {
          if (current->c[i] != NULL) {
            dqueue_push_tail(q, current->c[i]);
          }
        }
      }
    }
  }

  free_dqueue(q);
}
static int isLeaf(btree_node *node) {
  return node->is_leaf;
}
static int validateNode(btree_node *node,
                        int minKeys,
                        int maxKeys,
                        int minChildren,
                        int maxChildren,
                        int level,
                        int *leafLevel,
                        int isroot) {
  if (node->size == 0) {
    return 0;
  }
  // Check number of keys
  if (!isroot) {
    if (node->size < minKeys || node->size > maxKeys) {
      return 0;
    }
  }


  // Check if keys are in sorted order
  for (int i = 1; i < node->size; i++) {
    if (node->k[i - 1] >= node->k[i]) {
      return 0;
    }
  }

  // Check children
  if (!isLeaf(node)) {
    for (int i = 0; i <= node->size; i++) {
      if (!validateNode(node->c[i],
                        minKeys,
                        maxKeys,
                        minChildren,
                        maxChildren,
                        level + 1,
                        leafLevel,
                        0)) {
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

int bt_validate(btree *tree) {
  if (!tree || !tree->root) {
    return 1; // An empty tree is a valid B-tree
  }

  int minKeys = tree->t - 1;
  int maxKeys = tree->t * 2 - 1;
  int minChildren = tree->t;
  int maxChildren = tree->t * 2;
  int leafLevel = 0; // Start with leafLevel as 0

  // Special case for root
  if (isLeaf(tree->root)) {
    if (tree->root->size < 1 || tree->root->size > maxKeys) {
      return 0;
    }
  }

  // Start validation from level 1, as root is at level 1
  return validateNode(tree->root, minKeys, maxKeys, minChildren, maxChildren, 1, &leafLevel, 1);
}
static void free_node(btree_node *node) {
  if (node == NULL) return;

  if (!node->is_leaf) {
    for (int i = 0; i < node->size + 1; i++) {
      free_node(node->c[i]);
    }
  }

  free(node->k);
  free(node->c);
  free(node);
}

static int delete_key(btree *bt, btree_node *node, bt_key key) {
  int i = 0;
  while (i < node->size && key > node->k[i]) {
    i++;
  }
  // find the key in this node
  if (i < node->size && key == node->k[i]) {
    // 1. this node is a leaf node, delete the key
    if (node->is_leaf) {
      for (int j = i; j < node->size - 1; ++j) {
        node->k[j] = node->k[j + 1];
      }
      node->k[node->size - 1] = -1;
      node->size--;
      bt->size--;
      return 1;
    } else {
      // 2. this node is internal node
      btree_node *right;
      btree_node *left;
      left = node->c[i];
      if (left->size >= bt->t) {
        // 2.a if left child node has at least t keys, remove the predecessor of the left node
        // and take it to replace the key to remove in current node.
        btree_node *pred = predecessor_node(left);
        bt_key max = pred->k[pred->size - 1];
        delete_key(bt, left, max);
        node->k[i] = max;
        return 1;
      } else {
        // 2.b if right child node has at least t keys, remove the successor of the right node
        //  and take it to replace the key to remove in current node.
        right = node->c[i + 1];
        if (right->size >= bt->t) {
          btree_node *suc = successor_node(right);
          bt_key min = suc->k[0];
          delete_key(bt, right, min);
          node->k[i] = min;
          return 1;
        } else {
          // 2.c both left and right has only t-1 keys. merge key and right child to left child, so current node
          // will lose key and child[i+1] and left child will have 2t-1 keys. then delete key from left child.
          merge_nodes(bt, node, i, left, right);
          return delete_key(bt, left, key);
        }
      }
    }
  } else if (node->is_leaf) {
    // key not found
    return -1;
  } else {
    // 3. try to delete key in child
    btree_node *ci = node->c[i];
    btree_node *right;
    btree_node *left;
    int has_left = i > 0;
    if (ci->size == bt->t - 1) {
      // 3.a If x.ci has only t-1 keys and one of its siblings has at least t keys.
      // Drop down a key in x into x.ci and level up a key from one of x.ci's siblings
      // to x, so x.ci get one extra key.

      if (has_left) {
        /**
         *                      - > [i]
         *                     |
         *            x | S | x                                x | y | x
         *             /    \          rotate right             /    \
         * x | x | b | y     x | x       ----->        x | x | b      S | x | x
         *              \                                            /
         *               c                                          c
         *
         */
        left = node->c[i - 1];
        if (left->size >= bt->t) {
          rotate_right(bt, node, i, ci, left);
          return delete_key(bt, ci, key);
        }
      } else {
        /**
          *             - > [i]
          *            |
          *        x | S | x                                   x | y | x
          *         /    \               rotate left            /    \
          *    x | x     y | z | x | x     ----->      x | x | S     z | x | x
          *             /                                       \
          *            c                                         c
          *
          */
        right = node->c[i + 1];
        if (right->size >= bt->t) {
          rotate_left(bt, node, i, ci, right);
          return delete_key(bt, ci, key);
        }
      }
      // 3.b merge with siblings
      /**
       *            x | S | x                          x | x
       *             /    \        merge                |
       *         x | x     x      ------>        x | x | S | x
       *        /  \  \   / \                   /  /   /  /  \
       *      c0  c1 c2  c3  c4               c0  c1  c2 c3  c4
       */
      if (has_left) {
        right = ci;
      } else {
        left = ci;
      }
      int leftIdx = has_left ? i - 1 : i;
      merge_siblings(bt, node, left, right, leftIdx);
      return delete_key(bt, left, key);
    } else {
      return delete_key(bt, ci, key);
    }
  }
}

static void merge_siblings(btree *bt, btree_node *node, btree_node *left, btree_node *right, int leftIdx) {
  int left_c = left->size + 1;
  int right_c = right->size + 1;
  int node_c = node->size + 1;
  // move down s
  bt_key s = node->k[leftIdx];
  left->k[left->size] = s;
  left->size++;

  // move right to left
  for (int j = 0; j < right->size; ++j) {
    left->k[left->size] = right->k[j];
    left->size++;
  }

  // copy children
  if (!left->is_leaf) {
    for (int j = 0; j < right_c; ++j) {
      left->c[left_c] = right->c[j];
      left_c++;
    }
  }
  free_node_shallow(right);

  // left shift node
  for (int j = leftIdx; j < node->size - 1; ++j) {
    node->k[j] = node->k[j + 1];
  }
  node->size--;
  // left shift node's children
  for (int j = leftIdx + 1; j < node_c - 1; ++j) {
    node->c[j] = node->c[j + 1];
  }
  bt->node_count--;
}

static void rotate_right(btree *bt, btree_node *node, int i, btree_node *ci, btree_node *left) {
  bt_key s = node->k[i - 1];
  // right shift x.ci
  for (int j = ci->size; j > 0; j--) {
    ci->k[j] = ci->k[j - 1];
  }
  ci->k[0] = s;
  ci->size++;
  node->k[i - 1] = left->k[left->size - 1];
  left->k[left->size - 1] = -1;
  left->size--;

  if (!ci->is_leaf) {
    // right shift children
    for (int j = ci->size; j > 0; j--) {
      ci->c[j] = ci->c[j - 1];
    }
    // move left children
    ci->c[0] = left->c[left->size + 1];
    left->c[left->size + 1] = NULL;
  }
}

static void rotate_left(btree *bt, btree_node *node, int i, btree_node *ci, btree_node *right) {
  bt_key s = node->k[i];
  ci->k[ci->size] = s;
  ci->size++;
  node->k[i] = right->k[0];
  // left shift right keys
  for (int j = 0; j < right->size - 1; ++j) {
    right->k[j] = right->k[j + 1];
  }
  right->k[right->size - 1] = -1;
  right->size--;
  // move child
  if (!ci->is_leaf) {
    ci->c[ci->size] = right->c[0];

    // left shift right children
    for (int j = 0; j < right->size + 1; ++j) {
      right->c[j] = right->c[j + 1];
    }
    right->c[right->size + 1] = NULL;
  }
}

static btree_node *predecessor_node(btree_node *node) {
  btree_node *p = node;
  while (!p->is_leaf) {
    p = p->c[p->size];
  }
  return p;
}

static btree_node *successor_node(btree_node *node) {
  btree_node *p = node;
  while (!p->is_leaf) {
    p = p->c[0];
  }
  return p;
}

static void merge_nodes(btree *bt, btree_node *node, int i, btree_node *left, btree_node *right) {
  // merge node.key[i] and right node into left node. both size of left and right is t-1
  bt_key key = node->k[i];
  left->k[bt->t - 1] = key;
  for (int j = bt->t; j < bt->t * 2; ++j) {
    left->k[j] = right->k[j - bt->t];
  }
  if (!left->is_leaf) {
    for (int j = bt->t; j <= bt->t * 2; ++j) {
      left->c[j] = right->c[j - bt->t];
    }
  }
  // left shift key in node
  for (int j = i; j < node->size - 1; ++j) {
    node->k[j] = node->k[j + 1];
  }
  // left shift children in node
  for (int j = i + 1; j < node->size; ++j) {
    node->c[j] = node->c[j + 1];
  }
  // update counters
  left->size = bt->t * 2 - 1;
  node->size--;
  bt->node_count--;
  // free right
  free_node_shallow(right);
}

static void free_node_shallow(btree_node *node) {
  if (node) {
    free(node->k);
    free(node->c);
    free(node);
  }
}
