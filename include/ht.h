/*
 * ht.h
 *
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 *
 * A simple hashtable implementation for string keys, using separate chaining.
 */

#ifndef __HT_H__
#define __HT_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct ht_item_t {
    char* key;
    void* value;
    size_t size;
    struct ht_item_t* next;
} ht_item_t;

typedef struct ht_t {
    size_t capacity;
    size_t size;
    ht_item_t** items;
} ht_t;

ht_t* ht_new(size_t n);
void ht_delete(ht_t*);

void* ht_get(ht_t* ht, char* key);
void ht_put(ht_t* ht, char* key, void* value, size_t siz);
void ht_remove(ht_t* ht, char* key);
void ht_resize(ht_t* ht, size_t capacity);

// helpers

bool is_prime(size_t n);

#endif /* !__HT_H__ */
