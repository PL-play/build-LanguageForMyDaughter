//
// Created by ran on 2024-03-29.
//
#ifndef ZCOLLECTION_HASHTABLE_HASH_TABLE_M_H_
#define ZCOLLECTION_HASHTABLE_HASH_TABLE_M_H_
#define BASE_HASH_TABLE(__name) \
        typedef struct {               \
           unsigned int bucket_capacity; \
        } __name;

#define DECLARE_HASHTABLE(__key_type, __val_type, __name)                                                                           \
typedef __key_type __name##HashtableKey;                                                                                       \
typedef __val_type __name##HashtableValue;                                                                                     \
typedef uint32_t (* __name##HashtableHashFunc)(__name##HashtableKey hashtable_key);                                       \
typedef int(* __name##HashtableEqualsFunc)(__name##HashtableKey key1, __name##HashtableKey key2);                         \
typedef void(*__name##HashtableKeyFreeFunc)(__name##HashtableKey key);                                                    \
typedef void(*__name##HashtableValueFreeFunc)(__name##HashtableValue value);                                              \
typedef struct __name##KVEntry __name##KVEntry;                                                                           \
typedef struct __name##TableNode __name##TableNode;                                                                       \
typedef struct __name##Hashtable __name##Hashtable;                                                                       \
typedef struct __name##HashtableEntrySet {                                                                                \
  size_t size;                                                                                                            \
  __name##KVEntry **entry_set;                                                                                            \
} __name##HashtableEntrySet;                                                                                              \
typedef struct __name##HashtableIterator __name##HashtableIterator;                                                       \
                                                                                                                          \
__name##Hashtable *__name##new_hash_table(__name##HashtableHashFunc hash_func, __name##HashtableEqualsFunc equals_func);  \
                                                                                                                          \
int __name##put_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key, __name##HashtableValue value);         \
int __name##put_free_exist_hash_table(__name##Hashtable *hashtable,                                                       \
                              __name##HashtableKey key,                                                                   \
                              __name##HashtableValue value,                                                               \
                              void(*free_func)(__name##HashtableValue));                                                  \
                                                                                                                          \
void __name##register_hashtable_free_functions(__name##Hashtable *hashtable,                                              \
                                       __name##HashtableKeyFreeFunc key_free_func,                                        \
                                       __name##HashtableValueFreeFunc value_free_func);                                   \
                                                                                                                          \
size_t __name##size_of_hash_table(__name##Hashtable *hashtable);                                                                    \
typedef struct __name##KVEntry {                                                                                          \
  __name##HashtableKey key;                                                                                               \
  __name##HashtableValue value;                                                                                           \
  struct __name##KVEntry *next;                                                                                           \
  uint32_t hash_code;                                                                                                     \
} __name##KVEntry;                                                                                                        \
                                                                                                                          \
typedef struct __name##TableNode {                                                                                        \
  size_t node_size;                                                                                                       \
  __name##KVEntry *linked_list;          \
} __name##TableNode;          \
          \
typedef struct __name##Hashtable {                       \
 unsigned int bucket_capacity;          \
  __name##TableNode **bucket;          \
  unsigned int prime_index;          \
  unsigned int entry_size;          \
  unsigned int threshold;          \
  float load_factor;          \
          \
  __name##HashtableHashFunc hash_func;          \
  __name##HashtableEqualsFunc equals_func;          \
  __name##HashtableKeyFreeFunc key_free_func;          \
  __name##HashtableValueFreeFunc value_free_func;          \
} __name##Hashtable;          \
          \
          \
typedef struct __name##HashtableIterator {          \
  __name##Hashtable *hashtable;          \
  int current_bucket_index;          \
  __name##KVEntry *current_node;          \
} __name##HashtableIterator;\
                                                                                                                          \
__name##HashtableValue __name##get_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key);                    \
__name##HashtableValue __name##get_default_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key,             \
                                                      __name##HashtableValue default_value);                              \
int __name##contains_in_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key);                               \
__name##HashtableValue __name##remove_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key);                 \
__name##HashtableValue __name##remove_with_flag_hash_table(__name##Hashtable *hashtable,                                  \
                                                            __name##HashtableKey key, int *flag);                         \
void __name##set_load_factor_hash_table(__name##Hashtable *hashtable, float load_factor);                                 \
void __name##free_hash_table(__name##Hashtable *hashtable);                                                               \
__name##HashtableEntrySet *__name##hashtable_entry_set(__name##Hashtable *hashtable);                                     \
void __name##free_hashtable_entry_set(__name##HashtableEntrySet *hashtable_entry_set);                                    \
__name##HashtableKey __name##table_entry_key(__name##KVEntry *entry);                                                     \
__name##HashtableValue __name##table_entry_value(__name##KVEntry *entry);                                                 \
__name##HashtableIterator *__name##hashtable_iterator(__name##Hashtable *__name##hashtable);                              \
int __name##hashtable_iter_has_next(__name##HashtableIterator *iter);                                                     \
__name##KVEntry *__name##hashtable_next_entry(__name##HashtableIterator *iter);                                           \
void __name##free_hashtable_iter(__name##HashtableIterator *iter);                                                        \
__name##KVEntry *__name##get_entry_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key);

#define DEFINE_HASHTABLE(__val_type, __null_val, __name) \
static __val_type __name##null_val = __null_val;                                                                               \
static const unsigned int __name##hash_table_primes[] =                                                                   \
    {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,                                                               \
     49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469,                                                     \
     12582917, 25165843, 50331653, 100663319, 201326611, 402653189,                                                       \
     805306457, 1610612741};                                                                                              \
                                                                                                                          \
static uint32_t __name##hash(__name##HashtableKey key, __name##HashtableHashFunc hash_func);          \
static void __name##resize(__name##Hashtable *__name##hashtable);          \
static int __name##insert_entry(__name##TableNode *node,          \
                        __name##HashtableEqualsFunc equals_func,          \
                        uint32_t hash_code,          \
                        __name##HashtableKey key,          \
                        __name##HashtableValue value,          \
                        __name##HashtableValueFreeFunc free_func);          \
static __name##KVEntry *__name##search_entry_list(__name##KVEntry *head, __name##HashtableKey key, __name##HashtableEqualsFunc equals_func);          \
static void __name##prepend_entry(__name##TableNode *node, __name##KVEntry *entry);          \
static void __name##free_bucket(__name##TableNode **bucket, unsigned int size,          \
                        __name##HashtableKeyFreeFunc key_free_func,          \
                        __name##HashtableValueFreeFunc value_free_func, int clear_entry);          \
static unsigned int __name##free_node(__name##TableNode *node, __name##HashtableKeyFreeFunc key_free_func,          \
                              __name##HashtableValueFreeFunc value_free_func, int clear_entry);          \
static int __name##insert_entry_to_list(__name##TableNode *node, __name##KVEntry *entry, __name##HashtableEqualsFunc equals_func);          \
static int __name##transfer_nodes(__name##TableNode **new_bucket,     \
                              unsigned new_bucket_capacity,     \
                              __name##TableNode *old_node,     \
                              __name##HashtableEqualsFunc equals_func);                                           \
static unsigned int __name##free_list_node(__name##TableNode *node, __name##HashtableKeyFreeFunc key_free_func,          \
                                   __name##HashtableValueFreeFunc value_free_func);          \
          \
__name##Hashtable *__name##new_hash_table(__name##HashtableHashFunc hash_func, __name##HashtableEqualsFunc equals_func) {          \
  __name##Hashtable *hashtable = malloc(sizeof(__name##Hashtable));          \
  if (hashtable == NULL) return NULL;          \
  hashtable->hash_func = hash_func;          \
  hashtable->equals_func = equals_func;          \
  hashtable->load_factor = 0.75;          \
  hashtable->entry_size = 0;          \
  hashtable->prime_index = 0;          \
  hashtable->bucket_capacity = __name##hash_table_primes[0];          \
  hashtable->threshold = (unsigned int) (1.0 * hashtable->bucket_capacity * hashtable->load_factor);          \
  hashtable->bucket = NULL;          \
  hashtable->key_free_func = NULL;          \
  hashtable->value_free_func = NULL;          \
  return hashtable;          \
}          \
          \
void __name##register_hashtable_free_functions(__name##Hashtable *hashtable,          \
                                       __name##HashtableKeyFreeFunc key_free_func,          \
                                       __name##HashtableValueFreeFunc value_free_func) {          \
  hashtable->key_free_func = key_free_func;          \
  hashtable->value_free_func = value_free_func;          \
}          \
          \
int __name##put_free_exist_hash_table(__name##Hashtable *hashtable,          \
                              __name##HashtableKey key,          \
                              __name##HashtableValue value,          \
                              void(*free_func)(__name##HashtableValue)) {          \
  if (hashtable->bucket == NULL) {          \
    __name##TableNode **nodes = calloc(hashtable->bucket_capacity, sizeof(__name##TableNode *));          \
    if (nodes == NULL) return 0;          \
    hashtable->bucket = nodes;          \
  }          \
          \
  uint32_t h = __name##hash(key, hashtable->hash_func);          \
  unsigned index = h % hashtable->bucket_capacity;          \
  __name##TableNode *node;          \
  if (hashtable->bucket[index] == NULL) {          \
    node = malloc(sizeof(__name##TableNode));          \
    if (node == NULL) return 0;          \
    node->node_size = 0;          \
    node->linked_list = NULL;          \
    hashtable->bucket[index] = node;          \
  } else {          \
    node = hashtable->bucket[index];          \
  }          \
  int ret = __name##insert_entry(node, hashtable->equals_func, h, key, value, free_func);          \
          \
  if (ret == 1) hashtable->entry_size++;          \
          \
  if (hashtable->entry_size > hashtable->threshold) {          \
    __name##resize(hashtable);          \
  }          \
  return ret;          \
}          \
          \
int __name##put_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key, __name##HashtableValue value) {          \
  return __name##put_free_exist_hash_table(hashtable, key, value, NULL);          \
}          \
          \
static uint32_t __name##hash(__name##HashtableKey key, __name##HashtableHashFunc hash_func) {          \
  return hash_func(key);          \
}          \
          \
static unsigned int __name##free_list_node(__name##TableNode *node, __name##HashtableKeyFreeFunc key_free_func,          \
                                   __name##HashtableValueFreeFunc value_free_func) {          \
  __name##KVEntry *head = node->linked_list;          \
  while (head != NULL) {          \
    __name##KVEntry *next = head->next;          \
    if (key_free_func != NULL) {          \
      key_free_func(head->key);          \
    }          \
    if (value_free_func != NULL) {          \
      value_free_func(head->value);          \
    }          \
    free(head);          \
    head = next;          \
  }          \
  unsigned int size = node->node_size;          \
  free(node);          \
  return size;          \
}          \
          \
static unsigned int __name##free_node(__name##TableNode *node, __name##HashtableKeyFreeFunc key_free_func,          \
                              __name##HashtableValueFreeFunc value_free_func, int clear_entry) {          \
  if (node == NULL) return 0;          \
  return __name##free_list_node(node, key_free_func, value_free_func);          \
}          \
          \
static void __name##free_bucket(__name##TableNode **bucket,          \
                        unsigned int size,          \
                        __name##HashtableKeyFreeFunc key_free_func,          \
                        __name##HashtableValueFreeFunc value_free_func, int clear_entry) {          \
  if (bucket == NULL) return;          \
  unsigned int removed_entry = 0;          \
  for (unsigned int i = 0; i < size; i++) {          \
    removed_entry += __name##free_node(bucket[i], key_free_func, value_free_func, clear_entry);          \
  }          \
  free(bucket);          \
}          \
          \
static void __name##free_old_bucket_nodes_after_resize(__name##TableNode **bucket,          \
                                               unsigned int size) {          \
  for (unsigned int i = 0; i < size; i++) {          \
    if (bucket[i] == NULL) continue;          \
    free(bucket[i]);          \
  }          \
  free(bucket);          \
}          \
static int __name##transfer_nodes(__name##TableNode **new_bucket,     \
                              unsigned new_bucket_capacity,     \
                              __name##TableNode *old_node,     \
                              __name##HashtableEqualsFunc equals_func) {     \
     \
  unsigned int node_size = old_node->node_size;     \
  __name##KVEntry *entries[node_size];     \
  __name##KVEntry *head = old_node->linked_list;     \
  int i = 0;     \
  while (head != NULL) {     \
    entries[i++] = head;     \
    head = head->next;     \
  }     \
  for (int j = 0; j < node_size; ++j) {     \
    __name##KVEntry *entry = entries[j];     \
    entry->next = NULL;     \
    unsigned index = entries[j]->hash_code % new_bucket_capacity;     \
    if (new_bucket[index] == NULL) {     \
      __name##TableNode *node = malloc(sizeof(__name##TableNode));     \
      if (node == NULL) return 0;     \
      node->node_size = 0;     \
      node->linked_list = NULL;     \
      new_bucket[index] = node;     \
    }     \
    __name##TableNode *node = new_bucket[index];     \
    __name##insert_entry_to_list(node, entry, equals_func);     \
  }     \
  return 1;     \
}                                           \
                                           \
static void __name##resize(__name##Hashtable *hashtable) {          \
  unsigned int new_cap = __name##hash_table_primes[hashtable->prime_index + 1];          \
  __name##TableNode **new_bucket = calloc(new_cap, sizeof(__name##TableNode *));          \
  if (new_bucket == NULL) return;          \
          \
  for (unsigned int i = 0; i < hashtable->bucket_capacity; ++i) {          \
    __name##TableNode *old_node = hashtable->bucket[i];          \
    if (old_node == NULL)continue;         \
    __name##transfer_nodes(new_bucket, new_cap, old_node, hashtable->equals_func);   \
  }          \
  __name##free_old_bucket_nodes_after_resize(hashtable->bucket, hashtable->bucket_capacity);          \
  hashtable->bucket = new_bucket;          \
  hashtable->prime_index++;          \
  hashtable->bucket_capacity = new_cap;          \
  hashtable->threshold = (unsigned int) (1.0 * hashtable->bucket_capacity * hashtable->load_factor);          \
          \
}          \
          \
static __name##KVEntry *__name##search_entry_list(__name##KVEntry *head, __name##HashtableKey key, __name##HashtableEqualsFunc equals_func) {          \
  __name##KVEntry *cur = head;          \
  while (cur != NULL) {          \
    if (equals_func(cur->key, key) == 0) {          \
      return cur;          \
    }          \
    cur = cur->next;          \
  }          \
  return NULL;          \
}          \
          \
static __name##KVEntry *__name##remove_entry_list(__name##KVEntry **head,          \
                                  __name##HashtableKey key,          \
                                  __name##HashtableEqualsFunc equals_func,          \
                                  int *removed) {          \
  __name##KVEntry *cur = *head;          \
  __name##KVEntry *prev = NULL;          \
  int flag = 0;          \
  while (cur != NULL) {          \
    if (equals_func(cur->key, key) == 0) {          \
      flag = 1;          \
      break;          \
    }          \
    prev = cur;          \
    cur = cur->next;          \
  }          \
  if (flag == 0) {          \
    *removed = 0;          \
    return NULL;          \
  }          \
  *removed = 1;          \
  if (prev == NULL) {          \
    *head = cur->next;          \
  } else {          \
    prev->next = cur->next;          \
  }          \
  return cur;          \
}          \
          \
static void __name##prepend_entry(__name##TableNode *node, __name##KVEntry *entry) {          \
  __name##KVEntry *entry_head = node->linked_list;          \
  if (entry_head == NULL) {          \
    node->linked_list = entry;          \
  } else {          \
    entry->next = node->linked_list;          \
    node->linked_list = entry;          \
  }          \
}          \
          \
static int __name##insert_entry_to_list(__name##TableNode *node, __name##KVEntry *entry, __name##HashtableEqualsFunc equals_func) {          \
  int ret;          \
  __name##KVEntry *linked_list = node->linked_list;          \
  __name##KVEntry *exists = __name##search_entry_list(linked_list, entry->key, equals_func);          \
  if (exists == NULL) {          \
    __name##prepend_entry(node, entry);          \
    ret = 1;          \
    node->node_size++;          \
  } else {          \
    exists->value = entry->value;          \
    ret = -1;          \
  }          \
  return ret;          \
}          \
          \
static int __name##insert_entry(__name##TableNode *node,          \
                        __name##HashtableEqualsFunc equals_func,          \
                        unsigned int hash_code,          \
                        __name##HashtableKey key,          \
                        __name##HashtableValue value,          \
                        __name##HashtableValueFreeFunc free_func) {          \
  int ret = 0;          \
  __name##KVEntry *linked_list = node->linked_list;          \
  __name##KVEntry *exists = __name##search_entry_list(linked_list, key, equals_func);          \
  if (exists == NULL) {          \
          \
    __name##KVEntry *entry = malloc(sizeof(__name##KVEntry));          \
          \
    if (entry == NULL) return ret;          \
    entry->key = key;          \
    entry->value = value;          \
    entry->hash_code = hash_code;          \
    entry->next = NULL;          \
    __name##prepend_entry(node, entry);          \
    ret = 1;          \
    node->node_size++;          \
  } else {          \
    if (free_func) {          \
      free_func(exists->value);          \
    }          \
    exists->value = value;          \
    ret = -1;          \
  }          \
          \
  return ret;          \
}          \
          \
void __name##free_hash_table(__name##Hashtable *hashtable) {          \
  if (hashtable == NULL) return;          \
  __name##free_bucket(hashtable->bucket, hashtable->bucket_capacity, hashtable->key_free_func, hashtable->value_free_func, 1);          \
  free(hashtable);          \
}          \
          \
size_t __name##size_of_hash_table(__name##Hashtable *hashtable) {          \
  return hashtable->entry_size;          \
}          \
          \
__name##KVEntry *__name##get_entry_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key) {          \
  if (hashtable->bucket == NULL) {          \
    return NULL;          \
  }          \
  uint32_t h = __name##hash(key, hashtable->hash_func);          \
  unsigned index = h % hashtable->bucket_capacity;          \
  __name##TableNode *node = hashtable->bucket[index];          \
  if (node == NULL) return NULL;          \
  __name##KVEntry *e = __name##search_entry_list(node->linked_list, key, hashtable->equals_func);          \
  return e;          \
          \
}          \
          \
int __name##contains_in_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key) {          \
  __name##KVEntry *entry = __name##get_entry_hash_table(hashtable, key);          \
  return entry != NULL ? 1 : 0;          \
}          \
          \
__name##HashtableValue __name##get_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key) {          \
  __name##KVEntry *entry = __name##get_entry_hash_table(hashtable, key);          \
  return entry == NULL ? __name##null_val : entry->value;          \
}          \
          \
__name##HashtableValue __name##get_default_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key, __name##HashtableValue default_value) {          \
  __name##KVEntry *entry = __name##get_entry_hash_table(hashtable, key);          \
  return entry == NULL ? default_value : entry->value;          \
}          \
          \
__name##HashtableValue __name##remove_with_flag_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key, int *flag) {          \
  if (hashtable->bucket == NULL) {                       \
    *flag=0;                                                  \
    return __name##null_val;          \
  }          \
          \
  uint32_t h = __name##hash(key, hashtable->hash_func);          \
  unsigned index = h % hashtable->bucket_capacity;          \
  __name##TableNode *node = hashtable->bucket[index];          \
  if (node == NULL) return __name##null_val;          \
  *flag = 0;          \
  __name##KVEntry *removed = NULL;          \
  __name##HashtableValue value = __name##null_val;          \
  removed = __name##remove_entry_list(&node->linked_list, key, hashtable->equals_func, flag);          \
  if (*flag == 1) {          \
    node->node_size--;          \
    hashtable->entry_size--;          \
    value = removed->value;          \
    if (hashtable->key_free_func != NULL) {          \
      hashtable->key_free_func(removed->key);          \
    }          \
    free(removed);          \
  }          \
  return value;          \
}          \
__name##HashtableValue __name##remove_hash_table(__name##Hashtable *hashtable, __name##HashtableKey key) {          \
  int flag;          \
  return __name##remove_with_flag_hash_table(hashtable, key, &flag);          \
}          \
          \
void __name##set_load_factor_hash_table(__name##Hashtable *hashtable, float load_factor) {          \
  hashtable->load_factor = load_factor;          \
  hashtable->threshold = (unsigned int) (1.0 * hashtable->bucket_capacity * hashtable->load_factor);          \
}          \
          \
void __name##free_hashtable_entry_set(__name##HashtableEntrySet *hashtable_entry_set) {          \
  if (hashtable_entry_set != NULL) {          \
    free(hashtable_entry_set->entry_set);          \
    free(hashtable_entry_set);          \
  }          \
}          \
          \
__name##HashtableKey __name##table_entry_key(__name##KVEntry *entry) {          \
  return entry->key;          \
}          \
          \
__name##HashtableValue __name##table_entry_value(__name##KVEntry *entry) {          \
  return entry->value;          \
}          \
          \
__name##HashtableIterator *__name##hashtable_iterator(__name##Hashtable *hashtable) {          \
  __name##HashtableIterator *iterator = malloc(sizeof(__name##HashtableIterator));          \
  if (!iterator) return NULL;          \
  iterator->hashtable = hashtable;          \
  iterator->current_bucket_index = -1;          \
  if (hashtable->bucket == NULL) {          \
          \
    return iterator;          \
  } else {          \
          \
    for (int i = 0; i < hashtable->bucket_capacity; ++i) {          \
      __name##TableNode *node = hashtable->bucket[i];          \
      if (node != NULL && node->node_size > 0) {          \
        iterator->current_bucket_index = i;          \
        iterator->current_node = node->linked_list;          \
        break;          \
      }          \
    }          \
    return iterator;          \
  }          \
}          \
int __name##hashtable_iter_has_next(__name##HashtableIterator *iter) {          \
  if (iter->current_bucket_index == -1) return 0;          \
  return iter->current_node == NULL ? 0 : 1;          \
}          \
          \
__name##KVEntry *__name##hashtable_next_entry(__name##HashtableIterator *iter) {          \
  __name##KVEntry *ret;          \
  ret = iter->current_node;          \
  iter->current_node = iter->current_node->next;          \
          \
  int need_move_bucket = 0;          \
          \
  if (iter->current_node == NULL) {          \
    need_move_bucket = 1;          \
  }          \
  if (need_move_bucket) {          \
          \
    int current_bucket_index = iter->current_bucket_index;          \
    iter->current_bucket_index = -1;          \
    for (int i = current_bucket_index + 1; i < iter->hashtable->bucket_capacity; ++i) {          \
      __name##TableNode *node = iter->hashtable->bucket[i];          \
      if (node != NULL && node->node_size > 0) {          \
        iter->current_bucket_index = i;          \
        iter->current_node = node->linked_list;          \
        break;          \
      }          \
    }          \
  }          \
  return ret;          \
}          \
          \
void __name##free_hashtable_iter(__name##HashtableIterator *iter) {          \
  if (iter) {          \
    free(iter);          \
  }          \
}          \
__name##HashtableEntrySet *__name##hashtable_entry_set(__name##Hashtable *hashtable) {          \
  __name##HashtableEntrySet *entry_set = malloc(sizeof(__name##HashtableEntrySet));          \
  if (entry_set == NULL) return NULL;          \
  size_t table_size = __name##size_of_hash_table(hashtable);          \
  entry_set->size = table_size;          \
  __name##KVEntry **kv_entry = calloc(table_size, sizeof(__name##KVEntry *));          \
  if (kv_entry == NULL) {          \
    free(entry_set);          \
    return NULL;          \
  }          \
  unsigned int s = 0;          \
  if (hashtable->entry_size == 0) {          \
    entry_set->entry_set = kv_entry;          \
    return entry_set;          \
  }          \
          \
  __name##HashtableIterator *iter = __name##hashtable_iterator(hashtable);          \
  while (__name##hashtable_iter_has_next(iter)) {          \
    __name##KVEntry *entry = __name##hashtable_next_entry(iter);          \
    kv_entry[s++] = entry;          \
  }          \
  entry_set->entry_set = kv_entry;          \
  __name##free_hashtable_iter(iter);          \
  return entry_set;          \
}
#endif
