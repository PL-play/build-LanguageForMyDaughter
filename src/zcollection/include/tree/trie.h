//
// Created by ran on 2024/1/10.
//

#ifndef ZCOLLECTION_TREE_TIRE_H_
#define ZCOLLECTION_TREE_TIRE_H_
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct TrieNode TrieNode;

typedef struct Trie Trie;

typedef void *TrieValue;

/**
 * Create a new tire.
 *
 * @return
 */
Trie *new_trie();

int insert_trie(Trie *trie, char *key, TrieValue value);

bool contains_in_trie(Trie *trie, char *key);

bool is_prefix_in_trie(Trie *trie, char *prefix);

TrieValue get_value_trie(Trie *trie, char *key);

unsigned int size_of_trie(Trie *trie);

int remove_key_trie(Trie *trie, char *key);

void free_trie(Trie *trie);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_TREE_TIRE_H_
