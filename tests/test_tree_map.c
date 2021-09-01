#include <test.h>
#include <tree_map.h>
#include <assert.h>
#include <string.h>
#include <log.h>

typedef struct node {
    struct node *left;
    struct node *right;
    uint8_t entry[];
} node_t;

typedef struct tree_map {
    node_t *root;
    size_t key_size;
    size_t val_size;
    compare_t cmp_key;
    destroy_t key_free;
    destroy_t val_free;
} tree_map_t;

int int_cmp(int *a, int *b) {
    return *a - *b;
}

void test_tree_map_create() {
    tree_map_t *map = tree_map_create(sizeof(int), sizeof(int), (compare_t) int_cmp, NULL, NULL);
    assert(map != NULL);
    tree_map_destroy(map);
}

void test_tree_map_set() {
    tree_map_t *map = tree_map_create(sizeof(int), sizeof(int), (compare_t) int_cmp, NULL, NULL);
    assert(map != NULL);
    assert(map->root == NULL);

    tree_map_set(map, &(int){1}, &(int){1});
    assert(map->root != NULL);
    assert(*(int*)map->root->entry == 1);
    
    tree_map_set(map, &(int){2}, &(int){4});
    assert(map->root != NULL);
    assert(map->root->right != NULL);
    assert(*(int*)map->root->right->entry == 2);

    tree_map_set(map, &(int){3}, &(int){9});
    assert(map->root != NULL);
    assert(map->root->right->right != NULL);
    assert(*(int*)map->root->right->right->entry == 3);

    assert(*(int*)tree_map_get(map, &(int){1}) == 1);
    assert(*(int*)tree_map_get(map, &(int){2}) == 4);
    assert(*(int*)tree_map_get(map, &(int){3}) == 9);
    assert(map->root != NULL);

    tree_map_destroy(map);
}

void test_tree_map_remove() {
    tree_map_t *map = tree_map_create(sizeof(int), sizeof(int), (compare_t) int_cmp, NULL, NULL);
    assert(map != NULL);
    assert(map->root == NULL);

    tree_map_set(map, &(int){2}, &(int){4});
    assert(map->root != NULL);
    assert(*(int*)map->root->entry == 2);

    tree_map_set(map, &(int){1}, &(int){1});
    assert(map->root != NULL);
    assert(map->root->left != NULL);
    assert(*(int*)map->root->left->entry == 1);
    
    tree_map_set(map, &(int){3}, &(int){9});
    assert(map->root != NULL);
    assert(map->root->right != NULL);
    assert(*(int*)map->root->right->entry == 3);

    assert(*(int*)tree_map_get(map, &(int){1}) == 1);
    assert(*(int*)tree_map_get(map, &(int){2}) == 4);
    assert(*(int*)tree_map_get(map, &(int){3}) == 9);
    assert(map->root != NULL);
    
    tree_map_remove(map, &(int){2});
    assert(map->root != NULL);
    assert(*(int*)map->root->entry == 3);
    assert(map->root->left != NULL);
    assert(*(int*)map->root->left->entry == 1);

    tree_map_remove(map, &(int){1});
    assert(map->root != NULL);
    assert(*(int*)map->root->entry == 3);
    assert(map->root->left == NULL);

    tree_map_remove(map, &(int){3});
    assert(map->root == NULL);

    tree_map_destroy(map);
}

int main(int argc, char *argv[]) {
    TEST(test_tree_map_create);
    TEST(test_tree_map_set);
    TEST(test_tree_map_remove);
}

