#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>

/**
 * @file buffer.h
 * @brief 
 * @author Thomas Barrett
 */

typedef struct buffer {
    uint8_t *data;
    size_t length;
} buffer_t;

typedef buffer_t buffer_view_t;

/**
 * Create a new buffer of the specified length.
 * 
 * @param length the length of the new buffer
 * @return the buffer
 */
buffer_t buffer_create(size_t length);

/**
 * Create a new buffer containing a copy of the specified string
 * 
 * @param str the string to create a buffer from
 * @return the buffer
 */
buffer_t buffer_create_from_string(const char *str);

/**
 * Destroy the buffer by freeing its data.
 * 
 * @param buffer the buffer
 */
void buffer_destroy(buffer_t buffer);

/**
 * Compare the two buffers by lexographical order and return 0 if they are equal, 
 * some value < 0 if b1 < b2, and some value > 0 if b1 > b2.
 * 
 * @param buffer the buffer
 */
int buffer_compare(buffer_t b1, buffer_t b2);

/**
 * Create a new buffer which is an identical copy of b. 
 * 
 * @param b the buffer to copy
 * @return the new buffer
 */
buffer_t buffer_copy(buffer_t b);

/**
 * Concatenate buffers `a` and `b` into a new buffer, which is returned.
 * 
 * @param a the first buffer
 * @param b the second buffer
 * @return a buffer filled with the contents of a and b
 */
buffer_t buffer_concat(buffer_t a, buffer_t b);

/**
 * Resize the buffer `a` to the specified `length` by zero-extending the buffer as necessary to
 * grow it to the necessary length. This may change the value of a->data.
 * 
 * @param a the buffer to be resized
 * @param length the new length of `a`
 */
void buffer_resize(buffer_t *a, size_t length);

/**
 * Append the contents of `b` into `a`. The buffer `a` is resized, and the buffer `b` remains
 * unchanged.
 * 
 * @param a the buffer to be appended to
 * @param b the buffer to be appended
 */
void buffer_append(buffer_t *a, buffer_t b);

/**
 * Assign `res` to be the buffer containing the data from the ith byte of the buffer `a` to the
 * end of `a`.
 *
 * @param a: the buffer to slice
 * @param i: the start index of the slice
 * @param res: the result
 * @return -1 if an error occurs and 0 otherwise
 */
int buffer_slice(buffer_t a, size_t i, buffer_t *res);

int buffer_splice(buffer_t *a, size_t i);

/**
 * Return a null terminated string with a copy of the contents of `b`.
 *
 * @param b: the buffer to copy
 * @return a string with the contents of `b`
 */
char* buffer_to_string(buffer_t b);

#endif /* BUFFER_H */

