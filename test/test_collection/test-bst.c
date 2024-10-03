//
// Created by ran on 2024/1/3.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/framework.h"
#include "tree/binary_search_tree.h"

int nums[] = {45, 2, 23, 57, 36, 89, 77, 12, 80, 2};
int num = 10;
void *data[10];
int v1 = 55, v2 = 99, v3 = 17, v4 = 0, v5 = 76, vd = 12;

typedef struct Param {
  int size;
  int values[20];
} Param;

void printbst(BSTData value, void *param) {
  printf("%d ", *(int *) value);
  fflush(stdout);
  Param *p = (Param *) param;
  p->values[p->size++] = *(int *) value;
}

void traversal_in(BSTNode *bst_node, void *p) {
  traversal_bst(bst_node, BST_MID, printbst, p);
  printf("\n");
  fflush(stdout);
}

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

static void generate_data() {
  for (int i = 0; i < num; i++) {
    data[i] = &nums[i];
  }
}

static void test_printdata() {
  generate_data();
  for (int i = 0; i < num; i++) {
    printf("%p -> %d\n", data[i], *(int *) data[i]);
  }
  fflush(stdout);
}

BSTNode *generate_bst() {
  generate_data();
  BSTNode *bst = NULL;
  printf("size of bst:%d\n", size_of_bst(bst));
  printf("\n");
  fflush(stdout);
  for (int i = 0; i < num; ++i) {
    printf("add %d to bst\n", *(int *) data[i]);
    fflush(stdout);
    bst = add_to_bst(bst, data[i], int_compare);
    printf("size of bst:%d\n", size_of_bst(bst));
    printf("\n");
    fflush(stdout);
  }
  printf("size of bst:%d\n", size_of_bst(bst));
  fflush(stdout);
  assert(size_of_bst(bst) == 9);
  return bst;
}

void test_add_bst() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  Param p;
  p.size = 0;
  traversal_in(bst, &p);
  bst = add_to_bst(bst, &v1, int_compare);
  assert(size_of_bst(bst) == 10);
  bst = add_to_bst(bst, &v1, int_compare);
  assert(size_of_bst(bst) == 10);
  bst = add_to_bst(bst, &v2, int_compare);
  assert(size_of_bst(bst) == 11);

  for (int i = 0; i < num; i++) {
    assert(contains_in_bst(bst, &nums[i], int_compare) == 1);
  }
  assert(contains_in_bst(bst, &v4, int_compare) == 0);
  free_bst(bst);
  for (int i = 1; i < p.size; i++) {
    assert(p.values[i] > p.values[i - 1]);
  }
}

void test_remove_bst() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  test_printdata();
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 1);
  for (int i = 0; i < num - 1; ++i) {
    bool flag = false;
    bst = remove_data_in_bst(bst, &nums[i], int_compare, &flag);
    assert(flag == true);
    puts("");
    Param p;
    p.size = 0;
    traversal_in(bst, &p);
    for (int j = 1; j < p.size; j++) {
      assert(p.values[j] > p.values[j - 1]);
    }
    assert(size_of_bst(bst) == num - 2 - i);
  }

  free_bst(bst);

  bst = generate_bst();
  bool flag = false;
  bst = remove_data_in_bst(bst, &v1, int_compare, &flag);
  assert(false == false);
  assert(size_of_bst(bst) == 9);
  Param p;
  p.size = 0;
  traversal_in(bst, &p);

  bst = remove_data_in_bst(bst, &vd, int_compare, &flag);
  assert(flag == true);
  assert(size_of_bst(bst) == 8);
  p.size = 0;
  traversal_in(bst, &p);
  for (int j = 1; j < p.size; j++) {
    assert(p.values[j] > p.values[j - 1]);
  }
  free_bst(bst);
}

void test_remove_min_max() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  test_printdata();
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 1);

  bst = remove_min_of_bst(bst);
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 2);
  int min = 2;
  assert(contains_in_bst(bst, &min, int_compare) == 0);

  bst = remove_min_of_bst(bst);
  min = 12;
  assert(contains_in_bst(bst, &min, int_compare) == 0);

  int max = 89;
  bst = remove_max_of_bst(bst);
  assert(contains_in_bst(bst, &max, int_compare) == 0);

  assert(size_of_bst(bst) == 6);
  free_bst(bst);
}

void test_min_max_of_bst() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  test_printdata();
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 1);
  BSTData min = min_data_of_bst(bst);
  assert(*(int *) min == 2);

  BSTData max = max_data_of_bst(bst);
  assert(*(int *) max == 89);

  free_bst(bst);
}

void test_floor_bst() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  test_printdata();
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 1);

  for (int i = 0; i < 9; ++i) {
    BSTNode *floor = floor_of_bst(bst, &nums[i], int_compare);
    assert(floor != NULL);
    assert(contains_in_bst(bst, value_of_node_in_bst(floor), int_compare) == 1);
    assert(value_of_node_in_bst(floor) == &nums[i]);
  }
  int n = 40;
  BSTNode *floor = floor_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 36);

  n = 25;
  floor = floor_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 23);

  n = 46;
  floor = floor_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 45);

  n = 58;
  floor = floor_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 57);

  n = 90;
  floor = floor_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 89);

  n = 78;
  floor = floor_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 77);

  n = 1;
  floor = floor_of_bst(bst, &n, int_compare);
  assert(floor == NULL);

  free_bst(bst);
}

void test_ceil_bst() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  test_printdata();
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 1);

  for (int i = 0; i < 9; ++i) {
    BSTNode *floor = ceil_of_bst(bst, &nums[i], int_compare);
    assert(floor != NULL);
    assert(contains_in_bst(bst, value_of_node_in_bst(floor), int_compare) == 1);
    assert(value_of_node_in_bst(floor) == &nums[i]);
  }
  int n = 40;
  BSTNode *floor = ceil_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 45);

  n = 25;
  floor = ceil_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 36);

  n = 46;
  floor = ceil_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 57);

  n = 58;
  floor = ceil_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 77);

  n = 90;
  floor = ceil_of_bst(bst, &n, int_compare);
  assert(floor == NULL);

  n = 78;
  floor = ceil_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 80);

  n = 1;
  floor = ceil_of_bst(bst, &n, int_compare);
  assert(floor != NULL);
  assert(*(int *) value_of_node_in_bst(floor) == 2);

  free_bst(bst);
}

void test_select_bst() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  test_printdata();
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 1);
  //2 12 23 36 45 57 77 80 89
  BSTNode *node = select_of_bst(bst, 1);
  assert(node != NULL);
  int value = *(int *) value_of_node_in_bst(node);
  assert(value == 2);

  node = select_of_bst(bst, 2);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 12);

  node = select_of_bst(bst, 3);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 23);

  node = select_of_bst(bst, 4);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 36);

  node = select_of_bst(bst, 5);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 45);

  node = select_of_bst(bst, 6);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 57);

  node = select_of_bst(bst, 7);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 77);

  node = select_of_bst(bst, 8);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 80);

  node = select_of_bst(bst, 9);
  assert(node != NULL);
  value = *(int *) value_of_node_in_bst(node);
  assert(value == 89);

  node = select_of_bst(bst, 0);
  assert(node == NULL);

  node = select_of_bst(bst, 10);
  assert(node == NULL);
  free_bst(bst);
}

void test_rank_bst() {
  BSTNode *bst = NULL;
  bst = generate_bst();
  test_printdata();
  assert(size_of_bst(bst) == sizeof(nums) / sizeof(int) - 1);
  //2 12 23 36 45 57 77 80 89

  int v = 45;
  unsigned int rank = rank_of_bst(bst, &v, int_compare);
  assert(rank == 5);

  v = 57;
  rank = rank_of_bst(bst, &v, int_compare);
  assert(rank == 6);

  v = 2;
  rank = rank_of_bst(bst, &v, int_compare);
  assert(rank == 1);

  v = 57;
  rank = rank_of_bst(bst, &v, int_compare);
  assert(rank == 6);

  v = 89;
  rank = rank_of_bst(bst, &v, int_compare);
  assert(rank == 9);

  v = 8;
  rank = rank_of_bst(bst, &v, int_compare);
  assert(rank == 0);

  free_bst(bst);
}
static UnitTestFunction tests[] = {
    test_add_bst,
    test_remove_bst,
    test_remove_min_max,
    test_min_max_of_bst,
    test_floor_bst,
    test_ceil_bst,
    test_select_bst,
    test_rank_bst,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);

  return 0;
}