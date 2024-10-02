//
// Created by ran on 2024/1/6.
//
#include "tree/rb_tree.h"
#include <stdlib.h>
#include <stdio.h>
#include "stack/stack_ll.h"

/**
 * flag for insert.
 *
 * 3 bits.
 *  0  0  0
 *  from right to left:
 *      1st bit -> 0: parent is right node; 1: parent is left node
 *      2nd bit -> 0: current is right node; 1: current is left node
 *      3rd bit -> 0: uncle node is black; 1: uncle node is red
 *
 */
#define PARENT_LEFT (1<<2)
#define CURRENT_LEFT (1<<1)
#define UNCLE_RED 1

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
/**
 * Each node in rb-tree is either red or black.
 */
typedef enum {
  RB_TREE_RED,
  RB_TREE_BLACK
} RBTreeColor;

typedef enum {
  RB_LEFT, RB_RIGHT
} NodePosition;

struct RBTreeNode {
  RBTreeColor color;
  RBTreeKey key;
  RBTreeValue value;
  RBTreeNode *left, *right, *parent;
  unsigned int height;
  unsigned int size;
};

struct RBTreeIterator {
  StackLL *stack;
  RBTreeNode *current;
};

struct RBTree {
  RBTreeNode *root;
  RBTreeComparator comparator;
  RBTreeKeyFreeFunc key_free_func;
  RBTreeValueFreeFunc value_free_func;
};

static void left_rotate(RBTree *rb_tree, RBTreeNode *y);
static void right_rotate(RBTree *rb_tree, RBTreeNode *y);
static unsigned int size_of_node(RBTreeNode *node);
static void increase_size(RBTree *tree, RBTreeNode *node);
static RBTreeNode *parent_node(RBTreeNode *node);
static RBTreeNode *sibling_node(RBTreeNode *node);
static RBTreeColor color(RBTreeNode *node);
static void insert_fixup_tree(RBTree *rb_tree, RBTreeNode *o);
static unsigned int node_height(RBTreeNode *node);
static void fixup_height(RBTree *tree, RBTreeNode *node);
static void free_rb_node(RBTreeNode *node, RBTreeKeyFreeFunc, RBTreeValueFreeFunc);
static int number_of_children(RBTreeNode *node);
static void remove_balance(RBTree *tree, RBTreeNode *parent, RBTreeNode *sibling, NodePosition position);
static void remove_node_0child(RBTree *tree, RBTreeNode *node);
static void remove_node_1child(RBTree *tree, RBTreeNode *node);
static RBTreeNode *successor_node(RBTree *tree, RBTreeNode *node);
static void decrease_size(RBTree *tree, RBTreeNode *node);
static void swap_color(RBTreeNode *node1, RBTreeNode *node2);
static void in_order_rbt(RBTreeIterator *rb_tree_iterator);

RBTreeNode *new_rb_node(RBTreeKey key, RBTreeValue value) {
  RBTreeNode *node = malloc(sizeof(RBTreeNode));
  if (node == NULL) return NULL;
  node->key = key;
  node->value = value;
  // new node is red color
  node->color = RB_TREE_RED;
  node->size = 1;
  node->height = 1;
  node->left = NULL, node->right = NULL, node->parent = NULL;
  return node;
}

RBTree *new_rb_tree(RBTreeComparator comparator) {
  RBTree *rb_tree = malloc(sizeof(RBTree));
  if (rb_tree == NULL) return NULL;
  rb_tree->root = NULL;
  rb_tree->comparator = comparator;
  rb_tree->key_free_func = NULL;
  rb_tree->value_free_func = NULL;
  return rb_tree;
}

int contains_in_rb_tree(RBTree *rb_tree, RBTreeKey key) {
  return get_node_rb_tree(rb_tree, key) != NULL;
}

RBTreeNode *get_node_rb_tree(RBTree *tree, RBTreeKey key) {
  RBTreeNode *c = tree->root;
  while (c != NULL) {
    if (key == NULL && c->key == NULL) {
      return c;
    } else if (key == NULL) {
      c = c->left;
    } else if (c->key == NULL) {
      c = c->right;
    } else if (tree->comparator(key, c->key) > 0) {
      c = c->right;
    } else if (tree->comparator(key, c->key) < 0) {
      c = c->left;
    } else {
      return c;
    }
  }
  return c;
}

void get_hold_value_rb_tree(RBTree *rb_tree, RBTreeKey key, int *flag, RBTreeValue *value) {
  RBTreeNode *node = get_node_rb_tree(rb_tree, key);
  if (node == NULL) {
    *flag = 0;
    *value = NULL;
  } else {
    *flag = 1;
    *value = node->value;
  }
}

void register_rb_tree_free_functions(RBTree *rb_tree,
                                     RBTreeKeyFreeFunc key_free_func,
                                     RBTreeValueFreeFunc value_free_func) {
  rb_tree->key_free_func = key_free_func;
  rb_tree->value_free_func = value_free_func;
}

RBTreeValue get_value_rb_tree(RBTree *tree, RBTreeKey key) {
  RBTreeNode *node = get_node_rb_tree(tree, key);
  if (node == NULL) return NULL;
  return node->value;
}

RBTreeNode *put_rb_tree(RBTree *rb_tree, RBTreeKey key, RBTreeValue value) {
  /**
   * properties make a tree a read-black tree:
   *
   *  1. The root node and external nodes are black;
   *  2. There are no consecutive 2 red nodes on the path from the root to
   *     all external nodes;
   *  3. All paths from roots to external nodes have the same number of black
   *     nodes.
   *
   * New node is RED and may break property #2 if its parent node is RED.
   *
   */
  if (rb_tree == NULL) return NULL;
  if (rb_tree->root == NULL) {
    // if root is NULL, make the node root and change its color to RED.
    RBTreeNode *node = new_rb_node(key, value);
    if (node == NULL) return NULL;
    node->color = RB_TREE_BLACK;
    rb_tree->root = node;
    return node;
  }
  // make x the current node
  RBTreeNode *x = rb_tree->root;
  // make y the parent of key node to insert
  RBTreeNode *y = NULL;
  // 00000 000 low 3 bit for parent right, current right, uncle black
  char c = 0x00;

  while (x != NULL) {
    y = x;
    if (key == NULL && x->key == NULL) {
      x->value = value;
      return x;
    } else if (key == NULL) {
      // NULL key is less than any other key.
      c |= CURRENT_LEFT;
      x = x->left;
    } else if (x->key == NULL) {
      // key to insert in the right subtree
      c &= ~CURRENT_LEFT;
      x = x->right;
    } else if (rb_tree->comparator(key, x->key) > 0) {
      // key to insert in the right subtree
      c &= ~CURRENT_LEFT;
      x = x->right;
    } else if (rb_tree->comparator(key, x->key) < 0) {
      // key to insert in the left subtree
      c |= CURRENT_LEFT;
      x = x->left;
    } else {
      // update the value
      x->value = value;
      return x;
    }
  }
  RBTreeNode *node = new_rb_node(key, value);
  if (node == NULL) return NULL;
  node->parent = y;

  if (c & CURRENT_LEFT) {
    y->left = node;
    if (y->right == NULL) fixup_height(rb_tree, y);
  } else {
    y->right = node;
    if (y->left == NULL) fixup_height(rb_tree, y);
  }
  increase_size(rb_tree, y);

  if (y->color == RB_TREE_BLACK) {
    // parent color is black, no need to fixup tree
    return node;
  }
  // parent is red. property #2 is broken. fixup tree.
  insert_fixup_tree(rb_tree, node);
  rb_tree->root->color = RB_TREE_BLACK;
  fixup_height(rb_tree, rb_tree->root);
  return node;
}

/**
 * Remove a node with given key
 *
 * @param tree  pointer of rb tree
 * @param key   the removed node. NULL if not found.
 * @return
 */
RBTreeValue remove_rb_tree(RBTree *rb_tree, RBTreeKey key) {
  int flag = 0;
  return remove_with_flag_rb_tree(rb_tree, key, &flag);
}

RBTreeValue remove_with_flag_rb_tree(RBTree *rb_tree, RBTreeKey key, int *flag) {
  if (rb_tree == NULL) {
    *flag = 0;
    return NULL;
  }
  // find the node to delete
  RBTreeNode *d = rb_tree->root;
  while (d != NULL) {
    if (key == NULL && d->key == NULL) {
      break;
    } else if (key == NULL) {
      d = d->left;
    } else if (d->key == NULL) {
      d = d->right;
    } else if (rb_tree->comparator(key, d->key) > 0) {
      d = d->right;
    } else if (rb_tree->comparator(key, d->key) < 0) {
      d = d->left;
    } else {
      break;
    }
  }
  // node was not found
  if (d == NULL) {
    *flag = 0;
    return NULL;
  }
  *flag = 1;
  RBTreeValue ret = d->value;
  int n = number_of_children(d);
  if (n == 0) {
    remove_node_0child(rb_tree, d);
  } else if (n == 1) {
    remove_node_1child(rb_tree, d);
  } else {
    RBTreeNode *successor = successor_node(rb_tree, d);
    RBTreeKey s_key = successor->key;
    RBTreeValue s_value = successor->value;
    // must be 1 or 0
    int sn = number_of_children(successor);
    if (sn == 0) remove_node_0child(rb_tree, successor);
    else remove_node_1child(rb_tree, successor);
    // reset key and value of successor after remove successor
    d->key = s_key;
    d->value = s_value;
  }
  return ret;
}

/**
 * Remove a node
 *
 * @param tree  pointer of rb tree
 * @param node  node to remove
 * @return
 */
RBTreeValue remove_node_rb_tree(RBTree *tree, RBTreeNode *d) {
  return remove_rb_tree(tree, d->key);
}

unsigned int height_of_rb_node(RBTreeNode *node) {
  return node_height(node);
}

unsigned int height_of_rbt(RBTree *rb_tree) {
  if (rb_tree == NULL) return 0;
  return node_height(rb_tree->root);
}

unsigned int size_of_rbt(RBTree *rb_tree) {
  return rb_tree == NULL || rb_tree->root == NULL ? 0 : rb_tree->root->size;
}

int has_next_rbt_entry(RBTreeIterator *iterator) {
  return iterator->current != NULL || !is_empty_stack_ll(iterator->stack);
}

RBNodeEntry next_rbt_entry(RBTreeIterator *iterator) {
  RBTreeNode *ret = next_rbt_node(iterator);
  RBNodeEntry entry = {.key=ret->key, .value=ret->value};
  return entry;
}

RBTreeNode *next_rbt_node(RBTreeIterator *iterator) {
  RBTreeNode *ret = iterator->current;
  RBTreeNode *current = ret->right;
  iterator->current = NULL;
  if (current != NULL || !is_empty_stack_ll(iterator->stack)) {
    while (current != NULL) {
      push_stack_ll(iterator->stack, current);
      current = current->left;
    }
    iterator->current = pop_stack_ll(iterator->stack);
  }
  return ret;
}

RBTreeIterator *rb_tree_iterator(RBTree *rb_tree) {
  if (rb_tree == NULL) return NULL;
  RBTreeIterator *iterator = malloc(sizeof(RBTreeIterator));
  iterator->stack = new_stack_ll();
  iterator->current = rb_tree->root;
  in_order_rbt(iterator);
  return iterator;
}
RBTreeKey key_of_rb_node(RBTreeNode *node) {
  return node->key;
}

RBTreeValue value_of_rb_node(RBTreeNode *node) {
  return node->value;
}

void set_value_of_rb_node(RBTreeNode *node, RBTreeValue value) {
  node->value = value;
}

double rb_tree_diff_lr_height(RBTreeNode *node) {
  unsigned int left_h = height_of_rb_node(node->left) + 1;
  unsigned int right_h = height_of_rb_node(node->right) + 1;
  return 1.0 * left_h / right_h;
}

void free_rb_tree(RBTree *rb_tree) {
  if (rb_tree == NULL) return;
  if (rb_tree->root != NULL) {
    free_rb_node(rb_tree->root, rb_tree->key_free_func, rb_tree->value_free_func);
  }
  free(rb_tree);
}

void free_rb_tree_iterator(RBTreeIterator *iterator) {
  if (iterator == NULL) return;
  free_stack_ll(iterator->stack);
  free(iterator);
}

// static functions
static void free_rb_node(RBTreeNode *node, RBTreeKeyFreeFunc key_free_func, RBTreeValueFreeFunc value_free_func) {
  if (node == NULL) return;
  free_rb_node(node->left, key_free_func, value_free_func);
  free_rb_node(node->right, key_free_func, value_free_func);

  if (key_free_func != NULL) {
    key_free_func(node->key);
  }
  if (value_free_func != NULL) {
    value_free_func(node->value);
  }
  free(node);
}

/**
 * get height of node
 *
 * @param node
 * @return      height of node, 1 if node is NULL
 */

static unsigned int size_of_node(RBTreeNode *node) {
  if (node == NULL) return 0;
  return node->size;
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
static void left_rotate(RBTree *rb_tree, RBTreeNode *y) {
  RBTreeNode *x = y->right;
  RBTreeNode *t2 = x->left;

  y->right = t2;
  if (t2 != NULL) t2->parent = y;

  x->parent = y->parent;
  if (y->parent == NULL) {
    rb_tree->root = x;
  } else if (y == y->parent->left) {
    y->parent->left = x;
  } else if (y == y->parent->right) {
    y->parent->right = x;
  }

  x->left = y;
  y->parent = x;

  y->size = 1 + size_of_node(y->left) + size_of_node(y->right);
  x->size = 1 + size_of_node(x->left) + size_of_node(x->right);

  y->height = 1 + max(node_height(y->left), node_height(y->right));
  x->height = 1 + max(node_height(x->left), node_height(x->right));
  if (x->parent != NULL) {
    x->parent->height = 1 + max(node_height(x->parent->left), node_height(x->parent->right));
  }
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
static void right_rotate(RBTree *rb_tree, RBTreeNode *y) {
  RBTreeNode *x = y->left;
  if (x == NULL) {
    x = NULL;
  }
  RBTreeNode *t2 = x->right;

  y->left = t2;
  if (t2 != NULL) t2->parent = y;

  x->parent = y->parent;
  if (y->parent == NULL) {
    rb_tree->root = x;
  } else if (y == y->parent->left) {
    y->parent->left = x;
  } else if (y == y->parent->right) {
    y->parent->right = x;
  }

  x->right = y;
  y->parent = x;

  y->size = 1 + size_of_node(y->left) + size_of_node(y->right);
  x->size = 1 + size_of_node(x->left) + size_of_node(x->right);

  y->height = 1 + max(node_height(y->left), node_height(y->right));
  x->height = 1 + max(node_height(x->left), node_height(x->right));
  if (x->parent != NULL) {
    x->parent->height = 1 + max(node_height(x->parent->left), node_height(x->parent->right));
  }
}

static void increase_size(RBTree *tree, RBTreeNode *node) {
  RBTreeNode *p = node;
  while (p != NULL) {
    p->size++;
    p = p->parent;
  }
}

static void decrease_size(RBTree *tree, RBTreeNode *node) {
  RBTreeNode *p = node;
  while (p != NULL) {
    p->size--;
    p = p->parent;
  }
}

static RBTreeNode *parent_node(RBTreeNode *node) {
  return node->parent;
}
static RBTreeNode *sibling_node(RBTreeNode *node) {
  return node->parent->left == node ? node->parent->right : node->parent->left;
}
static RBTreeColor color(RBTreeNode *node) {
  return node == NULL ? RB_TREE_BLACK : node->color;
}

static void insert_fixup_tree(RBTree *rb_tree, RBTreeNode *o) {
  // parent node
  RBTreeNode *p = parent_node(o);
  // grand node
  RBTreeNode *g = parent_node(p);
  // uncle node
  RBTreeNode *u = sibling_node(p);
  char c = 0x00;
  c |= p->left == o ? CURRENT_LEFT : 0, c |= g->left == p ? PARENT_LEFT : 0, c |=
      color(u) == RB_TREE_RED ? UNCLE_RED : 0;

  if ((c == (PARENT_LEFT | CURRENT_LEFT | UNCLE_RED)) || (c == (PARENT_LEFT | UNCLE_RED))) {
    // LLr and LRr
    p->color = RB_TREE_BLACK;
    u->color = RB_TREE_BLACK;
    g->color = RB_TREE_RED;
    if (color(g->parent) != RB_TREE_BLACK) insert_fixup_tree(rb_tree, g);
  } else if (c == (PARENT_LEFT | CURRENT_LEFT)) {
    // LLb
    p->color = RB_TREE_BLACK;
    g->color = RB_TREE_RED;

    right_rotate(rb_tree, g);
  } else if (c == PARENT_LEFT) {
    // LRb
    left_rotate(rb_tree, p);
    insert_fixup_tree(rb_tree, p);
  } else if ((c == (CURRENT_LEFT | UNCLE_RED)) || (c == UNCLE_RED)) {
    //RLr and RRr
    p->color = RB_TREE_BLACK;
    u->color = RB_TREE_BLACK;
    g->color = RB_TREE_RED;
    if (color(g->parent) != RB_TREE_BLACK) insert_fixup_tree(rb_tree, g);
  } else if (c == CURRENT_LEFT) {
    //RLb <=> LRb
    right_rotate(rb_tree, p);
    insert_fixup_tree(rb_tree, p);
  } else if (c == 0) {
    //RRb <=> LLb
    p->color = RB_TREE_BLACK;
    g->color = RB_TREE_RED;

    left_rotate(rb_tree, g);
  } else {
  }
}

/**
 * get height of node
 *
 * @param node
 * @return      height of node, 0 if node is NULL
 */
static unsigned int node_height(RBTreeNode *node) {
  return node == NULL ? 0 : node->height;
}

static void fixup_height(RBTree *tree, RBTreeNode *node) {
  unsigned int o_height = node->height;
  unsigned int n_height = 1 + max(node_height(node->left), node_height(node->right));
  if (o_height != n_height) {
    node->height = n_height;
    while (node->parent != NULL) {
      node = node->parent;
      node->height = 1 + max(node_height(node->left), node_height(node->right));
    }
  }
}

static int number_of_children(RBTreeNode *node) {
  int c = 0;
  if (node->left != NULL) c++;
  if (node->right != NULL) c++;
  return c;
}

static void in_order_rbt(RBTreeIterator *rb_tree_iterator) {
  StackLL *stack_al = rb_tree_iterator->stack;
  RBTreeNode *current = rb_tree_iterator->current;
  while (current != NULL) {
    push_stack_ll(stack_al, current);
    current = current->left;
  }
  rb_tree_iterator->current = pop_stack_ll(stack_al);
}

/**
 * Remove a node with 0 children.
 * - if the node is red
 *     remove the node
 * - if the node is black
 *     remove and fixup balance
 *
 * @param tree
 * @param node
 */
static void remove_node_0child(RBTree *tree, RBTreeNode *node) {
  RBTreeNode *parent = parent_node(node);
  if (parent == NULL) {
    // this node is root.
    tree->root = NULL;
    free(node);
    return;
  }
  NodePosition sibling_p = parent->left == node ? RB_RIGHT : RB_LEFT;
  RBTreeColor node_c = node->color;
  // remove node
  if (sibling_p == RB_LEFT) {
    parent->right = NULL;
  } else {
    parent->left = NULL;
  }
  fixup_height(tree, parent);
  decrease_size(tree, parent);
  free(node);

  if (node_c == RB_TREE_BLACK) {
    // node is black, need to balance tree
    // must have a sibling node
    RBTreeNode *sibling = sibling_p == RB_RIGHT ? parent->right : parent->left;
    remove_balance(tree, parent, sibling, sibling_p);
  }
}

/**
 * The node must be black and it child must be red
 *
 * @param tree
 * @param node
 */
static void remove_node_1child(RBTree *tree, RBTreeNode *node) {
  RBTreeNode *parent = parent_node(node);
  RBTreeNode *child = node->left != NULL ? node->left : node->right;
  if (parent == NULL) {
    tree->root = child;
    child->color = RB_TREE_BLACK;
    child->parent = NULL;
    free(node);
    return;
  }
  bool node_left = parent->left == node;
  if (node_left) {
    parent->left = child;
  } else {
    parent->right = child;
  }
  child->parent = parent;
  child->color = RB_TREE_BLACK;
  free(node);
  decrease_size(tree, parent);
  fixup_height(tree, parent);
}

static RBTreeNode *successor_node(RBTree *tree, RBTreeNode *node) {
  RBTreeNode *s = node->right;
  while (s->left != NULL) {
    s = s->left;
  }
  return s;
}

static void swap_color(RBTreeNode *node1, RBTreeNode *node2) {
  RBTreeColor node1c = color(node1);
  node1->color = color(node2);
  node2->color = node1c;
}

/**
 * Balance tree after removing node. The removed node is N, the balancing node.
 *
 * @param tree
 * @param parent
 * @param sibling
 * @param position
 */
static void remove_balance(RBTree *tree, RBTreeNode *parent, RBTreeNode *sibling, NodePosition position) {
  // 1. N is root, parent is NULL, no need to balance
  if (parent == NULL) {
    return;
  }
  // 2. sibling is black
  if (color(sibling) == RB_TREE_BLACK) {
    // 2.1 all children of sibling is black
    if (color(sibling->left) == RB_TREE_BLACK && color(sibling->right) == RB_TREE_BLACK) {
      if (color(parent) == RB_TREE_RED) {
        //2.1.1 parent is red. swap color of P ,S
        swap_color(parent, sibling);
        return;
      } else {
        // 2.1.2 parent is black. turn S to red, let P as new N and recursive
        sibling->color = RB_TREE_RED;
        RBTreeNode *new_n = parent;
        RBTreeNode *new_p = parent_node(new_n);

        if (new_p != NULL) {
          NodePosition new_sp = new_p->left == new_n ? RB_RIGHT : RB_LEFT;
          RBTreeNode *new_s = new_sp == RB_RIGHT ? new_p->right : new_p->left;
          remove_balance(tree, new_p, new_s, new_sp);
        } else {
          // new N is root
          return;
        }
      }
    } else {
      //2.2 not all children of sibling is black
      if (position == RB_LEFT && color(sibling->left) == RB_TREE_RED) {
        // 2.2.1 sibling is left and sl is read
        swap_color(parent, sibling);
        right_rotate(tree, parent);
        sibling->left->color = RB_TREE_BLACK;
        return;
      } else if (position == RB_RIGHT && color(sibling->right) == RB_TREE_RED) {
        // 2.2.2 sibling is right and sr is red
        swap_color(parent, sibling);
        left_rotate(tree, parent);
        sibling->right->color = RB_TREE_BLACK;
        return;
      } else if (position == RB_LEFT && color(sibling->left) == RB_TREE_BLACK) {
        // 2.2.3 sibling is left and sl is black
        swap_color(sibling, sibling->right);
        left_rotate(tree, sibling);
        RBTreeNode *new_s = parent->left;
        remove_balance(tree, parent, new_s, RB_LEFT);
      } else if (position == RB_RIGHT && color(sibling->right) == RB_TREE_BLACK) {
        // 2.2.4 sibling is right and sr is black
        swap_color(sibling, sibling->left);
        right_rotate(tree, sibling);
        RBTreeNode *new_s = parent->right;
        remove_balance(tree, parent, new_s, RB_RIGHT);
      }
    }
  } else {
    // 3. sibling is red
    if (position == RB_LEFT) {
      swap_color(parent, sibling);
      right_rotate(tree, parent);
      RBTreeNode *new_s = parent->left;
      remove_balance(tree, parent, new_s, RB_LEFT);
    } else {
      swap_color(parent, sibling);
      left_rotate(tree, parent);
      RBTreeNode *new_s = parent->right;
      remove_balance(tree, parent, new_s, RB_RIGHT);
    }
  }
}