#include <test.h>
#include <list.h>
#include <assert.h>
#include <string.h>

void test_list_create() {
    list_t* list1 = list_create(0);
    assert(list1 != NULL);
    assert(list_size(list1) == 0);
    list_destroy(list1, NULL);

    list_t* list2 = list_create(32);
    assert(list2 != NULL);
    assert(list_size(list2) == 0);
    list_destroy(list2, NULL);
}

void test_list_destroy() {
    char *str1 = calloc(1, 16);
    char *str2 = calloc(1, 16);
    strcpy(str1, "str1");
    strcpy(str2, "str2");

    list_t* list = list_create(0);
    list_add(list, str1);
    list_add(list, str2);

    list_destroy(list, free);
}

void test_list_add() {
    list_t* list = list_create(1);
    list_add(list, (void*) 10);
    assert(list_size(list) == 1);
    list_add(list, (void*) 2);
    assert(list_size(list) == 2);
    assert(list_get(list, 0) == (void *) 10); 
    assert(list_get(list, 1) == (void *) 2); 
    list_destroy(list, NULL);
}


void test_list_remove() {
    list_t* list = list_create(1);
    list_add(list, (void*) 1);
    list_add(list, (void*) 2);
    list_add(list, (void*) 3);
    list_add(list, (void*) 4);

    assert(list_size(list) == 4);
    assert(list_remove(list, 1) == (void *) 2);
    assert(list_size(list) == 3);
    assert(list_get(list, 0) == (void*) 1);
    assert(list_get(list, 1) == (void*) 3);
    assert(list_get(list, 2) == (void*) 4);

    assert(list_remove(list, 0) == (void *) 1);
    assert(list_size(list) == 2);
    assert(list_get(list, 0) == (void*) 3);
    assert(list_get(list, 1) == (void*) 4);

    assert(list_remove(list, 1) == (void *) 4);
    assert(list_size(list) == 1);
    assert(list_get(list, 0) == (void*) 3);
    list_destroy(list, NULL);
}

void test_list_find() {
    list_t* list = list_create(1);
    list_add(list, (void*) 1);
    list_add(list, (void*) 2);
    list_add(list, (void*) 3);
    list_add(list, (void*) 4);

    assert(list_size(list) == 4);
    assert(list_find(list, (void *) 1, NULL) == 0);
    assert(list_find(list, (void *) 3, NULL) == 2);
    assert(list_find(list, (void *) 4, NULL) == 3);
    assert(list_find(list, (void *) 2, NULL) == 1);
    list_destroy(list, NULL);

    char *str1 = calloc(1, 16);
    char *str2 = calloc(1, 16);
    strcpy(str1, "str1");
    strcpy(str2, "str2");

    list_t* list2 = list_create(0);
    list_add(list2, str1);
    list_add(list2, str2);
    assert(list_find(list2, "str1", (compare_t) strcmp) == 0);
    assert(list_find(list2, "str2", (compare_t) strcmp) == 1);
    list_destroy(list2, free);
}

int main(int argc, char *argv[]) {
    TEST(test_list_create);
    TEST(test_list_destroy);
    TEST(test_list_add);
    TEST(test_list_remove);
    TEST(test_list_find);
}

