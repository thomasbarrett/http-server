#include <tree_map.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

tree_map_t* tree_map_create(
    size_t key_size,
    size_t val_size, 
    compare_t cmp_key,
    destroy_t key_free,
    destroy_t val_free
) {

    tree_map_t *res = calloc(1, sizeof(tree_map_t));
    assert (res != NULL && "out of memory");
    res->key_size = key_size;
    res->val_size = val_size;
    res->cmp_key = cmp_key;
    res->key_free = key_free;
    res->val_free = val_free; 
    return res;
}

static node_t* node_create(tree_map_t *map, void *key, void *val) {
    node_t *node = calloc(1, sizeof(node_t) + map->key_size + map->val_size);
    assert(node != NULL && "out of memory");
    memcpy(node->entry, key, map->key_size);
    memcpy(node->entry + map->key_size, val, map->val_size);
    return node;
}

static void node_destroy(tree_map_t *map, node_t *node) {
    if (node == NULL) return;
    if (map->key_free) map->key_free(node->entry);
    if (map->val_free) map->val_free(node->entry + map->key_size);
    free(node);
}

static void node_assign(tree_map_t *map, node_t *node, void *key, void *val) {
    if (map->key_free) map->key_free(node->entry);
    if (map->val_free) map->val_free(node->entry + map->key_size);
    memmove(node->entry, key, map->key_size);
    memmove(node->entry + map->key_size, val, map->val_size);
}

static void node_destroy_recursive(tree_map_t *map, node_t *node) {
    if (node == NULL) return;
    if (map->key_free) map->key_free(node->entry);
    if (map->val_free) map->val_free(node->entry + map->key_size);
    node_destroy_recursive(map, node->left);
    node_destroy_recursive(map, node->right);
    free(node);
}

void tree_map_destroy(tree_map_t *map) {
    node_destroy_recursive(map, map->root);
    free(map);
}

static node_t* node_search(tree_map_t *m, node_t *root, void *key) {
    if (root == NULL) return NULL;
    int cmp = m->cmp_key(root->entry, key);
    if (cmp == 0) return root;
    else if (cmp > 0) return node_search(m, root->left, key);
    else return node_search(m, root->right, key);
}

static node_t* parent_search(tree_map_t *m, node_t *parent, node_t *root, void *key) {
    if (root == NULL) return parent;
    int cmp = m->cmp_key(root->entry, key);
    if (cmp == 0) return parent;
    else if (cmp > 0) return parent_search(m, root, root->left, key);
    else return parent_search(m, root, root->right, key);
}

void* tree_map_get(tree_map_t *map, void *key) {
    node_t* node = node_search(map, map->root, key);
    return node == NULL ? NULL: node->entry + map->key_size;
}

void tree_map_set(tree_map_t *map, void *key, void *val) {
    // If there are no entries in map, create root node.
    if (map->root == NULL) {
        map->root = node_create(map, key, val);
        return;
    }

    // If there is no parent, assign to root node.
    node_t* parent = parent_search(map, NULL, map->root, key);
    if (parent == NULL) {
        assert(map != NULL);
        assert(map->root != NULL);
        node_assign(map, map->root, key, val);
        return;
    }

    int cmp = map->cmp_key(parent->entry, key);
    if (cmp > 0) {
        if (parent->left == NULL) parent->left = node_create(map, key, val);
        else node_assign(map, parent->left, key, val);
    } else {
        if (parent->right == NULL) parent->right = node_create(map, key, val);
        else node_assign(map, parent->right, key, val);
    }
}

typedef struct result {
    node_t* parent;
    node_t* child;
} result_t;

static result_t node_find_largest(node_t *parent, node_t *node) {
    if (node->right == NULL) return (result_t) {parent, node};
    else return node_find_largest(node, node->right);
}

static void node_remove(node_t *parent, node_t *node) {
    if (node->left == NULL && node->right == NULL) {
        if (parent->left == node) parent->left = NULL;
        if (parent->right == node) parent->right = NULL;
    } else if (node->left != NULL && node->right != NULL) {
        result_t res = node_find_largest(node, node->right);
        node_remove(res.parent, res.child);
        res.child->left = node->left;
        res.child->right = node->right;
        if (parent->left == node) parent->left = res.child;
        if (parent->right == node) parent->right = res.child;
    } else if (node->left != NULL) {
        if (parent->left == node) parent->left = node->left;
        if (parent->right == node) parent->right = node->left;
    } else if (node->right != NULL) {
        if (parent->left == node) parent->left = node->right;
        if (parent->right == node) parent->right = node->right;
    }
    node->left = NULL;
    node->right = NULL;
}

void tree_map_remove(tree_map_t *map, void *key) {
    if (map->root == NULL) {
        return;
    }

    node_t* parent = parent_search(map, NULL, map->root, key);
    if (parent == NULL) {
        node_t *node = map->root;
        if (node->left == NULL && node->right == NULL) {
            map->root = NULL;
        } else if (node->left != NULL && node->right != NULL) {
            result_t res = node_find_largest(node, node->right);
            node_remove(res.parent, res.child);
            res.child->left = map->root->left;
            res.child->right = map->root->right;
            map->root = res.child;
        } else if (node->left != NULL) {
            map->root = node->left;
        } else if (node->right != NULL) {
            map->root = node->right;
        }

        node_destroy(map, node);
        return;
    }

    int cmp = map->cmp_key(parent->entry, key);
    node_t *node = cmp > 0 ? parent->left: parent->right;
    node_remove(parent, node);
    node_destroy(map, node);
}

