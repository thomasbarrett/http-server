#include <array.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

struct array {
    uint8_t *data;
    size_t element_size;
    size_t size;
    size_t capacity;
};

array_t* array_create(size_t element_size, size_t initial_capacity) {
    assert(element_size > 0);
    assert(initial_capacity >= 0);
    array_t *res = malloc(sizeof(array_t));
    assert(res != NULL);
    res->element_size = element_size;
    res->capacity = initial_capacity < 1 ? 1: initial_capacity;
    res->data = malloc(res->capacity * element_size);
    assert(res->data != NULL);
    res->size = 0;
    return res;
}

void* array_data(array_t *array) {
    assert(array != NULL);
    return array->data;
}

void array_destroy(array_t *array, void (*destroy)(void *)) {
    assert(array != NULL);
    if (destroy != NULL) {
        for (size_t i = 0; i < array->size; i++) {
            destroy(&array->data[i * array->element_size]);
        }
    }
    free(array->data);
    free(array);
}

size_t array_size(array_t *array) {
    assert(array != NULL);
    return array->size;
}

void array_add(array_t *array, void *e) {
    assert(array != NULL);
    if (array->size < array->capacity) {
        memcpy(&array->data[array->size * array->element_size], e, array->element_size);
        array->size += 1;
    } else {
        array->data = realloc(array->data, array->element_size * array->capacity * 2);
        assert(array->data != NULL);
        memcpy(&array->data[array->size * array->element_size], e, array->element_size);
        array->size += 1;
        array->capacity *= 2;
    }
}

void* array_get(array_t *array, size_t i) {
    assert(array != NULL);
    assert(0 <= i && i < array->size);
    return &array->data[i * array->element_size];
}

void array_remove(array_t *array, size_t i) {
    assert(array != NULL);
    assert(0 <= i && i < array->size);
    size_t s = array->element_size;
    memmove(&array->data[i * s], &array->data[(i + 1) * s], s * (array->size - 1 - i));
    array->size -= 1;
}

size_t array_find(array_t *array, void *e, int (*cmp)(void*, void*)) {
    assert(array != NULL);
    size_t s = array->element_size;
    if (cmp == NULL) {
        for (size_t i = 0; i < array->size; i++) {
            if (memcmp(&array->data[i * s], e, s) == 0) return i;
        }
    } else {
        for (size_t i = 0; i < array->size; i++) {
            if (cmp(&array->data[i * s], e) == 0) return i;
        }
    }
    return array->size;
}

