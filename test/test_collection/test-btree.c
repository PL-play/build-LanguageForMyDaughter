//
// Created by ran on 2024/1/15.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tree/b_tree.h"
#include "common/framework.h"

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

void test_insert_btree() {
  int *k;
  BTree *tree = new_btree(4, int_compare);
  int size = 1000;
  int count = 0;
  srand(time(NULL));
  for (int i = 0; i < size; ++i) {
    k = malloc(sizeof(int));
    *k = rand() % (size + 1);
    printf("%d\n", *k);
    fflush(stdout);
    int inserted = btree_insert_key(tree, k);
    if (inserted == 1) {
      count++;
      BTreeNode *node = btree_search(tree, k);
      assert(node != NULL);
    } else {
      free(k);
    }
  }
  assert(sizeof_btree(tree) == count);
  //print_btree(tree);

  register_btree_keyfree(tree, free);
  free_btree(tree);
  tree = new_btree(3, int_compare);
  int insertElements[] = {25, 8, 37, 55, 95, 27, 88, 13, 29, 42, 51,
                          72, 100, 105, 90, 92, 50, 49, 45, 47};

  int i;
  for (i = 0; i < 20; i++) {
    btree_insert_key(tree, &insertElements[i]);
  }
  print_btree(tree);
  fflush(stdout);
  register_btree_keyfree(tree, NULL);
  free_btree(tree);

}

void test_remove_btree() {

  BTree *tree = new_btree(3, int_compare);
  int insertElements[] = {25, 8, 37, 55, 95, 27, 88, 13, 29, 42, 51,
                          72, 100, 105, 90, 92, 50, 49, 45, 47};
  int i;
  for (i = 0; i < 20; i++) {
    btree_insert_key(tree, &insertElements[i]);
  }
  int size = 20;
  assert(sizeof_btree(tree) == size);

  print_btree(tree);
  printf("\n");
  fflush(stdout);
  int DELETELEN = 16;

  int deleteElements[] = {27, 29, 88, 100, 51, 90, 37, 95, 72, 42,
                          92, 8, 55, 13, 105, 25};

  for (i = 0; i < DELETELEN; i++) {
    printf("-%04d: \n", deleteElements[i]);
    fflush(stdout);
    int r = btree_remove(tree, &deleteElements[i]);
    assert(r == 1);
    size--;
    assert(sizeof_btree(tree) == size);
    print_btree(tree);
    printf("\n");
    fflush(stdout);
  }
  free_btree(tree);

  tree = new_btree(6, int_compare);
  int *k;
  size = 400000;
  int count = 0;
  srand(time(NULL));
  for (int i = 0; i < size; ++i) {
    k = malloc(sizeof(int));
    *k = rand() % (size + 1);
    int inserted = btree_insert_key(tree, k);
    if (inserted == 1) {
      count++;
      BTreeNode *node = btree_search(tree, k);
      assert(node != NULL);
    } else {
      free(k);
    }
  }
  assert(btree_validate(tree) == 1);
  int removed_count = 0;
  assert(sizeof_btree(tree) == count);
  register_btree_keyfree(tree, free);
  for (int i = 0; i < size; ++i) {
    k = malloc(sizeof(int));
    *k = rand() % (size + 1);
    int remvoed = btree_remove(tree, k);
    if (remvoed == 1) {
      removed_count++;
      assert(sizeof_btree(tree) == count - removed_count);
    }
    free(k);
  }
  free_btree(tree);
}

BTree *generate_random_tree(int m, int size, int seed) {
  BTree *tree = new_btree(m, int_compare);
  register_btree_keyfree(tree, free);
  srand(seed);
  int count = 0;
  for (int i = 0; i < size; ++i) {
    int *k = malloc(sizeof(int));
    *k = rand() % (size + 1);
    int inserted = btree_insert_key(tree, k);
    if (inserted == 1) {
      count++;
      BTreeNode *node = btree_search(tree, k);
      assert(node != NULL);
    } else {
      free(k);
    }
  }

  return tree;
}

void test_validate() {
  int m_size = 22;
  int ms[] = {3, 4, 5, 6, 7, 8, 9, 10, 20, 50,
              51, 55, 58, 100, 101, 200, 251, 288,
              1000, 1001, 1023, 1024};

  //1705641779
  int seed_size = 7;
  int seeds[] = {1705641779, 1705641827, 1705641837, 1705641904,
                 1705642345, 1705642215, 1705647642};

  int s_size = 12;
  int ss[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 100, 1000, 10000};

  for (int i = 0; i < seed_size; i++) {
    for (int j = 0; j < m_size; j++) {
      for (int k = 0; k < s_size; ++k) {
        BTree *tree = generate_random_tree(ms[j], ss[k], seeds[i]);
        assert(btree_validate(tree) == 1);
        int tree_size = sizeof_btree(tree);
        int removed_count = 0;
        for (int p = 0; p < ss[k]; ++p) {
          int *r = malloc(sizeof(int));
          *r = rand() % (ss[k] + 1);
          int remvoed = btree_remove(tree, r);
          if (remvoed == 1) {
            assert(btree_search(tree, r) == NULL);
            removed_count++;
            assert(sizeof_btree(tree) == tree_size - removed_count);
            assert(btree_validate(tree) == 1);
          }
          free(r);
        }
        free_btree(tree);

        printf("validate: m:%d, size:%d, seeds: %d\n", ms[j], ss[k], seeds[i]);
      }

    }
  }
}

static UnitTestFunction tests[] = {
    test_insert_btree,
    test_remove_btree,
    test_validate,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}
