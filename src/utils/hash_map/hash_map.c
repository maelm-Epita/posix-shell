#include "hash_map.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -- AUX -- */
static struct pair_list *find_key(struct pair_list *list, char *key)
{
    while (list != NULL && strcmp(list->key, key) != 0)
    {
        list = list->next;
    }
    return list;
}

static size_t hash(const char *key)
{
    if (!key)
        return 0;

    uint32_t hash = 2166136261; // FNV offset basis
    uint32_t prime = 16777619; // FNV prime

    while (*key)
    {
        hash ^= *key;
        hash *= prime;
        key++;
    }

    return hash;
}

static void hash_map_rehash(struct hash_map *hash_map,
                            void (*free_func)(void *))
{
    hash_map->element_count = 0;
    hash_map->list_count = 0;
    struct pair_list *el = hash_map->insert_order_head;
    hash_map->insert_order_head = NULL;
    hash_map->insert_order_tail = NULL;
    struct pair_list *prev = NULL;
    while (el != NULL)
    {
        prev = el;
        hash_map_insert(hash_map, el->key, el->value, free_func);
        el = el->insert_order_next;
        free(prev);
    }
}

static int hash_map_resize(struct hash_map *hash_map, size_t new_capacity,
                           void (*free_func)(void *))
{
    hash_map->capacity = new_capacity;
    hash_map->data = realloc(hash_map->data,
                             sizeof(struct pair_list *) * hash_map->capacity);
    for (size_t i = 0; i < hash_map->capacity; i++)
    {
        hash_map->data[i] = 0;
    }
    if (hash_map->data == NULL && new_capacity != 0)
    {
        return 0;
    }
    hash_map_rehash(hash_map, free_func);
    return 1;
}
/* ! -- AUX -- */

struct hash_map *hash_map_init(void)
{
    struct hash_map *map = malloc(sizeof(struct hash_map));
    if (map == NULL)
    {
        return NULL;
    }
    map->capacity = HASH_DEFAULT_CAPACITY;
    map->element_count = 0;
    map->list_count = 0;
    map->data = calloc(map->capacity, sizeof(struct pair_list));
    map->insert_order_head = NULL;
    map->insert_order_tail = NULL;
    return map;
}

int hash_map_insert(struct hash_map *hash_map, char *key, void *value,
                    void (*free_func)(void *))
{
    if (hash_map == NULL || key == NULL || value == NULL)
    {
        return 1;
    }
    if (hash_map->list_count >= hash_map->capacity / 2)
    {
        size_t new_capacity =
            hash_map->capacity == 0 ? 1 : hash_map->capacity * 2;
        hash_map_resize(hash_map, new_capacity, free_func);
    }
    size_t hash_index = hash(key) % hash_map->capacity;
    struct pair_list *pair_list = hash_map->data[hash_index];
    struct pair_list *list = find_key(pair_list, key);
    // if only needs updating
    if (pair_list != NULL && list != NULL)
    {
        free_func(list->value);
        free(key);
        list->value = value;
    }
    else
    {
        struct pair_list *el = malloc(sizeof(struct pair_list));
        if (el == NULL)
        {
            return 1;
        }
        el->key = key;
        el->value = value;
        el->next = pair_list;
        el->insert_order_next = NULL;
        el->index = hash_index;
        hash_map->data[hash_index] = el;
        if (hash_map->element_count == 0)
        {
            hash_map->insert_order_head = el;
        }
        if (hash_map->insert_order_tail != NULL)
        {
            hash_map->insert_order_tail->insert_order_next = el;
        }
        hash_map->insert_order_tail = el;
        // if pair_list was null then we created a new list, else we prepended
        if (pair_list == NULL)
        {
            hash_map->list_count++;
        }
        hash_map->element_count++;
    }
    return 0;
}

void *hash_map_get(struct hash_map *hash_map, char *key)
{
    if (hash_map == NULL)
    {
        return NULL;
    }
    size_t hash_index = hash(key) % hash_map->capacity;
    struct pair_list *list = find_key(hash_map->data[hash_index], key);
    if (list == NULL)
    {
        return NULL;
    }
    return list->value;
}

void hash_map_free(struct hash_map *hash_map, void (*free_func)(void *))
{
    if (hash_map == NULL)
    {
        return;
    }
    struct pair_list *el = hash_map->insert_order_head;
    struct pair_list *prev = NULL;
    while (el != NULL)
    {
        prev = el;
        free(el->key);
        free_func(el->value);
        el = el->insert_order_next;
        free(prev);
    }
    free(hash_map->data);
    free(hash_map);
}
void hash_map_dump(struct hash_map *hash_map, void (*print_func)(void *))
{
    size_t c = 0;
    struct pair_list *el = hash_map->insert_order_head;
    while (el != NULL)
    {
        c++;
        printf("%s\n", el->key);
        print_func(el->value);
        el = el->insert_order_next;
    }
    printf("%zu vs %zu\n", c, hash_map->element_count);
}

int hash_map_remove(struct hash_map *hash_map, char *key,
                    void (*free_func)(void *))
{
    if (hash_map == NULL || key == NULL)
        return 0;

    size_t hash_index = hash(key) % hash_map->capacity;
    struct pair_list *curr = hash_map->data[hash_index];
    struct pair_list *prev = NULL;

    /* 1. Recherche dans le bucket */
    while (curr != NULL && strcmp(curr->key, key) != 0)
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) /* Pas trouvé */
        return 0;

    /* 2. Suppression du chaînage (Bucket) */
    if (prev == NULL)
    {
        hash_map->data[hash_index] = curr->next;
        if (curr->next == NULL)
            hash_map->list_count--;
    }
    else
        prev->next = curr->next;

    /* 3. Suppression de l'ordre d'insertion (Liste chaînée globale) */
    if (hash_map->insert_order_head == curr)
    {
        hash_map->insert_order_head = curr->insert_order_next;
        if (hash_map->insert_order_tail == curr)
            hash_map->insert_order_tail = NULL;
    }
    else
    {
        struct pair_list *tmp = hash_map->insert_order_head;
        while (tmp != NULL && tmp->insert_order_next != curr)
        {
            tmp = tmp->insert_order_next;
        }
        if (tmp != NULL)
        {
            tmp->insert_order_next = curr->insert_order_next;
            if (hash_map->insert_order_tail == curr)
                hash_map->insert_order_tail = tmp;
        }
    }

    /* 4. Libération mémoire */
    if (free_func && curr->value)
        free_func(curr->value);

    free(curr->key);
    free(curr);

    hash_map->element_count--;
    return 1;
}
