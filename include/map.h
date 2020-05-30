/*
 * A simple hashtable implementation for string keys, using separate chaining.
 */

#ifndef __HT_H__
#define __HT_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct MapItem {
    char *key;
    void *value;
    size_t size;
    struct MapItem *next;
} MapItem;

typedef struct Map {
    size_t capacity;
    size_t size;
    MapItem **items;
} Map;

Map *map_new(size_t n);
void map_delete(Map *);

void *map_get(Map *ht, char *key);
void map_put(Map *ht, char *key, void *value, size_t siz);
void map_remove(Map *ht, char *key);
void map_resize(Map *ht, size_t capacity);

// helpers

bool is_prime(size_t n);

#endif /* !__HT_H__ */
