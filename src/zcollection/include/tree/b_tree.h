//
// Created by ran on 2024/1/16.
//
/**
 * B-tree is a self-balancing tree data structure that maintains sorted data
 * and allows searches, sequential access, insertions, and deletions in logarithmic
 * time. The B-tree generalizes the binary search tree, allowing for nodes with more
 * than two children. Unlike other self-balancing binary search trees, the B-tree
 * is well suited for storage systems that read and write relatively large blocks
 * of data, such as databases and file systems.
 *
 * According to Knuth's definition, a B-tree of order m is a tree which satisfies
 * the following properties:
 *  1. Every node has at most m children.
 *  2. Every internal node has at least ⌈m/2⌉ children.
 *  3. The root node has at least two children unless it is a leaf.
 *  4. All leaves appear on the same level.
 *  5. A non-leaf node with k children contains k−1 keys.
 *
 *  Each internal node's keys act as separation values which divide its subtrees.
 *
 *
 */
#ifndef ZCOLLECTION_TREE_B_TREE_H_
#define ZCOLLECTION_TREE_B_TREE_H_
#ifdef __cplusplus
extern "C" {
#endif
#define BTREE_MIN_M 3

// TODO add btree value and iteration functions

typedef void *BTreeKey;

typedef int (*BTreeKeyCompareFunc)(BTreeKey key1, BTreeKey key2);

typedef void (*BTreeKeyFreeFunc)(BTreeKey key1);

typedef struct BTreeNode BTreeNode;

typedef struct BTree BTree;

BTree *new_btree(int m, BTreeKeyCompareFunc compare_func);

BTreeNode *btree_search(BTree *b_tree, BTreeKey key);

int btree_insert_key(BTree *b_tree, BTreeKey key);

void register_btree_keyfree(BTree *b_tree, BTreeKeyFreeFunc);

void free_btree(BTree *b_tree);

int sizeof_btree(BTree *b_tree);

int btree_remove(BTree *tree, BTreeKey key);

// for tests
void print_btree(BTree *tree);
int btree_validate(BTree* tree);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_TREE_B_TREE_H_
