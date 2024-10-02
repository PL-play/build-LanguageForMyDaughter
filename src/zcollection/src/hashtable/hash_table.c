//
// Created by ran on 2024/1/10.
//
#include "hashtable/hash_table.h"
#include "tree/rb_tree.h"
#include <stdlib.h>


/**
 *  if hash conflict number is greater than this threshold, it will turn to red black tree.
 */
#define LIST_RBT_THRESHOLD 8

/**
 *  threshold of hashtable is the biggest number to hold the entry。threshold = bucket_size * Load_factor.
 *  if the number of entry is bigger then threshold, hashtable will resize.
 */
#define DEFAULT_LOAD_FACTOR 0.75

#define PRIME_INDEXES 25

/**
 * This is a set of good hash string_intern prime numbers, from:
 *   https://planetmath.org/goodhashtableprimes
 */
static const unsigned int hash_table_primes[] =
    {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
     49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469,
     12582917, 25165843, 50331653, 100663319, 201326611, 402653189,
     805306457, 1610612741};

typedef enum {
  NODE_LINKED_LIST, NODE_RED_BLACK_TREE
} NodeType;

struct KVEntry {
  HashtableKey key;
  HashtableValue value;
  struct KVEntry *next;
  unsigned int hash_code;
};

struct TableNode {
  NodeType type;
  unsigned int node_size;
  union {
    KVEntry *linked_list;
    RBTree *rb_tree;
  } node;
};

struct Hashtable {
  TableNode **bucket;
  unsigned int bucket_capacity;
  unsigned int prime_index;
  unsigned int entry_size;
  unsigned int threshold;
  float load_factor;

  HashtableHashFunc hash_func;
  HashtableEqualsFunc equals_func;
  HashtableKeyFreeFunc key_free_func;
  HashtableValueFreeFunc value_free_func;
};

struct HashtableIterator {
  Hashtable *hashtable;
  int current_bucket_index;
  NodeType current_node_type;
  union {
    KVEntry *current_node;
    RBTreeIterator *rb_iter;
  };
};

static unsigned int hash(HashtableKey key, HashtableHashFunc hash_func);
static void resize(Hashtable *hashtable);
static int insert_entry(TableNode *node,
                        HashtableEqualsFunc equals_func,
                        unsigned int hash_code,
                        HashtableKey key,
                        HashtableValue value,
                        HashtableValueFreeFunc free_func);
static int node_convert(HashtableEqualsFunc equals_func, TableNode *node);
static KVEntry *search_entry_list(KVEntry *head, HashtableKey key, HashtableEqualsFunc equals_func);
static void prepend_entry(TableNode *node, KVEntry *entry);
static void free_bucket(TableNode **bucket, unsigned int size,
                        HashtableKeyFreeFunc key_free_func,
                        HashtableValueFreeFunc value_free_func, int clear_entry);
static unsigned int free_node(TableNode *node, HashtableKeyFreeFunc key_free_func,
                              HashtableValueFreeFunc value_free_func, int clear_entry);
static int transfer_nodes(TableNode **new_bucket,
                          unsigned new_bucket_capacity,
                          TableNode *old_node,
                          HashtableEqualsFunc equals_func);
static int insert_entry_to_list(TableNode *node, KVEntry *entry, HashtableEqualsFunc equals_func);
static int insert_entry_to_tree(TableNode *node, KVEntry *entry);
static unsigned int free_list_node(TableNode *node, HashtableKeyFreeFunc key_free_func,
                                   HashtableValueFreeFunc value_free_func);
static unsigned int free_tree_node(TableNode *node, HashtableKeyFreeFunc key_free_func,
                                   HashtableValueFreeFunc value_free_func, int clear_entry);
static KVEntry *get_entry_hash_table(Hashtable *hashtable, HashtableKey key);

Hashtable *new_hash_table(HashtableHashFunc hash_func, HashtableEqualsFunc equals_func) {
  Hashtable *hashtable = malloc(sizeof(Hashtable));
  if (hashtable == NULL) return NULL;
  hashtable->hash_func = hash_func;
  hashtable->equals_func = equals_func;
  hashtable->load_factor = DEFAULT_LOAD_FACTOR;
  hashtable->entry_size = 0;
  hashtable->prime_index = 0;
  hashtable->bucket_capacity = hash_table_primes[0];
  hashtable->threshold = (unsigned int) (1.0 * hashtable->bucket_capacity * hashtable->load_factor);
  hashtable->bucket = NULL;
  hashtable->key_free_func = NULL;
  hashtable->value_free_func = NULL;
  return hashtable;
}

void register_hashtable_free_functions(Hashtable *hashtable,
                                       HashtableKeyFreeFunc key_free_func,
                                       HashtableValueFreeFunc value_free_func) {
  hashtable->key_free_func = key_free_func;
  hashtable->value_free_func = value_free_func;
}

int put_hash_table(Hashtable *hashtable, HashtableKey key, HashtableValue value) {
  return put_free_exist_hash_table(hashtable, key, value, NULL);
}

int put_free_exist_hash_table(Hashtable *hashtable,
                              HashtableKey key,
                              HashtableValue value,
                              void(*free_func)(HashtableValue)) {
  if (hashtable->bucket == NULL) {
    // create a new bucket if its NULL
    TableNode **nodes = calloc(hashtable->bucket_capacity, sizeof(TableNode *));
    // +424 = 490
    if (nodes == NULL) return 0;
    hashtable->bucket = nodes;
  }

  // find index
  unsigned int h = hash(key, hashtable->hash_func);
  unsigned index = h % hashtable->bucket_capacity;
  TableNode *node;
  if (hashtable->bucket[index] == NULL) {
    // create node
    node = malloc(sizeof(TableNode));
    //+16 = 506
    if (node == NULL) return 0;
    node->type = NODE_LINKED_LIST;
    node->node_size = 0;
    node->node.linked_list = NULL;
    hashtable->bucket[index] = node;
  } else {
    node = hashtable->bucket[index];
  }
  int ret = insert_entry(node, hashtable->equals_func, h, key, value, free_func);
  // 538
  if (ret == 1) hashtable->entry_size++;
  if (node->type == NODE_LINKED_LIST && node->node_size > LIST_RBT_THRESHOLD) {
    node_convert(hashtable->equals_func, node);
  }
  if (hashtable->entry_size > hashtable->threshold) {
    // resize
    resize(hashtable);
  }
  return ret;
}

static unsigned int hash(HashtableKey key, HashtableHashFunc hash_func) {
  // the "NULL" key has 0 as hash, so its always at the bucket of index 0;
  return key == NULL ? 0 : hash_func(key);
}

static int transfer_nodes(TableNode **new_bucket,
                          unsigned new_bucket_capacity,
                          TableNode *old_node,
                          HashtableEqualsFunc equals_func) {

  if (old_node->type == NODE_LINKED_LIST) {
    // iterate entry
    unsigned int node_size = old_node->node_size;
    KVEntry *entries[node_size];
    KVEntry *head = old_node->node.linked_list;
    int i = 0;
    while (head != NULL) {
      entries[i++] = head;
      head = head->next;
    }
    // iterate entries
    for (int j = 0; j < node_size; ++j) {
      KVEntry *entry = entries[j];
      entry->next = NULL;
      unsigned index = entries[j]->hash_code % new_bucket_capacity;
      if (new_bucket[index] == NULL) {
        // create node
        TableNode *node = malloc(sizeof(TableNode));
        if (node == NULL) return 0;
        node->type = NODE_LINKED_LIST;
        node->node_size = 0;
        node->node.linked_list = NULL;
        new_bucket[index] = node;
      }
      TableNode *node = new_bucket[index];
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
      KVEntry *kv_entry = entry.value;
      kv_entry->next = NULL;
      unsigned index = kv_entry->hash_code % new_bucket_capacity;
      if (new_bucket[index] == NULL) {
        // create node
        TableNode *node = malloc(sizeof(TableNode));
        if (node == NULL) return 0;
        node->type = NODE_LINKED_LIST;
        node->node_size = 0;
        node->node.linked_list = NULL;
        new_bucket[index] = node;
      }
      TableNode *node = new_bucket[index];
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
static unsigned int free_list_node(TableNode *node, HashtableKeyFreeFunc key_free_func,
                                   HashtableValueFreeFunc value_free_func) {
  KVEntry *head = node->node.linked_list;
  while (head != NULL) {
    KVEntry *next = head->next;
    if (key_free_func != NULL) {
      key_free_func(head->key);
    }
    if (value_free_func != NULL) {
      value_free_func(head->value);
    }
    free(head);
    head = next;
  }
  unsigned int size = node->node_size;
  free(node);
  return size;
}
static unsigned int free_tree_node(TableNode *node, HashtableKeyFreeFunc key_free_func,
                                   HashtableValueFreeFunc value_free_func, int clear_entry) {
  RBTreeIterator *iter = rb_tree_iterator(node->node.rb_tree);
  // +72 = 698
  while (has_next_rbt_entry(iter)) {
    RBTreeNode *tree_node = next_rbt_node(iter);
    KVEntry *kv_entry = value_of_rb_node(tree_node);
    if (key_free_func != NULL) {
      key_free_func(kv_entry->key);
    }
    if (value_free_func != NULL) {
      value_free_func(kv_entry->value);
      // -2 = 696
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
static unsigned int free_node(TableNode *node, HashtableKeyFreeFunc key_free_func,
                              HashtableValueFreeFunc value_free_func, int clear_entry) {
  if (node == NULL) return 0;
  if (node->type == NODE_LINKED_LIST) {
    return free_list_node(node, key_free_func, value_free_func);
  } else {
    // iterate rb tree
    // printf("free tree node, size: %u\n", node->node_size);
    return free_tree_node(node, key_free_func, value_free_func, clear_entry);
  }
}

static void free_bucket(TableNode **bucket,
                        unsigned int size,
                        HashtableKeyFreeFunc key_free_func,
                        HashtableValueFreeFunc value_free_func, int clear_entry) {
  if (bucket == NULL) return;
  unsigned int removed_entry = 0;
  for (unsigned int i = 0; i < size; i++) {
    removed_entry += free_node(bucket[i], key_free_func, value_free_func, clear_entry);
    //free(bucket[i]);
  }
  free(bucket);
}

static void free_old_bucket_nodes_after_resize(TableNode **bucket,
                                               unsigned int size) {
  for (unsigned int i = 0; i < size; i++) {
    if (bucket[i] == NULL) continue;
    if (bucket[i]->type == NODE_RED_BLACK_TREE) {
      free_tree_node(bucket[i], NULL, NULL, 0);
    } else {
      free(bucket[i]);
    }

  }
  free(bucket);
}

static void resize(Hashtable *hashtable) {
  if (hashtable->prime_index >= PRIME_INDEXES) return;
  unsigned int new_cap = hash_table_primes[hashtable->prime_index + 1];
  TableNode **new_bucket = calloc(new_cap, sizeof(TableNode *));
  if (new_bucket == NULL) return;

  int ret = 0;
  for (unsigned int i = 0; i < hashtable->bucket_capacity; ++i) {
    TableNode *old_node = hashtable->bucket[i];
    if (old_node == NULL)continue;
    ret = transfer_nodes(new_bucket, new_cap, old_node, hashtable->equals_func);
    if (ret == 0)break;
  }
  if (ret != 0) {
    free_old_bucket_nodes_after_resize(hashtable->bucket, hashtable->bucket_capacity);
//    free_bucket(hashtable->bucket, hashtable->bucket_capacity, hashtable->key_free_func, hashtable->value_free_func);
    hashtable->bucket = new_bucket;
    hashtable->prime_index++;
    hashtable->bucket_capacity = new_cap;
    hashtable->threshold = (unsigned int) (1.0 * hashtable->bucket_capacity * hashtable->load_factor);
  } else {
    // rollback
    free_bucket(new_bucket, new_cap, hashtable->key_free_func, hashtable->value_free_func, 0);
  }
}

static KVEntry *search_entry_list(KVEntry *head, HashtableKey key, HashtableEqualsFunc equals_func) {
  KVEntry *cur = head;
  while (cur != NULL) {
    if ((key == NULL && cur->key == NULL) || (key != NULL && cur->key != NULL) && (equals_func(cur->key, key) == 0)) {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

static KVEntry *remove_entry_list(KVEntry **head,
                                  HashtableKey key,
                                  HashtableEqualsFunc equals_func,
                                  int *removed) {
  KVEntry *cur = *head;
  KVEntry *prev = NULL;
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

static void prepend_entry(TableNode *node, KVEntry *entry) {
  KVEntry *entry_head = node->node.linked_list;
  if (entry_head == NULL) {
    node->node.linked_list = entry;
  } else {
    entry->next = node->node.linked_list;
    node->node.linked_list = entry;
  }
}

static int insert_entry_to_list(TableNode *node, KVEntry *entry, HashtableEqualsFunc equals_func) {
  int ret;
  KVEntry *linked_list = node->node.linked_list;
  KVEntry *exists = search_entry_list(linked_list, entry->key, equals_func);
  if (exists == NULL) {
    // insert new entry
    prepend_entry(node, entry);
    ret = 1;
    node->node_size++;
  } else {
    // update value
    exists->value = entry->value;
    ret = -1;
  }
  return ret;
}

static int insert_entry_to_tree(TableNode *node, KVEntry *entry) {
  int ret;
  RBTree *rb_tree = node->node.rb_tree;
  RBTreeNode *tn = get_node_rb_tree(rb_tree, entry->key);
  if (tn != NULL) {
    // exists replace
    KVEntry *e = value_of_rb_node(tn);
    e->value = entry->value;
    ret = -1;
  } else {
    // insert new entry
    put_rb_tree(rb_tree, entry->key, entry);
    ret = 1;
    node->node_size++;
  }
  return ret;
}

static int insert_entry(TableNode *node,
                        HashtableEqualsFunc equals_func,
                        unsigned int hash_code,
                        HashtableKey key,
                        HashtableValue value,
                        HashtableValueFreeFunc free_func) {
  int ret = 0;
  if (node->type == NODE_LINKED_LIST) {
    KVEntry *linked_list = node->node.linked_list;
    KVEntry *exists = search_entry_list(linked_list, key, equals_func);
    if (exists == NULL) {
      // insert new entry
      KVEntry *entry = malloc(sizeof(KVEntry));
      // + 32 = 538
      if (entry == NULL) return ret;
      entry->key = key;
      entry->value = value;
      entry->hash_code = hash_code;
      entry->next = NULL;
      prepend_entry(node, entry);
      ret = 1;
      node->node_size++;
    } else {
      // update value
      if (free_func) {
        free_func(exists->value);
      }
      exists->value = value;
      ret = -1;
    }
  } else {
    RBTree *rb_tree = node->node.rb_tree;
    RBTreeNode *tn = get_node_rb_tree(rb_tree, key);
    if (tn != NULL) {
      // exists replace
      KVEntry *entry = value_of_rb_node(tn);
      if (free_func) {
        free_func(entry->value);
      }
      entry->value = value;
      ret = -1;
    } else {
      // insert new entry
      KVEntry *entry = malloc(sizeof(KVEntry));
      if (entry == NULL) return ret;
      entry->key = key;
      entry->value = value;
      entry->hash_code = hash_code;
      entry->next = NULL;
      put_rb_tree(rb_tree, key, entry);
      ret = 1;
      node->node_size++;
    }
  }
  return ret;
}

static int node_convert(HashtableEqualsFunc equals_func, TableNode *node) {
  RBTree *rb_tree = new_rb_tree(equals_func);
  // + 32 = 570
  if (rb_tree == NULL) return 0;
  KVEntry *head = node->node.linked_list;

  KVEntry *cur = head;

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

void free_hash_table(Hashtable *hashtable) {
  if (hashtable == NULL) return;
  free_bucket(hashtable->bucket, hashtable->bucket_capacity, hashtable->key_free_func, hashtable->value_free_func, 1);
  free(hashtable);
}

unsigned int size_of_hash_table(Hashtable *hashtable) {
  return hashtable->entry_size;
}

static KVEntry *get_entry_hash_table(Hashtable *hashtable, HashtableKey key) {
  if (hashtable->bucket == NULL) {
    return NULL;
  }
  // find index
  unsigned int h = hash(key, hashtable->hash_func);
  unsigned index = h % hashtable->bucket_capacity;
  TableNode *node = hashtable->bucket[index];
  if (node == NULL) return NULL;
  if (node->type == NODE_LINKED_LIST) {
    KVEntry *e = search_entry_list(node->node.linked_list, key, hashtable->equals_func);
    return e;
  } else {
    KVEntry *e = (KVEntry *) get_value_rb_tree(node->node.rb_tree, key);
    return e;
  }
}

int contains_in_hash_table(Hashtable *hashtable, HashtableKey key) {
  KVEntry *entry = get_entry_hash_table(hashtable, key);
  return entry != NULL ? 1 : 0;
}

HashtableValue get_hash_table(Hashtable *hashtable, HashtableKey key) {
  KVEntry *entry = get_entry_hash_table(hashtable, key);
  return entry == NULL ? NULL : entry->value;
}

HashtableValue get_default_hash_table(Hashtable *hashtable, HashtableKey key, HashtableValue default_value) {
  KVEntry *entry = get_hash_table(hashtable, key);
  return entry == NULL ? default_value : entry->value;
}

HashtableValue remove_hash_table(Hashtable *hashtable, HashtableKey key) {
  int flag;
  return remove_with_flag_hash_table(hashtable, key, &flag);
}

HashtableValue remove_with_flag_hash_table(Hashtable *hashtable, HashtableKey key, int *flag) {
  if (hashtable->bucket == NULL) {
    *flag = 0;
    return NULL;
  }
  // find index
  unsigned int h = hash(key, hashtable->hash_func);
  unsigned index = h % hashtable->bucket_capacity;
  TableNode *node = hashtable->bucket[index];
  if (node == NULL) return NULL;
  *flag = 0;
  KVEntry *removed = NULL;
  HashtableValue value = NULL;
  if (node->type == NODE_LINKED_LIST) {
    removed = remove_entry_list(&node->node.linked_list, key, hashtable->equals_func, flag);
  } else {
    removed = (KVEntry *) remove_with_flag_rb_tree(node->node.rb_tree, key, flag);
  }
  if (*flag == 1) {
    node->node_size--;
    hashtable->entry_size--;
    value = removed->value;
    if (hashtable->key_free_func != NULL) {
      hashtable->key_free_func(removed->key);
    }
    free(removed);
  }
  return value;
}

void set_load_factor_hash_table(Hashtable *hashtable, float load_factor) {
  hashtable->load_factor = load_factor;
  hashtable->threshold = (unsigned int) (1.0 * hashtable->bucket_capacity * hashtable->load_factor);
}

HashtableEntrySet *hashtable_entry_set(Hashtable *hashtable) {
  HashtableEntrySet *entry_set = malloc(sizeof(HashtableEntrySet));
  if (entry_set == NULL) return NULL;
  unsigned int table_size = size_of_hash_table(hashtable);
  entry_set->size = table_size;
  KVEntry **kv_entry = calloc(table_size, sizeof(KVEntry *));
  if (kv_entry == NULL) {
    free(entry_set);
    return NULL;
  }
  unsigned int s = 0;
  if (hashtable->entry_size == 0) {
    entry_set->entry_set = kv_entry;
    return entry_set;
  }

  HashtableIterator *iter = hashtable_iterator(hashtable);
  while (hashtable_iter_has_next(iter)) {
    KVEntry *entry = hashtable_next_entry(iter);
    kv_entry[s++] = entry;
  }
  entry_set->entry_set = kv_entry;
  free_hashtable_iter(iter);
  return entry_set;


//  for (int i = 0; i < hashtable->bucket_capacity; ++i) {
//    TableNode *node = hashtable->bucket[i];
//    if (node == NULL) continue;
//    if (node->type == NODE_LINKED_LIST) {
//      KVEntry *head = node->node.linked_list;
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

void free_hashtable_entry_set(HashtableEntrySet *hashtable_entry_set) {
  if (hashtable_entry_set != NULL) {
    free(hashtable_entry_set->entry_set);
    free(hashtable_entry_set);
  }
}

HashtableKey table_entry_key(KVEntry *entry) {
  return entry->key;
}

HashtableValue table_entry_value(KVEntry *entry) {
  return entry->value;
}

HashtableIterator *hashtable_iterator(Hashtable *hashtable) {
  HashtableIterator *iterator = malloc(sizeof(HashtableIterator));
  if (!iterator) return NULL;
  iterator->hashtable = hashtable;
  iterator->current_bucket_index = -1;
  iterator->current_node_type = NODE_LINKED_LIST;
  if (hashtable->bucket == NULL) {
    // no elements yet
    return iterator;
  } else {
    // find first no-empty bucket
    for (int i = 0; i < hashtable->bucket_capacity; ++i) {
      TableNode *node = hashtable->bucket[i];
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

int hashtable_iter_has_next(HashtableIterator *iter) {
  if (iter->current_bucket_index == -1) return 0;
  if (iter->current_node_type == NODE_LINKED_LIST) {
    return iter->current_node == NULL ? 0 : 1;
  } else {
    return has_next_rbt_entry(iter->rb_iter);
  }
}

KVEntry *hashtable_next_entry(HashtableIterator *iter) {
  KVEntry *ret;
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
    for (int i = current_bucket_index + 1; i < iter->hashtable->bucket_capacity; ++i) {
      TableNode *node = iter->hashtable->bucket[i];
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

void free_hashtable_iter(HashtableIterator *iter) {
  if (iter) {
    if (iter->current_bucket_index != -1 && iter->current_node_type == NODE_RED_BLACK_TREE) {
      free_rb_tree_iterator(iter->rb_iter);
    }
    free(iter);
  }
}