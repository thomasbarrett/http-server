#ifndef LIST_H
#define LIST_H

#include <stddef.h>

/**
 * The list_t container is an implementation of a generic array list. It 
 * provides constant time get and add operations, and linear time
 * lookup, insert, and delete operations.
 */
typedef struct list list_t;

/**
 * The compare_t function type represents a class of functions that order
 * a elements in the following manner: given two elements a and b, return
 * a negative number if a < b, 0 if a = b, and a positive number if a > b.
 */
typedef int (*compare_t)(void*, void*);

/**
 * Create a generic array list with the given initial capacity
 * @param initial_capacity the expected number of elements
 */
list_t* list_create(size_t initial_capacity);

/**
 * Destroy the contents of the array list with the given destructor, free
 * the array list and all allocated memory.
 * 
 * @param destroy an element destructor: unused if null
 */
void list_destroy(list_t *list, void (*destroy)(void *));

/**
 * Return the size of the list.
 * 
 * @return the size of the list
 */
size_t list_size(list_t *list);

/**
 * Add the element to the back of the list.
 * 
 * @return the ith element. 
 */
void list_add(list_t *list, void *e);

/**
 * Return the ith element in the list. Assert that the index is valid.
 * 
 * @return the ith element. 
 */
void* list_get(list_t *list, size_t i);

/**
 * Remove the ith element from the list and return the element.
 * Assert that the index is valid.
 * 
 * @return the ith element.
 */
void* list_remove(list_t *list, size_t i);

/**
 * Return the index of the element e in the list by performing a linear search
 * with the given compare function. If the element is not found, this function
 * will return list_size(list).
 * 
 * @param list the list
 * @param e the element
 * @param cmp the comparison function
 */
size_t list_find(list_t *list, void *e, compare_t cmp);

#endif /* LIST_H */

