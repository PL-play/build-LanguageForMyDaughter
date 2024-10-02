//
// Created by ran on 2024/1/3.
//
#include "tree/binary_search_tree.h"
#include <stdlib.h>
#include <stdio.h>
/**
 * The basic data structure in the tree.
 */
struct BSTNode {
  BSTData value;  /** value of node */
  BSTNode *left;  /** left child */
  BSTNode *right; /** right child */
  unsigned int size; /** size of this node */
};

static BSTNode *remove_max_in_bst_internal(BSTNode *node);
static BSTNode *remove_min_in_bst_internal(BSTNode *node);
static BSTNode *add_to_bst_internal(BSTNode *root, BSTData data, BSTCompare comparator, bool *flag);
static unsigned int rank_of_bst_internal(BSTNode *root, BSTData data, unsigned int rank, BSTCompare comparator);
/**
 * Create a new bst node.
 *
 * @param value the bst data
 * @return      pointer of the node
 */
BSTNode *new_bst_node(BSTData data) {
  BSTNode *node = malloc(sizeof(BSTNode));
  if (node == NULL) return NULL;

  node->value = data;
  node->left = NULL;
  node->right = NULL;
  node->size = 1;
  return node;
}

/**
 * Add a data to the bst.
 *
 * @param root          root pointer of the bst
 * @param data          data to add
 * @param comparator    the comparator of data
 * @return              root pointer of the bst. NULL if it was not
 *                      possible to allocate the memory for the new node.
 */
BSTNode *add_to_bst(BSTNode *root, BSTData data, BSTCompare comparator) {
  bool flag = false;
  return add_to_bst_internal(root, data, comparator, &flag);
}

static BSTNode *add_to_bst_internal(BSTNode *root, BSTData data, BSTCompare comparator, bool *flag) {
  // If the root is null, create a new node.
  if (root == NULL) {
    root = new_bst_node(data);
    if (root == NULL) return root;
    *flag = true;
    return root;
  }

  if (comparator(data, root->value) > 0) {
    // add the data in the right leaf
    bool add_flag = false;
    root->right = add_to_bst_internal(root->right, data, comparator, &add_flag);
    // increase the size of this node.
    if (add_flag) {
      *flag = true;
      root->size++;
    }

  } else if (comparator(data, root->value) < 0) {
    // increase the size of this node.
    bool add_flag = false;
    // add the data in the left leaf
    root->left = add_to_bst_internal(root->left, data, comparator, &add_flag);
    if (add_flag) {
      *flag = true;
      root->size++;
    }
  } else {
    *flag = false;
  }
  return root;
}

/**
 * Check if the given data is in this bst.
 *
 * @param root          root pointer of the bst
 * @param data          data to be checked
 * @param comparator    the comparator of data
 * @return              the number of nodes in the bst that containing the data
 */
unsigned int contains_in_bst(BSTNode *root, BSTData data, BSTCompare comparator) {
  if (root == NULL) return 0;
  if (comparator(data, root->value) == 0) {
    // current node's value is the data. return the count of this data
    return 1;
  } else if (comparator(data, root->value) > 0) {
    // find in the right leaf
    return contains_in_bst(root->right, data, comparator);
  } else {
    // find in the left leaf
    return contains_in_bst(root->left, data, comparator);
  }
}

/**
 * Find the node containing the minimum value.
 *
 * @param root  root pointer of the bst.
 * @return      the node of in which the minimum value belong.
 */
BSTNode *max_of_bst(BSTNode *root) {
  if (root == NULL || root->right == NULL) return root;
  return max_of_bst(root->right);
}

BSTData max_data_of_bst(BSTNode *root) {
  BSTNode *max = max_of_bst(root);
  return value_of_node_in_bst(max);
}

/**
 * Find the node containing the minimum value.
 *
 * @param root  root pointer of the bst.
 * @return      the node of in which the minimum value belong.
 */
BSTNode *min_of_bst(BSTNode *root) {
  if (root == NULL || root->left == NULL) return root;
  return min_of_bst(root->left);
}

BSTData min_data_of_bst(BSTNode *root) {
  BSTNode *min = min_of_bst(root);
  return value_of_node_in_bst(min);
}
/**
 * Remove the max node of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      pointer of new root
 */
BSTNode *remove_max_of_bst(BSTNode *root) {
  if (root == NULL) return root;
  root = remove_max_in_bst_internal(root);
  return root;
}
/**
 * Remove the max node and return the new root after removing.
 *
 * @param node  current root of bst
 * @return      the new root after removing
 */
static BSTNode *remove_max_in_bst_internal(BSTNode *node) {
  if (node->right == NULL) {
    // this node is the max node, return it's left node
    BSTNode *left = node->left;
    // free the current node
    free(node);
    return left;
  }
  node->right = remove_max_in_bst_internal(node->right);
  node->size--;
  return node;
}

/**
 * Remove the min node of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      pointer of new root
 */
BSTNode *remove_min_of_bst(BSTNode *root) {
  if (root == NULL) return root;
  root = remove_min_in_bst_internal(root);
  return root;
}

static BSTNode *remove_min_in_bst_internal(BSTNode *node) {
  if (node->left == NULL) {
    // this node is the min node, return it's right node
    BSTNode *right = node->right;
    // free the current node
    free(node);
    return right;
  }
  node->left = remove_min_in_bst_internal(node->left);
  node->size--;
  return node;
}
/**
 * Remove the node of given data.
 *
 * @param root          root pointer of the bst.
 * @param data          data in which node should be remove
 * @param comparator    the comparator of data
 * @return              pointer of new root
 */
BSTNode *remove_data_in_bst(BSTNode *root, BSTData data, BSTCompare comparator, bool *flag) {
  // if root is NULL, nothing to be done
  if (root == NULL) return root;
  if (comparator(data, root->value) > 0) {
    // if data bigger than root's value, delete from the right subtree
    bool remove_flag = false;
    root->right = remove_data_in_bst(root->right, data, comparator, &remove_flag);
    if (remove_flag) {
      *flag = true;
      root->size--;
    }
    return root;
  } else if (comparator(data, root->value) < 0) {
    // if data lower than root's value, delete from the left subtree
    bool remove_flag = false;
    root->left = remove_data_in_bst(root->left, data, comparator, &remove_flag);
    if (remove_flag) {
      *flag = true;
      root->size--;
    }
    return root;
  } else {
    *flag = true;
    // the root itself is to remove
    if (root->left == NULL) {
      // #1 the root has no left subtree, remove this node and make it's right node the new root
      BSTNode *right = root->right;
      free(root);
      return right;
    } else if (root->right == NULL) {
      // #2 the root has no right subtree, remove this node and make it's left node the new root
      BSTNode *left = root->left;
      free(root);
      return left;
    } else {
      // #3. both left and right node is not NULL. Find the successor of this node.
      // Find the biggest node in left subtree to be the successor.
      BSTNode *successor = min_of_bst(root->right);
      root->value = successor->value;
      root->right = remove_min_of_bst(root->right);
      root->size--;
      return root;
    }
  }
}

/**
 * Size of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      size of the bst.
 */
unsigned int size_of_bst(BSTNode *root) {
  if (root == NULL) return 0;
  return root->size;
}

/**
 * Height of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      height of the bst.
 */
unsigned int height_of_bst(BSTNode *root) {
  if (root == NULL) return 0;
  unsigned int right_h = height_of_bst(root->right);
  unsigned int left_h = height_of_bst(root->left);
  return right_h > left_h ? right_h + 1 : left_h + 1;
}

/**
 * Free the bst.
 *
 * @param root
 */
void free_bst(BSTNode *root) {
  if (root == NULL) return;
  free_bst(root->left);
  free_bst(root->right);
  free(root);
}

/**
 * Traversal the bst.
 *
 * @param root      root pointer of the bst.
 * @param order     pre, mid or post order.
 * @param handler   the handler to handle data.
 */
void traversal_bst(BSTNode *root, Tord order, TsHandler handler, void *param) {
  if (root == NULL) return;
  if (order == BST_PRE) {
    handler(root->value, param);
  }
  traversal_bst(root->left, order, handler, param);
  if (order == BST_MID) {
    handler(root->value, param);
  }
  traversal_bst(root->right, order, handler, param);
  if (order == BST_POST) {
    handler(root->value, param);
  }
}

/**
 * Get the value of node.
 *
 * @param node  node pointer of the bst.
 * @return      value of the node.
 */
BSTData value_of_node_in_bst(BSTNode *node) {
  if (node == NULL) return NULL;
  return node->value;
}

/**
 * Find the floor node of given data in the bst.
 * Floor node has the biggest value but lower than given data.
 * Floor <= data.
 *
 * @param root      root pointer of the bst.
 * @param data          the data
 * @param comparator    the comparator of data
 * @return              floor node
 */
BSTNode *floor_of_bst(BSTNode *root, BSTData data, BSTCompare comparator) {
  if (root == NULL) return NULL;
  if (comparator(data, root->value) == 0) {
    return root;
  } else if (comparator(root->value, data) < 0) {
    // root < data, could be floor, or there is a floor in it's right subtree.
    BSTNode *candidate = floor_of_bst(root->right, data, comparator);
    return candidate == NULL ? root : candidate;
  } else {
    // root > data, floor can only be in it's left subtree
    return floor_of_bst(root->left, data, comparator);
  }
}

/**
 * Find the ceil node of given data in the bst.
 * Ceil node has the smallest value but greater than given data.
 * Ceil >= data.
 *
 * @param root      root pointer of the bst.
 * @param data          the data
 * @param comparator    the comparator of data
 * @return              ceil node
 */
BSTNode *ceil_of_bst(BSTNode *root, BSTData data, BSTCompare comparator) {
  if (root == NULL) return NULL;
  if (comparator(data, root->value) == 0) {
    return root;
  } else if (comparator(data, root->value) > 0) {
    return ceil_of_bst(root->right, data, comparator);
  } else {
    BSTNode *candidate = ceil_of_bst(root->left, data, comparator);
    return candidate == NULL ? root : candidate;
  }
}

/**
 * Find the rank of given data in the bst.
 * Rank means the n-th in the bst.
 *
 * @param root      root pointer of the bst.
 * @param data          the data
 * @param comparator    the comparator of data
 * @return              no-zero for the rank. zero if data is not in the bst.
 */
unsigned int rank_of_bst(BSTNode *root, BSTData data, BSTCompare comparator) {
  if (root == NULL) return 0;
  if (comparator(data, root->value) < 0) {
    return rank_of_bst(root->left, data, comparator);
  } else if (comparator(data, root->value) > 0) {
    unsigned int right_rank = rank_of_bst(root->right, data, comparator);
    if (right_rank == 0) return 0;
    unsigned int current_rank = root->left == NULL ? 1 : root->left->size + 1;
    return current_rank + right_rank;
  } else {
    return root->left == NULL ? 1 : root->left->size + 1;
  }
}

static unsigned int rank_of_bst_internal(BSTNode *root, BSTData data, unsigned int rank, BSTCompare comparator) {
  if (root == NULL || rank == 0) return 0;
  if (comparator(data, root->value) == 0) {
    return rank;
  } else if (comparator(data, root->value) > 0) {
    return rank_of_bst_internal(root->right, data, rank + 1, comparator);
  } else {
    return rank_of_bst_internal(root->left, data, rank - 1, comparator);
  }
}

/**
 * Find the rank-th node in the bst.
 *
 * @param root      root pointer of the bst.
 * @param rank  the rank
 * @return      return the rank-th node. NULL if rank is out of the size of bst.
 */
BSTNode *select_of_bst(BSTNode *root, unsigned int rank) {
  if (rank == 0 || root == NULL || rank > root->size) return NULL;
  unsigned int current_root_rank;
  if (root->left != NULL) {
    current_root_rank = root->left->size + 1;
  } else {
    current_root_rank = 1;
  }
  if (rank == current_root_rank) {
    return root;
  } else if (rank > current_root_rank) {
    return select_of_bst(root->right, rank - current_root_rank);
  } else {
    return select_of_bst(root->left, rank);
  }

}