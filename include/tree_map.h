#ifndef TREE_MAP_H
#define TREE_MAP_H

/**
 * @file tree_map.h
 * @brief A generic tree map implementation 
 * @author Thomas Barrett
 */

#include <stddef.h>
#include <stdbool.h>

typedef struct tree_map tree_map_t;

typedef int (*compare_t)(void*, void*);
typedef void (*destroy_t)(void*);

/**
 * Create a new map with with key type and value type of the given size.
 * 
 * @param key_size the size of the key type in bytes
 * @param val_size the size of the val type in bytes.
 * @param key_cmp the key type comparison function
 * @param key_free the key type destructor
 * @param val_free the val type destructor
 * @return a new tree map
 */
tree_map_t* tree_map_create(
    size_t key_size,
    size_t val_size, 
    compare_t cmp_key,
    destroy_t key_free,
    destroy_t val_free
);

/**
 * Destroy the map and free all memory owned by the map.
 * 
 * @param map
 */
void tree_map_destroy(tree_map_t *map);

/**
 * Return the value associated with the given key or null if no such value exists.
 * 
 * @param map the map
 * @param key the key
 */
void* tree_map_get(tree_map_t *map, void *key);

/**
 * Store the key value pair in the map. If there is already a value with the same key,
 * destroy it and replace it with the new key and value.
 * 
 * @param map the map
 * @param key the key
 * @param val the val
 */
void tree_map_set(tree_map_t *map, void *key, void *val);

/**
 * Remove the value with the given key from the map if it exists.
 * 
 * @param map the map
 * @param key the key to remove
 */
void tree_map_remove(tree_map_t *map, void *key);

#endif /* TREE_MAP_H */

