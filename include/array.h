#ifndef array_H
#define array_H

#include <stddef.h>

/**
 * The array_t container is an implementation of a generic array array. It 
 * provides constant time get and add operations, and linear time
 * lookup, insert, and delete operations.
 */
typedef struct array array_t;

/**
 * The compare_t function type represents a class of functions that order
 * a elements in the following manner: given two elements a and b, return
 * a negative number if a < b, 0 if a = b, and a positive number if a > b.
 */
typedef int (*compare_t)(void*, void*);
typedef void (*destroy_t)(void*);

/**
 * Create a generic array array with the given initial capacity
 * @param initial_capacity the expected number of elements
 */
array_t* array_create(size_t element_size, size_t initial_capacity);

/**
 * Destroy the contents of the array with the given destructor, free
 * the array array and all allocated memory.
 * 
 * @param destroy an element destructor: unused if null
 */
void array_destroy(array_t *array, void (*destroy)(void *));

/**
 * Return a pointer to the underlying buffer. The elements are stored contiguously
 * in memory with a stride of `element_size` as defined in the constructor. 
 * Note that it is not safe to reference the buffer after adding elements
 * since the buffer may be reallocated while adding elements.
 * 
 * @param array the array
 */
void* array_data(array_t *array);

/**
 * Return the size of the array.
 * 
 * @return the size of the array
 */
size_t array_size(array_t *array);

/**
 * Add the element to the back of the array.
 * 
 * @return the ith element. 
 */
void array_add(array_t *array, void *e);

/**
 * Return the ith element in the array. Assert that the index is valid.
 * 
 * @return the ith element. 
 */
void* array_get(array_t *array, size_t i);

/**
 * Remove the ith element from the array and return the element.
 * Assert that the index is valid.
 * 
 * @return the ith element.
 */
void array_remove(array_t *array, size_t i);

/**
 * Return the index of the element e in the array by performing a linear search
 * with the given compare function. If the element is not found, this function
 * will return array_size(array).
 * 
 * @param array the array
 * @param e the element
 * @param cmp the comparison function
 */
size_t array_find(array_t *array, void *e, compare_t cmp);

#endif /* array_H */

