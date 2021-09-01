#include <buffer.h>

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

buffer_t buffer_create(size_t length) {
    uint8_t *data = (uint8_t*) calloc(1, length + 1);
    assert(data != NULL && "out of memory");
    return (buffer_t) {data, length};
}

buffer_t buffer_create_from_string(const char *str) {
    size_t len = strlen(str);
    uint8_t *data = (uint8_t*) calloc(1, len + 1);
    assert(data != NULL && "out of memory");
    memcpy(data, str, len);
    return (buffer_t) {data, len};
}

void buffer_destroy(buffer_t buffer) {
    free(buffer.data);
}

buffer_t buffer_copy(buffer_t b) {
    uint8_t *data = (uint8_t*) calloc(1, b.length + 1);
    assert(data != NULL && "out of memory");
    memcpy(data, b.data, b.length);
    return (buffer_t) {data, b.length};
}

int buffer_compare(buffer_t b1, buffer_t b2) {
    int len = b1.length < b2.length ? b1.length: b2.length;
    int cmp = memcmp(b1.data, b2.data, len);
    if (b1.length == b2.length || cmp != 0) return cmp;
    else return b1.length - b2.length;
}

buffer_t buffer_concat(buffer_t a, buffer_t b) {
    uint8_t *data = (uint8_t*) calloc(1, a.length + b.length + 1);
    assert(data != NULL && "out of memory");
    memcpy(data, a.data, a.length);
    memcpy(data + a.length, b.data, b.length);
    return (buffer_t) {data, a.length + b.length};
}

void buffer_resize(buffer_t *a, size_t length) {
    a->data = (uint8_t*) realloc(a->data, length + 1);
    assert(a->data != NULL && "out of memory");
    if (length > a->length) {
        memset(a->data + a->length, 0, length - a->length + 1);
    }
    a->length = length;
}

void buffer_append(buffer_t *a, buffer_t b) {
    size_t len = a->length;
    buffer_resize(a, a->length + b.length);
    memcpy(a->data + len, b.data, b.length);
}

int buffer_slice(buffer_t a, size_t i, buffer_t *res) {
    if (res == NULL) return -1;
    if (i > a.length) return -1;
    res->data = a.data + i;
    res->length = a.length - i;
    return 0;
}

char* buffer_to_string(buffer_t b) {
    char *res = calloc(1, b.length + 1);
    assert (res != NULL && "out of memory");
    memcpy(res, b.data, b.length);
    return res;
}

int buffer_splice(buffer_t *a, size_t i) {
   if (a == NULL) return -1;
   if (i > a->length) return -1;
   memmove(a->data, a->data + i, a->length - i);
   a->length -= i;
   a->data[a->length] = '\0';
   return 0;
}

