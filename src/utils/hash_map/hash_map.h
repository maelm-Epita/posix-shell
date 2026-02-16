#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define HASH_DEFAULT_CAPACITY 64

/**
 * @brief A key/value pair node used internally by the hash map.
 *
 * This is a chained list node (collision handling) and also participates
 * in an insertion-order linked list.
 *
 * @param key Key string
 * @param value Stored value pointer
 * @param next Next node in the bucket chain
 * @param insert_order_next Next node in insertion order
 */
struct pair_list
{
    char *key;
    void *value;
    struct pair_list *next;
    struct pair_list *insert_order_next;
    int index;
};

/**
 * @brief Generic hash map (string keys -> void* values).
 *
 * Uses separate chaining for collisions and keeps insertion order.
 *
 * @param data Bucket array
 * @param element_count Number of stored elements
 * @param capacity Number of buckets
 * @param insert_order_head Head of insertion-order list
 * @param insert_order_tail Tail of insertion-order list
 */
struct hash_map
{
    struct pair_list **data;
    size_t element_count;
    size_t list_count;
    size_t capacity;
    struct pair_list *insert_order_head;
    struct pair_list *insert_order_tail;
};

/**
 * @brief Creates a new hash map.
 * @return Newly allocated hash map, or NULL on failure
 */
struct hash_map *hash_map_init(void);

/**
 * @brief Inserts or replaces a key/value entry in the hash map.
 * @param hash_map Hash map instance
 * @param key Key string (must remain valid or be owned by the map depending on
 * implementation)
 * @param value Value pointer to store
 * @param free_func Optional destructor used when replacing an existing value
 * (can be NULL)
 * @return 0 on success, non-zero on error
 */
int hash_map_insert(struct hash_map *hash_map, char *key, void *value,
                    void (*free_func)(void *));

/**
 * @brief Retrieves the value associated with a key.
 * @param hash_map Hash map instance
 * @param key Key string
 * @return Stored value pointer, or NULL if not found
 */
void *hash_map_get(struct hash_map *hash_map, char *key);

int hash_map_remove(struct hash_map *hash_map, char *key,
                    void (*free_func)(void *));

/**
 * @brief Frees a hash map and all its entries.
 * @param hash_map Hash map instance
 * @param free_func Optional destructor called on each stored value (can be
 * NULL)
 */
void hash_map_free(struct hash_map *hash_map, void (*free_func)(void *));

/**
 * @brief Dumps the hash map content (debug helper).
 * @param hash_map Hash map instance
 */
void hash_map_dump(struct hash_map *hash_map, void (*print_func)(void *));

#endif /* ! HASH_MAP_H */
