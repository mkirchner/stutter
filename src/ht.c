/*
 * ht.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "ht.h"
#include "djb2.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "log.h"


bool is_prime(size_t n)
{
    // https://stackoverflow.com/questions/1538644/c-determine-if-a-number-is-prime
    if (n <= 1)
        return false;
    else if (n <= 3 && n > 1)
        return true;            // as 2 and 3 are prime
    else if (n % 2==0 || n % 3==0)
        return false;     // check if n is divisible by 2 or 3
    else {
        for (size_t i=5; i*i<=n; i+=6) {
            if (n % i == 0 || n%(i + 2) == 0)
                return false;
        }
        return true;
    }
}

size_t next_prime(size_t n)
{
    while (!is_prime(n)) ++n;
    return n;
}

static double load_factor(ht_t* ht)
{
    LOG_DEBUG("Load factor: %.2f", (double) ht->size / (double) ht->capacity);
    return (double) ht->size / (double) ht->capacity;
}

static ht_item_t* ht_item_new(char* key, void* value, size_t siz)
{
    ht_item_t* item = (ht_item_t*) malloc(sizeof(ht_item_t));
    item->key = strdup(key);
    item->size = siz;
    item->value = malloc(siz);
    memcpy(item->value, value, siz);
    item->next = NULL;
    return item;
}

static void ht_item_delete(ht_item_t* item)
{
    if (item) {
        free(item->key);
        free(item->value);
        free(item);
    }
}

ht_t* ht_new(size_t capacity)
{
    ht_t* ht = (ht_t*) malloc(sizeof(ht_t));
    ht->capacity = next_prime(capacity);
    ht->size = 0;
    ht->items = calloc(ht->capacity, sizeof(ht_item_t*));
    return ht;
}

void ht_delete(ht_t* ht)
{
    for (size_t i=0; i < ht->capacity; ++i) {
        if (ht->items[i] != NULL) {
            ht_item_delete(ht->items[i]);
        }
    }
    free(ht);
}

void ht_put(ht_t* ht, char* key, void* value, size_t siz)
{
    // hash
    unsigned long index = djb2(key) % ht->capacity;
    LOG_DEBUG("index: %lu", index);
    // create item
    ht_item_t* item = ht_item_new(key, value, siz);
    ht_item_t* cur = ht->items[index];
    // update if exists
    ht_item_t* prev = NULL;
    while(cur != NULL) {
        if (strcmp(cur->key, key) == 0) {
            // found it
            item->next = cur->next;
            if (!prev) {
                // position 0
                ht->items[index] = item;
            } else {
                // in the list
                prev->next = item;
            }
            ht_item_delete(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    // insert (at front of list)
    cur = ht->items[index];
    item->next = cur;
    ht->items[index] = item;
    ht->size++;
    if (load_factor(ht) > 0.7)
        ht_resize(ht, next_prime(ht->capacity*2));
}

void* ht_get(ht_t* ht, char* key)
{
    unsigned long index = djb2(key) % ht->capacity;
    LOG_DEBUG("index: %lu", index);
    ht_item_t* cur = ht->items[index];
    LOG_DEBUG("ptr: %p", (void *)cur);
    while(cur != NULL) {
        if (strcmp(cur->key, key) == 0) {
            return cur->value;
        }
        cur = cur->next;
    }
    return NULL;
}

void ht_remove(ht_t* ht, char* key)
{
    // ignores unknown keys
    unsigned long index = djb2(key) % ht->capacity;
    ht_item_t* cur = ht->items[index];
    ht_item_t* prev = NULL;
    while(cur != NULL) {
        if (strcmp(cur->key, key) == 0) {
            // found it
            if (!prev) {
                // first item in list
                ht->items[index] = cur->next;
            } else {
                // not the first item in the list
                prev->next = cur->next;
            }
            ht_item_delete(cur);
            ht->size--;
        } else {
            // move on
            prev = cur;
            cur = cur->next;
        }
        cur = cur->next;
    }
    if (load_factor(ht) < 0.1)
        ht_resize(ht, next_prime(ht->capacity/2));
}

void ht_resize(ht_t* ht, size_t new_capacity)
{
    // Replaces the existing items array in the hash table
    // with a resized one and pushes items into the new, correct buckets
    LOG_DEBUG("Resizing to %lu", new_capacity);
    ht_item_t** resized_items = calloc(new_capacity, sizeof(ht_item_t*));

    for (size_t i=0; i<ht->capacity; ++i) {
        ht_item_t* item = ht->items[i];
        while(item) {
            ht_item_t* next_item = item->next;
            unsigned long new_index = djb2(item->key) % new_capacity;
            item->next = resized_items[new_index];
            resized_items[new_index] = item;
            item = next_item;
        }
    }
    free(ht->items);
    ht->capacity = new_capacity;
    ht->items = resized_items;
}
