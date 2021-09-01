#include <list.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

struct list {
    void **data;
    size_t size;
    size_t capacity;
};

list_t* list_create(size_t initial_capacity) {
    assert(initial_capacity >= 0);
    list_t *res = malloc(sizeof(list_t));
    assert(res != NULL);
    res->capacity = initial_capacity < 1 ? 1: initial_capacity;
    res->data = malloc(res->capacity * sizeof(void*));
    assert(res->data != NULL);
    res->size = 0;
    return res;
}

void list_destroy(list_t *list, void (*destroy)(void *)) {
    assert(list != NULL);
    if (destroy != NULL) {
        for (size_t i = 0; i < list->size; i++) {
            destroy(list->data[i]);
        }
    }
    free(list->data);
    free(list);
}

size_t list_size(list_t *list) {
    assert(list != NULL);
    return list->size;
}

void list_add(list_t *list, void *e) {
    assert(list != NULL);
    if (list->size < list->capacity) {
        list->data[list->size] = e;
        list->size += 1;
    } else {
        list->data = realloc(list->data, sizeof(void *) * list->capacity * 2);
        assert(list->data != NULL);
        list->data[list->size] = e;
        list->size += 1;
        list->capacity *= 2;
    }
}

void* list_get(list_t *list, size_t i) {
    assert(list != NULL);
    assert(0 <= i && i < list->size);
    return list->data[i];
}

void* list_remove(list_t *list, size_t i) {
    assert(list != NULL);
    assert(0 <= i && i < list->size);
    void *result = list->data[i];
    memmove(&list->data[i], &list->data[i + 1], sizeof(void*) * (list->size - 1 - i));
    list->size -= 1;
    return result;
}

size_t list_find(list_t *list, void *e, int (*cmp)(void*, void*)) {
    assert(list != NULL);
    if (cmp == NULL) {
        for (size_t i = 0; i < list->size; i++) {
            if (list->data[i] == e) return i;
        }
    } else {
        for (size_t i = 0; i < list->size; i++) {
            if (cmp(list->data[i], e) == 0) return i;
        }
    }
    return list->size;
}

