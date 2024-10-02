//
// Created by ran on 2024/1/1.
//
#include "list/array_list.h"
#include <stdlib.h>
#include <string.h>


#define ARRAYLIST_DEFAULT_CAPACITY 8

static int grow_capacity(ArrayList *array_list);

static void quick_sort_data(ArrayListData *array_list_data,
                            ArrayListDataCompare comparator,
                            int l,
                            int r);
static int partition_data(ArrayListData *array_list_data,
                          ArrayListDataCompare comparator,
                          int l,
                          int r);

ArrayList *new_arraylist(int init_capacity) {
  if (init_capacity == 0) {
    init_capacity = ARRAYLIST_DEFAULT_CAPACITY;
  }

  ArrayList *array_list = malloc(sizeof(ArrayList));
  if (array_list == NULL) {
    return NULL;
  }

  array_list->data = malloc(sizeof(ArrayListData) * init_capacity);
  if (array_list->data == NULL) {
    free(array_list);
    return NULL;
  }

  array_list->capacity = init_capacity;
  array_list->size = 0;

  return array_list;
}

void free_arraylist(ArrayList *array_list) {
  if (array_list != NULL) {
    free(array_list->data);
    free(array_list);
  }
}

int insert_arraylist(ArrayList *array_list, int index, ArrayListData data) {
  if (array_list->size >= array_list->capacity) {
    int ret = grow_capacity(array_list);
    if (ret != ARRAYLIST_SUCCESS) {
      return ret;
    }
  }
  if (index > array_list->size) {
    return ARRAYLIST_INVALID_POSITION;
  }
  if (index != array_list->size) {
    memmove(&array_list->data[index + 1],
            &array_list->data[index],
            (array_list->size - index) * sizeof(ArrayListData));
  }
  array_list->data[index] = data;
  array_list->size++;
  return ARRAYLIST_SUCCESS;
}

int append_arraylist(ArrayList *array_list, ArrayListData data) {
  return insert_arraylist(array_list, array_list->size, data);
}

int prepend_arraylist(ArrayList *array_list, ArrayListData data) {
  return insert_arraylist(array_list, 0, data);
}

int remove_arraylist(ArrayList *array_list, int index) {
  if (index == array_list->size - 1) {
    array_list->data[index] = NULL;
    array_list->size--;
    return ARRAYLIST_SUCCESS;
  }
  if (index >= array_list->size) {
    return ARRAYLIST_INVALID_POSITION;
  }
  memmove(&array_list->data[index],
          &array_list->data[index + 1],
          (array_list->size - (index + 1)) * sizeof(ArrayListData));
  array_list->size--;
  return ARRAYLIST_SUCCESS;
}

int index_of_arraylist(ArrayList *array_list, ArrayListData data, ArrayListDataCompare comparator) {
  for (int i = 0; i < array_list->size; i++) {
    if (comparator(data, array_list->data[i]) == 0) {
      return i;
    }
  }
  return -1;
}

ArrayListData get_data_arraylist(ArrayList *array_list, int index) {
  if (index >= array_list->size) {
    return NULL;
  }
  return array_list->data[index];
}

int set_arraylist(ArrayList *array_list, int index, ArrayListData data) {
  if (index >= array_list->size) {
    return ARRAYLIST_INVALID_POSITION;
  }
  array_list->data[index] = data;
  return ARRAYLIST_SUCCESS;
}

void clear_arraylist(ArrayList *array_list) {
  array_list->size = 0;
}

void sort_arraylist(ArrayList *array_list, ArrayListDataCompare comparator) {
  if (array_list->size == 0) return;
  quick_sort_data(array_list->data, comparator, 0, array_list->size - 1);
}

static int grow_capacity(ArrayList *array_list) {
  // hope won't overflow
  unsigned int new_capacity = array_list->capacity * 2;
  ArrayListData *newData = realloc(array_list->data, sizeof(ArrayListData) * new_capacity);
  if (newData == NULL) {
    return ARRAYLIST_ERROR;
  }
  array_list->data = newData;
  array_list->capacity = new_capacity;
  return ARRAYLIST_SUCCESS;
}

/**
 *
 *
 * @param array_list_data
 * @param comparator
 * @param l
 * @param r
 */
static void quick_sort_data(ArrayListData *array_list_data,
                            ArrayListDataCompare comparator,
                            int l,
                            int r) {
  if (l >= r) return;
  int p = partition_data(array_list_data, comparator, l, r);
  quick_sort_data(array_list_data, comparator, l, p - 1);
  quick_sort_data(array_list_data, comparator, p + 1, r);

}

/**
 *
 * 2 way quick sort.
 *
 * arr[l+1 ... i-1)<=v          arr(j...r]>=v
 * --------------------------------------------
 *      <=v                         >=v
 * |v|*************|          |#############|
 * ^                ^       ^               ^
 * l                i       j               r
 *
 * swap(l,j) and return j as partition.
 *
 * @param array_list_data
 * @param comparator
 * @param l
 * @param r
 */
static int partition_data(ArrayListData *array_list_data,
                          ArrayListDataCompare comparator,
                          int l,
                          int r) {
#define SWAP(a, b) { \
                      ArrayListData t = array_list_data[a]; \
                      array_list_data[a] = array_list_data[b]; \
                      array_list_data[b]=t;                 \
                   }
  int mid = l + (r - l) / 2;
  SWAP(l, mid)

  ArrayListData p = array_list_data[l];
  int i = l + 1;
  int j = r;
  while (1) {
    while (i <= r && comparator(array_list_data[i], p) < 0) i++;
    while (j >= l + 1 && comparator(array_list_data[j], p) > 0) j--;
    if (i > j) break;
    SWAP(i, j)
    i++;
    j--;
  }
  SWAP(l, j)
  return j;
#undef SWAP
}