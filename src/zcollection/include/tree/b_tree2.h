//
// Created by ran on 2024/1/20.
//
/**
 * A b-tree with pre-split when inserting and pre-rotate/merge when deleting.
 *
 */
#ifndef ZCOLLECTION_TREE_B_TREE2_H_
#define ZCOLLECTION_TREE_B_TREE2_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef int bt_key;

typedef struct btree btree;
typedef struct btree_node btree_node;

btree *bt_create(int order);

bt_key bt_search(btree *bt, bt_key key);

int bt_insert(btree *bt, bt_key key);

int bt_delete(btree *bt, bt_key key);

int bt_size(btree *bt);
void bt_free(btree *bt);
// for tests
void print_bt(btree *tree);
int bt_validate(btree *tree);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_TREE_B_TREE2_H_
