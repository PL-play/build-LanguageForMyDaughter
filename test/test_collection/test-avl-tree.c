#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "common/framework.h"

#include "tree/avl_tree.h"

#define NUM_TEST_VALUES 1000

int test_array[NUM_TEST_VALUES];

#if 0
/* Tree print function - useful for debugging. */

static void print_tree(AVLNode *node, int depth)
{
    int *value;
    int i;

    if (node == NULL) {
        return;
    }

    print_tree(avl_tree_node_child(node, AVL_TREE_NODE_LEFT), depth + 1);

    for (i=0; i<depth*6; ++i) {
        printf(" ");
    }

    value = avl_tree_node_key(node);
    printf("%i\n", *value);

    print_tree(avl_tree_node_child(node, AVL_TREE_NODE_RIGHT), depth + 1);
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

int find_subtree_height(AVLNode *node) {
  AVLNode *left_subtree;
  AVLNode *right_subtree;
  int left_height, right_height;

  if (node == NULL) {
    return 0;
  }

  left_subtree = avl_tree_node_child(node, AVL_TREE_NODE_LEFT);
  right_subtree = avl_tree_node_child(node, AVL_TREE_NODE_RIGHT);
  left_height = find_subtree_height(left_subtree);
  right_height = find_subtree_height(right_subtree);

  if (left_height > right_height) {
    return left_height + 1;
  } else {
    return right_height + 1;
  }
}

/* Validates a subtree, returning its height */

int counter;

int validate_subtree(AVLNode *node) {
  AVLNode *left_node, *right_node;
  int left_height, right_height;
  int *key;

  if (node == NULL) {
    return 0;
  }

  left_node = avl_tree_node_child(node, AVL_TREE_NODE_LEFT);
  right_node = avl_tree_node_child(node, AVL_TREE_NODE_RIGHT);

  /* Check the parent references of the children */

  /* Recursively validate the left and right subtrees,
   * obtaining the height at the same time. */

  left_height = validate_subtree(left_node);

  /* Check that the keys are in the correct order */

  key = (int *) avl_tree_node_key(node);

  assert(*key > counter);
  counter = *key;

  right_height = validate_subtree(right_node);

  /* Check that the returned height value matches the
   * result of avl_tree_subtree_height(). */

  assert(avl_tree_subtree_height(left_node) == left_height);
  assert(avl_tree_subtree_height(right_node) == right_height);

  /* Check this node is balanced */

  assert(left_height - right_height < 2 &&
      right_height - left_height < 2);

  /* Calculate the height of this node */

  if (left_height > right_height) {
    return left_height + 1;
  } else {
    return right_height + 1;
  }
}

void validate_tree(AVLTree *tree) {
  AVLNode *root_node;
  int height;

  root_node = avl_tree_root_node(tree);

  if (root_node != NULL) {
    height = find_subtree_height(root_node);
    assert(avl_tree_subtree_height(root_node) == height);
  }

  counter = -1;
  validate_subtree(root_node);
}

AVLTree *create_tree(void) {
  AVLTree *tree;
  int i;

  /* Create a tree and fill with nodes */

  tree = new_avl_tree((AVLTreeComparator) int_compare);

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = i;
    avl_tree_put(tree, &test_array[i], &test_array[i]);
  }

  return tree;
}

void test_new_avl_tree(void) {
  AVLTree *tree;

  tree = new_avl_tree((AVLTreeComparator) int_compare);

  assert(tree != NULL);
  assert(avl_tree_size(tree) == 0);

  free_avl_tree(tree);

}

void test_avl_tree_put_lookup(void) {
  AVLTree *tree;
  AVLNode *node;
  unsigned int i;
  int *value;

  /* Create a tree containing some values. Validate the
   * tree is consistent at all stages. */

  tree = new_avl_tree((AVLTreeComparator) int_compare);

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    test_array[i] = (int) i;
    // printf("put (%d, %d) to avl\n", test_array[i], test_array[i]);
    avl_tree_put(tree, &test_array[i], &test_array[i]);
//    printf("    size of avl : %d\n", avl_tree_size(tree));
//    fflush(stdout);
    assert(avl_tree_size(tree) == i + 1);
    validate_tree(tree);
  }

  assert(avl_tree_root_node(tree) != NULL);

  /* Check that all values can be read back again */

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    node = avl_tree_get_node(tree, &i);
    assert(node != NULL);
    value = avl_tree_node_key(node);
    assert(*value == (int) i);
    value = avl_tree_node_value(node);
    assert(*value == (int) i);
  }

  /* Check that invalid nodes are not found */

  i = NUM_TEST_VALUES + 100;
  assert(avl_tree_get_node(tree, &i) == NULL);

  free_avl_tree(tree);
}

void test_avl_tree_child(void) {
  AVLTree *tree;
  AVLNode *root;
  AVLNode *left;
  AVLNode *right;
  int values[] = {1, 2, 3};
  int *p;
  int i;

  /* Create a tree containing some values. Validate the
   * tree is consistent at all stages. */

  tree = new_avl_tree((AVLTreeComparator) int_compare);

  for (i = 0; i < 3; ++i) {
    avl_tree_put(tree, &values[i], &values[i]);
  }

  /* Check the tree */

  root = avl_tree_root_node(tree);
  p = avl_tree_node_value(root);
  assert(*p == 2);

  left = avl_tree_node_child(root, AVL_TREE_NODE_LEFT);
  p = avl_tree_node_value(left);
  assert(*p == 1);

  right = avl_tree_node_child(root, AVL_TREE_NODE_RIGHT);
  p = avl_tree_node_value(right);
  assert(*p == 3);

  /* Check invalid values */

  assert(avl_tree_node_child(root, 10000) == NULL);
  assert(avl_tree_node_child(root, 2) == NULL);

  free_avl_tree(tree);
}


void test_free_avl_tree(void) {
  AVLTree *tree;

  /* Try freeing an empty tree */

  tree = new_avl_tree((AVLTreeComparator) int_compare);
  free_avl_tree(tree);

  /* Create a big tree and free it */

  tree = create_tree();
  free_avl_tree(tree);
}

void test_avl_tree_get(void) {
  AVLTree *tree;
  int i;
  int *value;

  /* Create a tree and look up all values */

  tree = create_tree();

  for (i = 0; i < NUM_TEST_VALUES; ++i) {
    value = avl_tree_get(tree, &i);

    assert(value != NULL);
    assert(*value == i);
  }

  /* Test invalid values */

  i = -1;
  assert(avl_tree_get(tree, &i) == NULL);
  i = NUM_TEST_VALUES + 1;
  assert(avl_tree_get(tree, &i) == NULL);
  i = 8724897;
  assert(avl_tree_get(tree, &i) == NULL);

  free_avl_tree(tree);
}

void test_avl_tree_remove(void) {
  AVLTree *tree;
  int i;
  int x, y, z;
  int value;
  unsigned int expected_entries;

  tree = create_tree();

  /* Try removing invalid entries */

  i = NUM_TEST_VALUES + 100;
  assert(avl_tree_remove(tree, &i) == AVL_TREE_REMOVED_UNCHANGED);
  i = -1;
  assert(avl_tree_remove(tree, &i) == AVL_TREE_REMOVED_UNCHANGED);

  /* Delete the nodes from the tree */

  expected_entries = NUM_TEST_VALUES;

  /* This looping arrangement causes nodes to be removed in a
   * randomish fashion from all over the tree. */

  for (x = 0; x < 10; ++x) {
    for (y = 0; y < 10; ++y) {
      for (z = 0; z < 10; ++z) {
        value = z * 100 + (9 - y) * 10 + x;
        assert(avl_tree_remove(tree, &value) == AVL_TREE_REMOVED_SUCCESS);
        validate_tree(tree);
        expected_entries -= 1;
        assert(avl_tree_size(tree)
                   == expected_entries);
      }
    }
  }

  /* All entries removed, should be empty now */

  assert(avl_tree_root_node(tree) == NULL);

  free_avl_tree(tree);
}

//void test_avl_tree_to_array(void)
//{
//	AVLTree *tree;
//	int entries[] = { 89, 23, 42, 4, 16, 15, 8, 99, 50, 30 };
//	int sorted[]  = { 4, 8, 15, 16, 23, 30, 42, 50, 89, 99 };
//	unsigned int num_entries = sizeof(entries) / sizeof(int);
//	unsigned int i;
//	int **array;
//
//	/* Add all entries to the tree */
//
//	tree = new_avl_tree((AVLTreeComparator) int_compare);
//
//	for (i=0; i<num_entries; ++i) {
//		avl_tree_put(tree, &entries[i], NULL);
//	}
//
//	assert(avl_tree_size(tree) == num_entries);
//
//	/* Convert to an array and check the contents */
//
//	array = (int **) avl_tree_to_array(tree);
//
//	for (i=0; i<num_entries; ++i) {
//		assert(*array[i] == sorted[i]);
//	}
//
//	free(array);
//
//	/* Test out of memory scenario */
//
//	alloc_test_set_limit(0);
//
//	array = (int **) avl_tree_to_array(tree);
//	assert(array == NULL);
//	validate_tree(tree);
//
//	free_avl_tree(tree);
//}

static UnitTestFunction tests[] = {
    test_new_avl_tree,
    test_free_avl_tree,
    test_avl_tree_child,
    test_avl_tree_put_lookup,
    test_avl_tree_get,
    test_avl_tree_remove,
//    //test_avl_tree_to_array,
    NULL
};

int main(int argc, char *argv[]) {
  run_tests(tests);
  return 0;
}


