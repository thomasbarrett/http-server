#include <test.h>
#include <array.h>
#include <assert.h>
#include <string.h>


struct example {
    int arg1;
    char *arg2;
};

void example_destroy(struct example *x) {
    free(x->arg2);
}

int example_compare(struct example *a, struct example *b) {
    if (a->arg1 != b->arg1) return a->arg1 - b->arg1;
    return strcmp(a->arg2, b->arg2);
}

void test_array_create() {
    array_t* array1 = array_create(sizeof(int), 0);
    assert(array1 != NULL);
    assert(array_size(array1) == 0);
    array_destroy(array1, NULL);

    array_t* array2 = array_create(sizeof(int), 32);
    assert(array2 != NULL);
    assert(array_size(array2) == 0);
    array_destroy(array2, NULL);
}

void test_array_destroy() { 
    char *str1 = calloc(1, 16);
    char *str2 = calloc(1, 16);
    strcpy(str1, "str1");
    strcpy(str2, "str2");
    array_t* array = array_create(sizeof(struct example), 0);
    array_add(array, &(struct example){1, str1});
    array_add(array, &(struct example){2, str2});
    array_destroy(array, (destroy_t) example_destroy);
}

void test_array_add() {
    array_t* array = array_create(sizeof(int), 1);
    array_add(array, &(int){10});
    assert(array_size(array) == 1);
    array_add(array, &(int){2});
    assert(array_size(array) == 2);
    assert(*(int*)array_get(array, 0) == 10); 
    assert(*(int*)array_get(array, 1) == 2); 
    array_destroy(array, NULL);
}

void test_array_remove() {
    array_t* array = array_create(sizeof(int), 1);
    array_add(array, &(int){1});
    array_add(array, &(int){2});
    array_add(array, &(int){3});
    array_add(array, &(int){4});
    assert(array_size(array) == 4);

    array_remove(array, 1);
    assert(array_size(array) == 3);
    assert(*(int*)array_get(array, 0) == 1);
    assert(*(int*)array_get(array, 1) == 3);
    assert(*(int*)array_get(array, 2) == 4);

    array_remove(array, 0);
    assert(array_size(array) == 2);
    assert(*(int*)array_get(array, 0) == 3);
    assert(*(int*)array_get(array, 1) == 4);

    array_remove(array, 1);
    assert(array_size(array) == 1);
    assert(*(int*)array_get(array, 0) == 3);
    array_destroy(array, NULL);
}

void test_array_find() {
    array_t* array = array_create(sizeof(int), 1);
    array_add(array, &(int){1});
    array_add(array, &(int){2});
    array_add(array, &(int){3});
    array_add(array, &(int){4});

    assert(array_size(array) == 4);
    assert(array_find(array, &(int){1}, NULL) == 0);
    assert(array_find(array, &(int){3}, NULL) == 2);
    assert(array_find(array, &(int){4}, NULL) == 3);
    assert(array_find(array, &(int){2}, NULL) == 1);
    array_destroy(array, NULL);

    char *str1 = calloc(1, 16);
    char *str2 = calloc(1, 16);
    strcpy(str1, "str1");
    strcpy(str2, "str2");

    array_t* array2 = array_create(sizeof(struct example), 0);
    array_add(array2, &(struct example){1, str1});
    array_add(array2, &(struct example){2, str2});
    assert(array_find(array2, &(struct example){1, "str1"}, (compare_t) strcmp) == 0);
    assert(array_find(array2, &(struct example){2, "str2"}, (compare_t) strcmp) == 1);
    array_destroy(array2, (destroy_t) example_destroy);
}

int main(int argc, char *argv[]) {
    TEST(test_array_create);
    TEST(test_array_destroy);
    TEST(test_array_add);
    TEST(test_array_remove);
    TEST(test_array_find);
}

