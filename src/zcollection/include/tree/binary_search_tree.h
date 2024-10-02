//
// Created by ran on 2024/1/3.
//

#ifndef ZCOLLECTION_TREE_BINARY_SEARCH_TREE_H_
#define ZCOLLECTION_TREE_BINARY_SEARCH_TREE_H_

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
/**
 * The value in @ref BSTNode.
 */
typedef void *BSTData;

/**
 * The node of bst.
 */
typedef struct BSTNode BSTNode;

typedef enum {
  BST_PRE, BST_MID, BST_POST
} Tord;
/**
 * Comparator of BSTData.
 * @return Returns zero if they are equal. Positive if data1 is bigger than data2, negative otherwise.
 */
typedef int (*BSTCompare)(BSTData v1, BSTData v2);

/**
 * Function to handle the data when traversal the data.
 */
typedef void (*TsHandler)(BSTData data, void *);

/**
 * Create a new bst node.
 *
 * @param value the bst data
 * @return      pointer of the node
 */
BSTNode *new_bst_node(BSTData data);

/**
 * Add a data to the bst.
 *
 * @param root          root pointer of the bst
 * @param data          data to add
 * @param comparator    the comparator of data
 * @return              root pointer of the bst. NULL if it was not
 *                      possible to allocate the memory for the new node.
 */
BSTNode *add_to_bst(BSTNode *root, BSTData data, BSTCompare comparator);

/**
 * Check if the given data is in this bst.
 *
 * @param root          root pointer of the bst
 * @param data          data to be checked
 * @param comparator    the comparator of data
 * @return              the number of nodes in the bst that containing the data
 */
unsigned int contains_in_bst(BSTNode *root, BSTData data, BSTCompare comparator);

/**
 * Find the node containing the minimum value.
 *
 * @param root  root pointer of the bst.
 * @return      the node of in which the minimum value belong.
 */
BSTNode *max_of_bst(BSTNode *root);

/**
 * Find the max data in bst.
 *
 * @param root  root pointer of the bst.
 * @return      the data of max node.
 */
BSTData max_data_of_bst(BSTNode *root);

/**
 * Find the node containing the minimum value.
 *
 * @param root  root pointer of the bst.
 * @return      the node of in which the minimum value belong.
 */
BSTNode *min_of_bst(BSTNode *root);

/**
 * Find the min data in bst.
 *
 * @param root  root pointer of the bst.
 * @return      the data of min node.
 */
BSTData min_data_of_bst(BSTNode *root);

/**
 * Remove the max node of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      the max data. NULL if root is NULL
 */
BSTNode *remove_max_of_bst(BSTNode *root);

/**
 * Remove the min node of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      the min data. NULL if root is NULL.
 */
BSTNode *remove_min_of_bst(BSTNode *root);

/**
 * Remove the of node of given data.
 *
 * @param root          root pointer of the bst.
 * @param data          data in which node should be remove
 * @param comparator    the comparator of data
 * @param flag          indicate if any node was actually removed.
 * @return
 */
BSTNode *remove_data_in_bst(BSTNode *root, BSTData data, BSTCompare comparator, bool *flag);

/**
 * Size of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      size of the bst.
 */
unsigned int size_of_bst(BSTNode *root);

/**
 * Height of the bst.
 *
 * @param root  root pointer of the bst.
 * @return      height of the bst.
 */
unsigned int height_of_bst(BSTNode *root);

/**
 * Free the bst.
 *
 * @param root
 */
void free_bst(BSTNode *root);

/**
 * Traversal the bst.
 *
 * @param root      root pointer of the bst.
 * @param order     pre, mid or post order.
 * @param handler   the handler to handle data.
 */
void traversal_bst(BSTNode *root, Tord order, TsHandler handler, void *param);

/**
 * Get the value of node.
 *
 * @param node  node pointer of the bst.
 * @return      value of the node.
 */
BSTData value_of_node_in_bst(BSTNode *node);

/**
 * Find the floor node of given data in the bst.
 * Floor node has the biggest value but lower than given data.
 *
 * @param node  node pointer of the bst.
 * @param data          the data
 * @param comparator    the comparator of data
 * @return              floor node
 */
BSTNode *floor_of_bst(BSTNode *root, BSTData data, BSTCompare comparator);

/**
 * Find the ceil node of given data in the bst.
 * Ceil node has the smallest value but greater than given data.
 *
 * @param node  node pointer of the bst.
 * @param data          the data
 * @param comparator    the comparator of data
 * @return              ceil node
 */
BSTNode *ceil_of_bst(BSTNode *root, BSTData data, BSTCompare comparator);

/**
 * Find the rank of given data in the bst.
 * Rank means the n-th in the bst.
 *
 * @param node  node pointer of the bst.
 * @param data          the data
 * @param comparator    the comparator of data
 * @return              no-zero for the rank. zero if data is not in the bst.
 */
unsigned int rank_of_bst(BSTNode *root, BSTData data, BSTCompare comparator);

/**
 * Find the rank-th node in the bst.
 *
 * @param node  node pointer of the bst.
 * @param rank  the rank
 * @return      return the rank-th node. NULL if rank is out of the size of bst.
 */
BSTNode *select_of_bst(BSTNode *root, unsigned int rank);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_TREE_BINARY_SEARCH_TREE_H_
