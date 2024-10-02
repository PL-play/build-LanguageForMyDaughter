//
// Created by ran on 2024/1/10.
//
#include "tree/trie.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct TrieNode {
  TrieValue data;
  bool is_word;
  // use char as key so the next is a 256 array
  TrieNode *next[256];
};

struct Trie {
  TrieNode *root;
  unsigned int size;
};

static TrieNode *new_trie_node();

Trie *new_trie() {
  Trie *trie = malloc(sizeof(Trie));
  if (trie == NULL) return NULL;
  trie->root = new_trie_node();
  if (trie->root == NULL) {
    free(trie);
    return NULL;
  }
  trie->size = 0;
  return trie;
}

static void rollback_cache(Trie *trie, TrieNode *cached[], unsigned int size) {
  for (int i = 0; i < size; i++) {
    free(cached[i]);
  }
}

static int insert0_trie(Trie *trie, char *key, TrieValue value) {
  TrieNode *node = trie->root->next[0];
  if (node == NULL) {
    TrieNode *new_node = new_trie_node();
    if (new_node == NULL) return 0;
    trie->root->next[0] = new_node;
    node = new_node;
  }
  if (!node->is_word) {
    node->is_word = true;
    trie->size++;
  }
  node->data = value;
  return 1;
}

int insert_trie(Trie *trie, char *key, TrieValue value) {
  if (key == NULL) return 0;
  TrieNode *node = trie->root;
  if (*key == '\0') {
    return insert0_trie(trie, key, value);
  }
  unsigned int key_size = 0;
  for (char *p = key; p != NULL && *p != '\0'; ++p) {
    key_size++;
  }
  TrieNode *cached[key_size];
  unsigned int cached_size = 0;
  TrieNode *first_new_node = NULL;
  char *start = key;
  for (int i = 0; i < key_size; ++i) {
    char *p = key + i;
    if (node->next[(unsigned char) *p] == NULL) {
      TrieNode *new_node = new_trie_node();
      if (new_node == NULL) {
        rollback_cache(trie, cached, cached_size);
        if (first_new_node != NULL) {
          first_new_node->next[(unsigned char) *start] = NULL;
        }
        return 0;
      }
      cached[cached_size++] = new_node;
      node->next[(unsigned char) *p] = new_node;
      if (first_new_node == NULL) {
        first_new_node = node;
        start = p;
      }
    }
    node = node->next[(unsigned char) *p];
  }
  if (!node->is_word) {
    node->is_word = true;
    trie->size++;
  }
  node->data = value;
  return 1;
}

bool contains_in_trie(Trie *trie, char *key) {
  TrieNode *cur = trie->root;
  for (char *p = key; p != NULL && *p != '\0'; ++p) {
    if (cur->next[(unsigned char) *p] == NULL) {
      return false;
    }
    cur = cur->next[(unsigned char) *p];
  }
  return cur->is_word;
}
bool is_prefix_in_trie(Trie *trie, char *prefix) {
  TrieNode *cur = trie->root;
  for (char *p = prefix; p != NULL && *p != '\0'; ++p) {
    if (cur->next[(unsigned char) *p] == NULL) {
      return false;
    }
    cur = cur->next[(unsigned char) *p];
  }
  return true;
}

TrieValue get_0_value(Trie *trie, char *key) {
  if (trie->root->next[0] != NULL) {
    return trie->root->next[0]->data;
  }
  return NULL;
}

TrieValue get_value_trie(Trie *trie, char *key) {
  if (*key == '\0') {
    return get_0_value(trie, key);
  }
  TrieNode *cur = trie->root;
  for (char *p = key; p != NULL && *p != '\0'; ++p) {
    if (cur->next[(unsigned char) *p] == NULL) {
      return false;
    }
    cur = cur->next[(unsigned char) *p];
  }
  if (!cur->is_word) {
    return NULL;
  }
  return cur->data;
}

static bool is_node_prefix(TrieNode *node) {
  bool is_prefix = false;
  for (int i = 0; i < 256; ++i) {
    if (node->next[i] != NULL) {
      is_prefix = true;
    }
  }
  return is_prefix;
}

static int remove_key_internal(Trie *trie, TrieNode *node, char *key) {
  if (node == NULL) {
    return 0;
  }
  if (key == NULL || *key == '\0') {
    // this is the end of key
    if (!node->is_word) {
      // word not found
      return 0;
    } else {
      // find the key
      // is the key prefix of other key
      trie->size--;
      bool is_prefix = is_node_prefix(node);
      if (is_prefix) {
        // if is a prefix, just change the flag
        node->is_word = false;
        return 2;
      }
      // not a prefix, need to free
      free(node);
      return 1;
    }
  }
  // still need to check forward
  int ret = remove_key_internal(trie, node->next[(unsigned char) *key], key + 1);
  if (ret == 0) {
    // not found ,return
    return 0;
  } else if (ret == 2) {
    // found but is a prefix, changed flag. this node is also a prefix
    return 2;
  } else {
    // find and removed.
    node->next[(unsigned char) *key] = NULL;
    bool is_prefix = is_node_prefix(node);
    if (node->is_word || is_prefix || trie->root == node) {
      // this node is a word or prefix or the root.
      return 2;
    } else {
      free(node);
      return 1;
    }
  }
}

static int remove0_trie(Trie *trie) {
  if (trie->root->next[0] != NULL) {
    free(trie->root->next[0]);
    trie->root->next[0] = NULL;
    trie->size--;
    return 1;
  }
  return 0;
}

int remove_key_trie(Trie *trie, char *key) {
  if(*key=='\0') return remove0_trie(trie);
  TrieNode *start = trie->root;
  int ret = remove_key_internal(trie, start, key);
  return ret == 0 ? 0 : 1;
}

unsigned int size_of_trie(Trie *trie) {
  return trie->size;
}

static void free_trie_node(TrieNode *node) {
  if (node == NULL) return;
  for (int i = 0; i < 256; ++i) {
    free_trie_node(node->next[i]);
    node->next[i] = NULL;
  }
  free(node);
}
void free_trie(Trie *trie) {
  TrieNode *cur = trie->root;
  free_trie_node(cur);
  free(trie);
}

static TrieNode *new_trie_node() {
  TrieNode *node = calloc(1, sizeof(TrieNode));
  if (node == NULL) return NULL;
  return node;
}
