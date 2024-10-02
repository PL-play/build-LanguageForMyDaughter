//
// Created by ran on 2024/1/1.
//

#ifndef ZCOLLECTION_ARRAY_LIST_H_
#define ZCOLLECTION_ARRAY_LIST_H_
#ifdef __cplusplus
extern "C" {
#endif
#define ARRAYLIST_ERROR (-1)
#define ARRAYLIST_SUCCESS 0
#define ARRAYLIST_INVALID_POSITION 1
#define ARRAYLIST_POSITION_EMPTY 2

/**
 * The value in @ref ArrayList.
 */
typedef void *ArrayListData;

/**
 * The ArrayList structure.
 */
typedef struct ArrayList ArrayList;

struct ArrayList {
  ArrayListData *data;
  int size;
  int capacity;
};

/**
 * Compare two data.
 * @return Returns zero if they are equal. Positive if data1 is bigger than data2, negative otherwise.
 */
typedef int (*ArrayListDataCompare)(ArrayListData data1, ArrayListData data2);

/**
 * Initialize a new ArrayList.
 *
 * @param init_capacity     Indicate the size of the capacity.
 *                          If zero is given, a default value is used.
 * @return                  A new arraylist or NULL if there was no memory to allocate.
 */
ArrayList *new_arraylist(int init_capacity);

/**
 * Destroy the ArrayList and free the memory it uses.
 *
 * @param array_list    The ArrayList to free.
 */
void free_arraylist(ArrayList *array_list);

/**
 * Insert a data into the ArrayList with the specific index.
 * The index should be in the size of the ArrayList.
 *
 * @param array_list    The ArrayList.
 * @param index         The index where the value to be inserted.
 * @param data          The data.
 * @return              Returns zero if successful, else no-zero if unsuccessful.
 */
int insert_arraylist(ArrayList *array_list, int index, ArrayListData data);

/**
 * Append data to the end of the ArrayList.
 *
 * @param array_list    The ArrayList.
 * @param data          The data to append.
 * @return              Returns zero if successful, else no-zero if unsuccessful.
 */
int append_arraylist(ArrayList *array_list, ArrayListData data);

/**
 * Prepend data to the beginning of the ArrayList.
 *
 * @param array_list    The ArrayList.
 * @param data          The data to prepend.
 * @return              Returns zero if successful, else no-zero if unsuccessful.
 */
int prepend_arraylist(ArrayList *array_list, ArrayListData data);

/**
 * Remove the entry at the specific location in the ArrayList.
 *
 * @param array_list   The ArrayList
 * @param index        The index of the data to remove.
 * @return             Returns zero if successful, else no-zero if unsuccessful.
 */
int remove_arraylist(ArrayList *array_list, int index);

/**
 * Find the index of the given data.
 *
 * @param array_list    The ArrayList to search.
 * @param data          The data to find.
 * @param comparator    The comparator to determine equality.
 * @return              The index of the data if founded, otherwise -1.
 */
int index_of_arraylist(ArrayList *array_list, ArrayListData data, ArrayListDataCompare comparator);

/**
 * Get the data at specific index of the ArrayList.
 *
 * @param array_list    The ArrayList.
 * @param index         The index of data to get.
 * @return              The data. NULL if index is out of range.
 */
ArrayListData get_data_arraylist(ArrayList *array_list, int index);

/**
 * Set the data to the specific index of the ArrayList.
 *
 * @param array_list    The ArrayList.
 * @param index         The index to be set.
 * @param data          The data to set.
 * @return              Returns zero f successful, else no-zero if unsuccessful.
 */
int set_arraylist(ArrayList *array_list, int index, ArrayListData data);

/**
 * Clear the ArrayList.
 *
 * @param array_list    The ArrayList.
 */
void clear_arraylist(ArrayList *array_list);

/**
 * Sort the ArrayList.
 *
 * @param array_list    The ArrayList.
 * @param comparator    The comparator for sort.
 */
void sort_arraylist(ArrayList *array_list, ArrayListDataCompare comparator);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_ARRAY_LIST_H_
