//
// Created by ran on 2024/1/15.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tree/b_tree2.h"
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
  btree *tree = bt_create(3);
  int size = 100;
  int count = 0;
  srand(time(NULL));
  for (int i = 0; i < size; ++i) {
    int k = rand() % (size + 1);
    int inserted = bt_insert(tree, k);
    if (inserted == 1) {
      count++;
      int r = bt_search(tree, k);
      assert(r == k);
    }
    assert(bt_validate(tree) == 1);
  }
  assert(bt_size(tree) == count);
  //print_btree(tree);
  bt_free(tree);
}

btree *generate_random_tree(int m, int size, int seed) {
  btree *tree = bt_create(m);
  srand(seed);
  int count = 0;
  for (int i = 0; i < size; ++i) {
    int k = rand() % (size + 1);
    int inserted = bt_insert(tree, k);
    if (inserted == 1) {
      count++;
      assert(bt_search(tree, k) == k);
    }
  }
  assert(bt_size(tree) == count);
  return tree;
}
void test_remove_btree() {

  btree *tree = bt_create(3);
  int insertElements[] = {25, 8, 37, 55, 95, 27, 88, 13, 29, 42, 51,
                          72, 100, 105, 90, 92, 50, 49, 45, 47};
  int i;
  for (i = 0; i < 20; i++) {
    bt_insert(tree, insertElements[i]);
  }
  int size = 20;
  assert(bt_size(tree) == size);

  print_bt(tree);
  printf("\n");
  fflush(stdout);
  int DELETELEN = 16;

  int deleteElements[] = {27, 29, 88, 100, 51, 90, 37, 95, 72, 42,
                          92, 8, 55, 13, 105, 25};

  for (i = 0; i < DELETELEN; i++) {
    printf("-%04d: \n", deleteElements[i]);
    fflush(stdout);
    int r = bt_delete(tree, deleteElements[i]);
    assert(r == 1);
    size--;
    assert(bt_size(tree) == size);
    print_bt(tree);
    printf("\n");
    fflush(stdout);
  }
  bt_free(tree);
}

void test_validate() {
  int t_size = 23;
  int ts[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 50,
              51, 55, 58, 100, 101, 200, 251, 288,
              1000, 1001, 1023, 1024};

  //1705641779
  int seed_size = 7;
  int seeds[] = {1705641779, 1705641827, 1705641837, 1705641904,
                 1705642345, 1705642215, 1705647642};

  int s_size = 12;
  int ss[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 100, 1000, 10000};

  for (int i = 0; i < seed_size; i++) {
    for (int j = 0; j < t_size; j++) {
      for (int k = 0; k < s_size; ++k) {
        btree *tree = generate_random_tree(ts[j], ss[k], seeds[i]);
        assert(bt_validate(tree) == 1);

        int tree_size = bt_size(tree);
        int removed_count = 0;
        printf("validate: t:%d, size:%d, seeds: %d\n", ts[j], ss[k], seeds[i]);
        print_bt(tree);
        for (int p = 0; p < ss[k]; ++p) {
          int r = rand() % (ss[k] + 1);
          int remvoed = bt_delete(tree, r);
          if (remvoed == 1) {
            assert(bt_search(tree, r) == -1);
            removed_count++;
            assert(bt_size(tree) == tree_size - removed_count);
            if(bt_validate(tree)!=1){
              printf("remove:%d\n",r);
              print_bt(tree);
              fflush(stdout);
            }
            assert(bt_validate(tree) == 1);
          } else {
            assert(bt_size(tree) == tree_size - removed_count);
          }
        }
        print_bt(tree);
        bt_free(tree);

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
