//
// Created by ran on 2024/1/13.
//
#include "hashtable/hash_set.h"
#include "tree/rb_tree.h"
#include <stdlib.h>
#include <stdio.h>


/**
 *  if hash conflict number is greater than this threshold, it will turn to red black tree.
 */
#define LIST_RBT_THRESHOLD 8

/**
 *  threshold of Hashset is the biggest number to hold the entry。threshold = bucket_size * Load_factor.
 *  if the number of entry is bigger then threshold, Hashset will resize.
 */
#define DEFAULT_LOAD_FACTOR 0.75

#define PRIME_INDEXES 25

/**
 * This is a set of good hash string_intern prime numbers, from:
 *   https://planetmath.org/goodHashsetprimes
 */
static const unsigned int hash_set_primes[] =
    {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
     49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469,
     12582917, 25165843, 50331653, 100663319, 201326611, 402653189,
     805306457, 1610612741};

typedef enum {
  NODE_LINKED_LIST, NODE_RED_BLACK_TREE
} NodeType;

struct KEntry {
  HashsetKey key;
  struct KEntry *next;
  unsigned int hash_code;
};

struct SetNode {
  NodeType type;
  unsigned int node_size;
  union {
    KEntry *linked_list;
    RBTree *rb_tree;
  } node;
};

struct Hashset {
  SetNode **bucket;
  unsigned int bucket_capacity;
  unsigned int prime_index;
  unsigned int entry_size;
  unsigned int threshold;
  float load_factor;

  HashsetHashFunc hash_func;
  HashsetEqualsFunc equals_func;
  HashsetKeyFreeFunc key_free_func;
};

struct HashsetIterator {
  Hashset *hashset;
  int current_bucket_index;
  NodeType current_node_type;
  union {
    KEntry *current_node;
    RBTreeIterator *rb_iter;
  };
};

static unsigned int hash(HashsetKey key, HashsetHashFunc hash_func);
static void resize(Hashset *Hashset);
static int insert_entry(SetNode *node,
                        HashsetEqualsFunc equals_func,
                        unsigned int hash_code,
                        HashsetKey key,
                        HashsetKeyFreeFunc free_func);
static int node_convert(HashsetEqualsFunc equals_func, SetNode *node);
static KEntry *search_entry_list(KEntry *head, HashsetKey key, HashsetEqualsFunc equals_func);
static void prepend_entry(SetNode *node, KEntry *entry);
static void free_bucket(SetNode **bucket, unsigned int size, HashsetKeyFreeFunc key_free_func, int clear_entry);
static unsigned int free_node(SetNode *node, HashsetKeyFreeFunc key_free_func, int clear_entry);
static int transfer_nodes(SetNode **new_bucket,
                          unsigned new_bucket_capacity,
                          SetNode *old_node,
                          HashsetEqualsFunc equals_func);
static int insert_entry_to_list(SetNode *node, KEntry *entry, HashsetEqualsFunc equals_func);
static int insert_entry_to_tree(SetNode *node, KEntry *entry);
static unsigned int free_list_node(SetNode *node, HashsetKeyFreeFunc key_free_func);
static unsigned int free_tree_node(SetNode *node, HashsetKeyFreeFunc key_free_func, int clear_entry);
static KEntry *get_entry_hash_set(Hashset *Hashset, HashsetKey key);
Hashset *copy_of_hashset(Hashset *hashset);

Hashset *new_hash_set(HashsetHashFunc hash_func, HashsetEqualsFunc equals_func) {
  Hashset *hashset = malloc(sizeof(Hashset));
  if (hashset == NULL) return NULL;
  hashset->hash_func = hash_func;
  hashset->equals_func = equals_func;
  hashset->load_factor = DEFAULT_LOAD_FACTOR;
  hashset->entry_size = 0;
  hashset->prime_index = 0;
  hashset->bucket_capacity = hash_set_primes[0];
  hashset->threshold = (unsigned int) (1.0 * hashset->bucket_capacity * hashset->load_factor);
  hashset->bucket = NULL;
  hashset->key_free_func = NULL;
  return hashset;
}

void register_hashset_free_functions(Hashset *hashset, HashsetKeyFreeFunc key_free_func) {
  hashset->key_free_func = key_free_func;
}

HashsetKeyFreeFunc get_hashset_free_function(Hashset *hashset) {
  return hashset->key_free_func;
}

int put_hash_set(Hashset *hashset, HashsetKey key) {
  return put_free_new_key_hash_set(hashset, key, NULL);
}

int put_free_new_key_hash_set(Hashset *hashset, HashsetKey key, HashsetKeyFreeFunc free_func) {
  if (hashset->bucket == NULL) {
    // create a new bucket if its NULL
    SetNode **nodes = calloc(hashset->bucket_capacity, sizeof(SetNode *));
    // +424 = 490
    if (nodes == NULL) return 0;
    hashset->bucket = nodes;
  }

  // find index
  unsigned int h = hash(key, hashset->hash_func);
  unsigned index = h % hashset->bucket_capacity;
  SetNode *node;
  if (hashset->bucket[index] == NULL) {
    // create node
    node = malloc(sizeof(SetNode));
    //+16 = 506
    if (node == NULL) return 0;
    node->type = NODE_LINKED_LIST;
    node->node_size = 0;
    node->node.linked_list = NULL;
    hashset->bucket[index] = node;
  } else {
    node = hashset->bucket[index];
  }
  int ret = insert_entry(node, hashset->equals_func, h, key, free_func);
  // 538
  if (ret == 1) hashset->entry_size++;
  if (node->type == NODE_LINKED_LIST && node->node_size > LIST_RBT_THRESHOLD) {
    node_convert(hashset->equals_func, node);
  }
  if (hashset->entry_size > hashset->threshold) {
    // resize
    resize(hashset);
  }
  return ret;
}

static unsigned int hash(HashsetKey key, HashsetHashFunc hash_func) {
  // the "NULL" key has 0 as hash, so its always at the bucket of index 0;
  return key == NULL ? 0 : hash_func(key);
}

static int transfer_nodes(SetNode **new_bucket,
                          unsigned new_bucket_capacity,
                          SetNode *old_node,
                          HashsetEqualsFunc equals_func) {

  if (old_node->type == NODE_LINKED_LIST) {
    // iterate entry
    unsigned int node_size = old_node->node_size;
    KEntry *entries[node_size];
    KEntry *head = old_node->node.linked_list;
    int i = 0;
    while (head != NULL) {
      entries[i++] = head;
      head = head->next;
    }
    // iterate entries
    for (int j = 0; j < node_size; ++j) {
      KEntry *entry = entries[j];
      entry->next = NULL;
      unsigned index = entries[j]->hash_code % new_bucket_capacity;
      if (new_bucket[index] == NULL) {
        // create node
        SetNode *node = malloc(sizeof(SetNode));
        if (node == NULL) return 0;
        node->type = NODE_LINKED_LIST;
        node->node_size = 0;
        node->node.linked_list = NULL;
        new_bucket[index] = node;
      }
      SetNode *node = new_bucket[index];
      // move entry to this node
      if (node->type == NODE_LINKED_LIST) {
        insert_entry_to_list(node, entry, equals_func);
      } else {
        insert_entry_to_tree(node, entry);
      }
      if (node->type == NODE_LINKED_LIST && node->node_size > LIST_RBT_THRESHOLD) {
        node_convert(equals_func, node);
      }
    }
  } else {
    RBTree *rb_tree = old_node->node.rb_tree;
    // iterate rb tree
    RBTreeIterator *iter = rb_tree_iterator(rb_tree);
    while (has_next_rbt_entry(iter)) {
      RBNodeEntry entry = next_rbt_entry(iter);
      KEntry *kv_entry = entry.value;
      kv_entry->next = NULL;
      unsigned index = kv_entry->hash_code % new_bucket_capacity;
      if (new_bucket[index] == NULL) {
        // create node
        SetNode *node = malloc(sizeof(SetNode));
        if (node == NULL) return 0;
        node->type = NODE_LINKED_LIST;
        node->node_size = 0;
        node->node.linked_list = NULL;
        new_bucket[index] = node;
      }
      SetNode *node = new_bucket[index];
      // move entry to this node
      if (node->type == NODE_LINKED_LIST) {
        insert_entry_to_list(node, kv_entry, equals_func);
      } else {
        insert_entry_to_tree(node, kv_entry);
      }
      if (node->type == NODE_LINKED_LIST && node->node_size > LIST_RBT_THRESHOLD) {
        node_convert(equals_func, node);
      }
    }
    free_rb_tree_iterator(iter);
  }
  return 1;
}
static unsigned int free_list_node(SetNode *node, HashsetKeyFreeFunc key_free_func) {
  KEntry *head = node->node.linked_list;
  while (head != NULL) {
    KEntry *next = head->next;
    if (key_free_func != NULL) {
      key_free_func(head->key);
    }
    free(head);
    head = next;
  }
  unsigned int size = node->node_size;
  free(node);
  return size;
}
static unsigned int free_tree_node(SetNode *node, HashsetKeyFreeFunc key_free_func, int clear_entry) {
  RBTreeIterator *iter = rb_tree_iterator(node->node.rb_tree);
  // +72 = 698
  while (has_next_rbt_entry(iter)) {
    RBTreeNode *tree_node = next_rbt_node(iter);
    KEntry *kv_entry = value_of_rb_node(tree_node);
    if (key_free_func != NULL) {
      key_free_func(kv_entry->key);
    }

    if (clear_entry) {
      free(kv_entry);
    }
    free(tree_node);
    // - 56 = 640
  }
  free(node->node.rb_tree);
  unsigned int size = node->node_size;
  free(node);
  free_rb_tree_iterator(iter);
  return size;
}
static unsigned int free_node(SetNode *node, HashsetKeyFreeFunc key_free_func, int clear_entry) {
  if (node == NULL) return 0;
  if (node->type == NODE_LINKED_LIST) {
    return free_list_node(node, key_free_func);
  } else {
    // iterate rb tree
    // printf("free tree node, size: %u\n", node->node_size);
    return free_tree_node(node, key_free_func, clear_entry);
  }
}

static void free_bucket(SetNode **bucket,
                        unsigned int size,
                        HashsetKeyFreeFunc key_free_func,
                        int clear_entry) {
  if (bucket == NULL) return;
  unsigned int removed_entry = 0;
  for (unsigned int i = 0; i < size; i++) {
    removed_entry += free_node(bucket[i], key_free_func, clear_entry);
    //free(bucket[i]);
  }
  free(bucket);
}

static void free_old_bucket_nodes_after_resize(SetNode **bucket,
                                               unsigned int size) {
  for (unsigned int i = 0; i < size; i++) {
    if (bucket[i] == NULL) continue;
    if (bucket[i]->type == NODE_RED_BLACK_TREE) {
      free_tree_node(bucket[i], NULL, 0);
    } else {
      free(bucket[i]);
    }

  }
  free(bucket);
}

static void resize(Hashset *hashset) {
  if (hashset->prime_index >= PRIME_INDEXES) return;
  unsigned int new_cap = hash_set_primes[hashset->prime_index + 1];
  SetNode **new_bucket = calloc(new_cap, sizeof(SetNode *));
  if (new_bucket == NULL) return;

  int ret = 0;
  for (unsigned int i = 0; i < hashset->bucket_capacity; ++i) {
    SetNode *old_node = hashset->bucket[i];
    if (old_node == NULL)continue;
    ret = transfer_nodes(new_bucket, new_cap, old_node, hashset->equals_func);
    if (ret == 0)break;
  }
  if (ret != 0) {
    free_old_bucket_nodes_after_resize(hashset->bucket, hashset->bucket_capacity);
//    free_bucket(Hashset->bucket, Hashset->bucket_capacity, Hashset->key_free_func, Hashset->value_free_func);
    hashset->bucket = new_bucket;
    hashset->prime_index++;
    hashset->bucket_capacity = new_cap;
    hashset->threshold = (unsigned int) (1.0 * hashset->bucket_capacity * hashset->load_factor);
  } else {
    // rollback
    free_bucket(new_bucket, new_cap, hashset->key_free_func, 0);
  }
}

static KEntry *search_entry_list(KEntry *head, HashsetKey key, HashsetEqualsFunc equals_func) {
  KEntry *cur = head;
  while (cur != NULL) {
    if ((key == NULL && cur->key == NULL) || (key != NULL && cur->key != NULL) && (equals_func(cur->key, key) == 0)) {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

static KEntry *remove_entry_list(KEntry **head,
                                 HashsetKey key,
                                 HashsetEqualsFunc equals_func,
                                 int *removed) {
  KEntry *cur = *head;
  KEntry *prev = NULL;
  int flag = 0;
  while (cur != NULL) {
    if ((key == NULL && cur->key == NULL) || (key != NULL && cur->key != NULL) && (equals_func(cur->key, key) == 0)) {
      flag = 1;
      break;
    }
    prev = cur;
    cur = cur->next;
  }
  if (flag == 0) {
    *removed = 0;
    return NULL;
  }
  *removed = 1;
  if (prev == NULL) {
    *head = cur->next;
  } else {
    prev->next = cur->next;
  }
  return cur;
}

static void prepend_entry(SetNode *node, KEntry *entry) {
  KEntry *entry_head = node->node.linked_list;
  if (entry_head == NULL) {
    node->node.linked_list = entry;
  } else {
    entry->next = node->node.linked_list;
    node->node.linked_list = entry;
  }
}

static int insert_entry_to_list(SetNode *node, KEntry *entry, HashsetEqualsFunc equals_func) {
  int ret;
  KEntry *linked_list = node->node.linked_list;
  KEntry *exists = search_entry_list(linked_list, entry->key, equals_func);
  if (exists == NULL) {
    // insert new entry
    prepend_entry(node, entry);
    ret = 1;
    node->node_size++;
  } else {
    ret = -1;
  }
  return ret;
}

static int insert_entry_to_tree(SetNode *node, KEntry *entry) {
  int ret;
  RBTree *rb_tree = node->node.rb_tree;
  RBTreeNode *tn = get_node_rb_tree(rb_tree, entry->key);
  if (tn != NULL) {
    // exists replace
    // KEntry *e = value_of_rb_node(tn);
    ret = -1;
  } else {
    // insert new entry
    put_rb_tree(rb_tree, entry->key, entry);
    ret = 1;
    node->node_size++;
  }
  return ret;
}

static int insert_entry(SetNode *node,
                        HashsetEqualsFunc equals_func,
                        unsigned int hash_code,
                        HashsetKey key,
                        HashsetKeyFreeFunc free_func) {
  int ret = 0;
  if (node->type == NODE_LINKED_LIST) {
    KEntry *linked_list = node->node.linked_list;
    KEntry *exists = search_entry_list(linked_list, key, equals_func);
    if (exists == NULL) {
      // insert new entry
      KEntry *entry = malloc(sizeof(KEntry));
      // + 32 = 538
      if (entry == NULL) return ret;
      entry->key = key;
      entry->hash_code = hash_code;
      entry->next = NULL;
      prepend_entry(node, entry);
      ret = 1;
      node->node_size++;
    } else {
      if (free_func) {
        free_func(key);
      }
      // exists->key = key;
      ret = -1;
    }
  } else {
    RBTree *rb_tree = node->node.rb_tree;
    RBTreeNode *tn = get_node_rb_tree(rb_tree, key);
    if (tn != NULL) {
      // exists replace
      if (free_func) {
        free_func(key);
      }
      ret = -1;
    } else {
      // insert new entry
      KEntry *entry = malloc(sizeof(KEntry));
      if (entry == NULL) return ret;
      entry->key = key;
      entry->hash_code = hash_code;
      entry->next = NULL;
      put_rb_tree(rb_tree, key, entry);
      ret = 1;
      node->node_size++;
    }
  }
  return ret;
}

static int node_convert(HashsetEqualsFunc equals_func, SetNode *node) {
  RBTree *rb_tree = new_rb_tree(equals_func);
  // + 32 = 570
  if (rb_tree == NULL) return 0;
  KEntry *head = node->node.linked_list;

  KEntry *cur = head;

  while (cur != NULL) {
    RBTreeNode *new_node = put_rb_tree(rb_tree, cur->key, cur);
    // +56 = 626
    if (new_node == NULL) {
      free_rb_tree(rb_tree);
      return 0;
    }
    cur = cur->next;
  }
  node->type = NODE_RED_BLACK_TREE;
  node->node.rb_tree = rb_tree;
  return 1;
}

void free_hash_set(Hashset *hashset) {
  if (hashset == NULL) return;
  free_bucket(hashset->bucket, hashset->bucket_capacity, hashset->key_free_func, 1);
  free(hashset);
}

unsigned int size_of_hash_set(Hashset *hashset) {
  return hashset->entry_size;
}

static KEntry *get_entry_hash_set(Hashset *hashset, HashsetKey key) {
  if (hashset->bucket == NULL) {
    return NULL;
  }
  // find index
  unsigned int h = hash(key, hashset->hash_func);
  unsigned index = h % hashset->bucket_capacity;
  SetNode *node = hashset->bucket[index];
  if (node == NULL) return NULL;
  if (node->type == NODE_LINKED_LIST) {
    KEntry *e = search_entry_list(node->node.linked_list, key, hashset->equals_func);
    return e;
  } else {
    KEntry *e = (KEntry *) get_value_rb_tree(node->node.rb_tree, key);
    return e;
  }
}

int contains_in_hash_set(Hashset *Hashset, HashsetKey key) {
  KEntry *entry = get_entry_hash_set(Hashset, key);
  return entry != NULL ? 1 : 0;
}

int remove_hash_set(Hashset *Hashset, HashsetKey key) {
  int flag;
  remove_with_flag_hash_set(Hashset, key, &flag);
  return flag;
}

HashsetKey get_key_in_hash_set(Hashset *hashset, HashsetKey key) {
  KEntry *entry = get_entry_hash_set(hashset, key);
  return entry != NULL ? entry->key : NULL;
}

void remove_with_flag_hash_set(Hashset *hashset, HashsetKey key, int *flag) {
  if (hashset->bucket == NULL) {
    *flag = 0;
    return;
  }
  // find index
  unsigned int h = hash(key, hashset->hash_func);
  unsigned index = h % hashset->bucket_capacity;
  SetNode *node = hashset->bucket[index];
  if (node == NULL) return;
  *flag = 0;
  KEntry *removed = NULL;
  if (node->type == NODE_LINKED_LIST) {
    removed = remove_entry_list(&node->node.linked_list, key, hashset->equals_func, flag);
  } else {
    removed = (KEntry *) remove_with_flag_rb_tree(node->node.rb_tree, key, flag);
  }
  if (*flag == 1) {
    node->node_size--;
    hashset->entry_size--;
    if (hashset->key_free_func != NULL) {
      hashset->key_free_func(removed->key);
    }
    free(removed);
  }
}

void set_load_factor_hash_set(Hashset *Hashset, float load_factor) {
  Hashset->load_factor = load_factor;
  Hashset->threshold = (unsigned int) (1.0 * Hashset->bucket_capacity * Hashset->load_factor);
}

HashsetEntrySet *hashset_entry_set(Hashset *hashset) {
  HashsetEntrySet *entry_set = malloc(sizeof(HashsetEntrySet));
  if (entry_set == NULL) return NULL;
  unsigned int table_size = size_of_hash_set(hashset);
  entry_set->size = table_size;
  KEntry **kv_entry = calloc(table_size, sizeof(KEntry *));
  if (kv_entry == NULL) {
    free(entry_set);
    return NULL;
  }
  if (hashset->entry_size == 0) {
    entry_set->entry_set = kv_entry;
    return entry_set;
  }
  unsigned int s = 0;

  HashsetIterator *iter = hashset_iterator(hashset);
  while (hashset_iter_has_next(iter)) {
    KEntry *entry = hashset_next_entry(iter);
    kv_entry[s++] = entry;
  }
  entry_set->entry_set = kv_entry;
  free_hashset_iter(iter);
  return entry_set;


//  for (int i = 0; i < hashset->bucket_capacity; ++i) {
//    SetNode *node = hashset->bucket[i];
//    if (node == NULL) continue;
//    if (node->type == NODE_LINKED_LIST) {
//      KEntry *head = node->node.linked_list;
//      while (head != NULL) {
//        kv_entry[s++] = head;
//        head = head->next;
//      }
//    } else {
//      RBTree *tree = node->node.rb_tree;
//      RBTreeIterator *iter = rb_tree_iterator(tree);
//      while (has_next_rbt_entry(iter)) {
//        RBTreeNode *entry = next_rbt_node(iter);
//        kv_entry[s++] = value_of_rb_node(entry);
//      }
//      free_rb_tree_iterator(iter);
//    }
//  }
//  entry_set->entry_set = kv_entry;
//  return entry_set;
}

void free_hashset_entry_set(HashsetEntrySet *hashset_entry_set) {
  if (hashset_entry_set != NULL) {
    free(hashset_entry_set->entry_set);
    free(hashset_entry_set);
  }
}

HashsetKey set_entry_key(KEntry *entry) {
  return entry->key;
}

Hashset *copy_of_hashset(Hashset *hashset) {
  if (hashset == NULL) return NULL;
  Hashset *ret = new_hash_set(hashset->hash_func, hashset->equals_func);
  if (ret == NULL) return NULL;
  SetNode **bucket = calloc(hashset->bucket_capacity, sizeof(SetNode *));
  if (bucket == NULL) {
    free(ret);
    return NULL;
  }
  ret->bucket = bucket;
  for (int i = 0; i < hashset->bucket_capacity; ++i) {
    SetNode *node = hashset->bucket[i];
    if (node == NULL) continue;

    SetNode *copy_node = malloc(sizeof(SetNode));
    if (copy_node == NULL) {
      goto copy_fail;
    }
    ret->bucket[i] = copy_node;

    if (node->type == NODE_LINKED_LIST) {
      KEntry *head = node->node.linked_list;
      copy_node->type = NODE_LINKED_LIST;
      copy_node->node_size = node->node_size;
      copy_node->node.linked_list = NULL;
      KEntry *copy_head = NULL;
      KEntry *prev = NULL;
      while (head != NULL) {
        KEntry *copy = malloc(sizeof(KEntry));
        if (copy == NULL) {
          goto copy_fail;
        }
        copy->key = head->key;
        copy->hash_code = head->hash_code;
        copy->next = NULL;
        if (copy_head == NULL) copy_head = copy;
        if (prev != NULL) prev->next = copy;
        prev = copy;
        head = head->next;
      }
      copy_node->node.linked_list = copy_head;
    } else {
      RBTree *tree = node->node.rb_tree;
      RBTree *copy_tree = new_rb_tree(hashset->equals_func);
      if (copy_tree == NULL) {
        goto copy_fail;
      }
      copy_node->node.rb_tree = copy_tree;
      RBTreeIterator *iter = rb_tree_iterator(tree);
      while (has_next_rbt_entry(iter)) {
        RBTreeNode *entry = next_rbt_node(iter);
        KEntry *ke = (KEntry *) value_of_rb_node(entry);
        KEntry *new_node = malloc(sizeof(KEntry));
        if (new_node == NULL) {
          goto copy_fail;
        }
        new_node->key = ke->key;
        new_node->hash_code = ke->hash_code;
        RBTreeNode *put = put_rb_tree(copy_tree, new_node->key, new_node);
        if (put == NULL) {
          goto copy_fail;
        }
      }
      free_rb_tree_iterator(iter);
    }
  }
  ret->entry_size = hashset->entry_size;
  return ret;

  copy_fail:
  free_hash_set(ret);
  return NULL;
}

/**
 * Union two sets
 *
 * @param hashset1
 * @param hashset2
 * @return
 */
Hashset *union_hashset(Hashset *hashset1, Hashset *hashset2) {
  if (hashset1 == NULL) return hashset2;
  if (hashset2 == NULL) return hashset1;
  unsigned int size1 = size_of_hash_set(hashset1);
  unsigned int size2 = size_of_hash_set(hashset2);
  Hashset *large = size1 > size2 ? hashset1 : hashset2;
  Hashset *little = size1 > size2 ? hashset2 : hashset1;
  Hashset *ret = copy_of_hashset(large);
  HashsetEntrySet *es = hashset_entry_set(little);
  KEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    KEntry *e = entries[j];
    HashsetKey key = set_entry_key(e);
    put_hash_set(ret, key);
  }
  free_hashset_entry_set(es);
  return ret;
}

/**
 * Perform an intersection of two sets.
 *
 * @param hashset1
 * @param hashset2
 * @return
 */
Hashset *intersect_hashset(Hashset *hashset1, Hashset *hashset2) {
  if (hashset1 == NULL || hashset2 == NULL) return NULL;

  unsigned int size1 = size_of_hash_set(hashset1);
  unsigned int size2 = size_of_hash_set(hashset2);
  Hashset *large = size1 > size2 ? hashset1 : hashset2;
  Hashset *little = size1 > size2 ? hashset2 : hashset1;
  Hashset *ret = copy_of_hashset(little);
  HashsetEntrySet *es = hashset_entry_set(ret);
  KEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    KEntry *e = entries[j];
    HashsetKey key = set_entry_key(e);
    if (!contains_in_hash_set(large, (char *) key)) {
      remove_hash_set(ret, (char *) key);
    }
  }
  free_hashset_entry_set(es);
  return ret;

}

Hashset *difference_hashset(Hashset *hashset1, Hashset *hashset2) {
  if (hashset1 == NULL || hashset2 == NULL) return hashset1;
  Hashset *ret = copy_of_hashset(hashset1);
  HashsetEntrySet *es = hashset_entry_set(hashset1);
  KEntry **entries = es->entry_set;
  for (int j = 0; j < es->size; ++j) {
    KEntry *e = entries[j];
    HashsetKey key = set_entry_key(e);
    if (contains_in_hash_set(hashset2, key)) {
      remove_hash_set(ret, key);
    }
  }
  free_hashset_entry_set(es);
  return ret;
}

HashsetIterator *hashset_iterator(Hashset *hashset) {
  HashsetIterator *iterator = malloc(sizeof(HashsetIterator));
  if (!iterator) return NULL;
  iterator->hashset = hashset;
  iterator->current_bucket_index = -1;
  iterator->current_node_type = NODE_LINKED_LIST;
  iterator->current_node = NULL;
  if (hashset->bucket == NULL) {
    // no elements yet
    return iterator;
  } else {
    // find first no-empty bucket
    for (int i = 0; i < hashset->bucket_capacity; ++i) {
      SetNode *node = hashset->bucket[i];
      if (node != NULL && node->node_size > 0) {
        iterator->current_bucket_index = i;
        iterator->current_node_type = node->type;
        if (node->type == NODE_LINKED_LIST) {
          iterator->current_node = node->node.linked_list;
        } else {
          iterator->rb_iter = rb_tree_iterator(node->node.rb_tree);
        }
        break;
      }
    }
    return iterator;
  }
}

int hashset_iter_has_next(HashsetIterator *iter) {
  if (iter->current_bucket_index == -1) return 0;
  if (iter->current_node_type == NODE_LINKED_LIST) {
    return iter->current_node == NULL ? 0 : 1;
  } else {
    return has_next_rbt_entry(iter->rb_iter);
  }
}

KEntry *hashset_next_entry(HashsetIterator *iter) {
  KEntry *ret;
  if (iter->current_node_type == NODE_LINKED_LIST) {
    ret = iter->current_node;
    iter->current_node = iter->current_node->next;
  } else {
    RBTreeNode *entry = next_rbt_node(iter->rb_iter);
    ret = value_of_rb_node(entry);
  }
  // move to next entry
  int need_move_bucket = 0;
  if (iter->current_node_type == NODE_LINKED_LIST) {
    // current node null
    if (iter->current_node == NULL) {
      need_move_bucket = 1;
    }
  } else {
    if (!has_next_rbt_entry(iter->rb_iter)) {
      free_rb_tree_iterator(iter->rb_iter);
      iter->rb_iter = NULL;
      need_move_bucket = 1;
    }
  }
  if (need_move_bucket) {
    // move forward
    int current_bucket_index = iter->current_bucket_index;
    iter->current_bucket_index = -1;
    for (int i = current_bucket_index + 1; i < iter->hashset->bucket_capacity; ++i) {
      SetNode *node = iter->hashset->bucket[i];
      if (node != NULL && node->node_size > 0) {
        iter->current_bucket_index = i;
        iter->current_node_type = node->type;
        if (node->type == NODE_LINKED_LIST) {
          iter->current_node = node->node.linked_list;
        } else {
          iter->rb_iter = rb_tree_iterator(node->node.rb_tree);
        }
        break;
      }
    }
  }
  return ret;
}

void free_hashset_iter(HashsetIterator *iter) {
  if (iter) {
    if (iter->current_bucket_index != -1 && iter->current_node_type == NODE_RED_BLACK_TREE) {
      free_rb_tree_iterator(iter->rb_iter);
    }
    free(iter);
  }
}