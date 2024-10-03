
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "common/framework.h"

#include "tree/rb_tree.h"

#define NUM_TEST_VALUES 1000

int test_array[NUM_TEST_VALUES];

#if 0
/* Tree print function - useful for debugging. */

static void print_tree(RBTreeNode *node, int depth)
{
    int *value;
    int i;

    if (node == NULL) {
        return;
    }

    print_tree(rb_tree_node_child(node, RB_TREE_NODE_LEFT), depth + 1);

    for (i=0; i<depth*6; ++i) {
        printf(" ");
    }

    value = rb_tree_node_key(node);
    printf("%i\n", *value);

    print_tree(rb_tree_node_child(node, RB_TREE_NODE_RIGHT), depth + 1);
}
#endif
int int_compare(void *vlocation1, void *vlocation2) {
  int *location1;
  int *location2;

  location1 = (int *) vlocation1;
  location2 = (int *) vlocation2;

  //printf("data1: %d, data2: %d\n", *location1, *location2);

  if (*location1 < *location2) {
    //printf("   <\n");
    return -1;
  } else if (*location1 > *location2) {
    //printf("   >\n");
    return 1;
  } else {
    // printf("   ==\n");
    return 0;
  }
}
int keys[10] = {98, 87, 65, 65, 43, 32, 21, 22, 20, 18};
int values[10] = {98, 87, 65, 650, 43, 32, 21, 22, 20, 18};

int k2[4] = {4, 9, 5, 8};
int k3[8] = {0, 3, 4, 5, 10, 9, 6, 8};

int size = 1000000;

RBTree *create_rbtree() {
  RBTree *rb_tree = new_rb_tree(int_compare);

  srand(1024);
  for (int i = 0; i < size; i++) {
    int *b = malloc(sizeof(int));
    *b = rand() % (size + 1);
    if (!contains_in_rb_tree(rb_tree, b)) {
      put_rb_tree(rb_tree, b, b);
    } else {
      free(b);
    }
  }
  printf("size of rbt: %d , height of rbt: %d\n", size_of_rbt(rb_tree), height_of_rbt(rb_tree));
  fflush(stdout);
  // Generate a random number between 0 and 1000000


//  for (int i = 0; i < 8; i++) {
//    printf("insert key:%d\n", k3[i]);
//    fflush(stdout);
//    put_rb_tree(rb_tree, &k3[i], &k3[i]);
//  }
  assert(rb_tree != NULL);
//  assert(size_of_rbt(rb_tree) == 9);
  return rb_tree;
}

void test_remove_rbt() {
  // create new rbt
  RBTree *rbt = create_rbtree();
  RBTree *removed_keys = new_rb_tree(int_compare);

  int total = size_of_rbt(rbt);

  int total_removed = 0;
  int round = 10;
  // remove for 10 rounds
  for (int r = 0; r < round; ++r) {
    int round_removed = 0;
    srand(r+1);
    for (int i = 0; i < size; ++i) {
      int b = rand() % (size + 1);
      RBTreeNode *node = get_node_rb_tree(rbt, &b);
      if (node != NULL) {
        put_rb_tree(removed_keys, key_of_rb_node(node), value_of_rb_node(node));
        RBTreeValue va = remove_rb_tree(rbt, &b);
        assert(*(int *) va == b);
        total_removed++;
        round_removed++;
      }
    }
    printf("round[%d] : remove count : %d. total removed: %d\n", r, round_removed, total_removed);
    fflush(stdout);
    assert(size_of_rbt(rbt) == total - total_removed);


    //validate rbt after a round of remove
    RBTreeIterator *iter = rb_tree_iterator(rbt);

    int last_key = -1;
    int itsize = 0;
    while (has_next_rbt_entry(iter)) {
      itsize++;
      RBNodeEntry entry = next_rbt_entry(iter);
      int k = *(int *) entry.key;
      assert(last_key < k);
      last_key = k;
      RBTreeNode *n = get_node_rb_tree(rbt, entry.key);
      double diff = rb_tree_diff_lr_height(n);
      assert(diff > 0.5 || diff < 2.0);
      //free(entry.key);
    }
    free_rb_tree_iterator(iter);
  }
  // free removed keys
  RBTreeIterator *iter = rb_tree_iterator(removed_keys);
  while (has_next_rbt_entry(iter)) {
    RBNodeEntry entry = next_rbt_entry(iter);
    free(entry.key);
  }
  int r = size_of_rbt(rbt);
  printf("size at first: %d, total removed: %d ,remaining count: %d, \n", total, total_removed, r);
  assert(total == total_removed + r);


  // remove remaining keys
  RBTreeIterator *iter2 = rb_tree_iterator(rbt);
  while (has_next_rbt_entry(iter2)) {
    RBNodeEntry entry = next_rbt_entry(iter2);
    free(entry.key);
  }

  free_rb_tree_iterator(iter);
  free_rb_tree_iterator(iter2);
  free_rb_tree(removed_keys);
  free_rb_tree(rbt);
}

static UnitTestFunction tests[] = {
    test_remove_rbt,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);
  return 0;
}


