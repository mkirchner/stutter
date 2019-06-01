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

#include "log.h"

static ht_item_t* ht_item_new(char* key, void* value, size_t siz)
{
    ht_item_t* item = (ht_item_t*) malloc(sizeof(ht_item_t));
    item->key = strdup(key);
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

ht_t* ht_new(size_t n)
{
    ht_t* ht = (ht_t*) malloc(sizeof(ht_t));
    ht->size = n;
    ht->count = 0;
    ht->items = calloc(n, sizeof(ht_item_t*));
    return ht;
}

void ht_delete(ht_t* ht)
{
    for (size_t i=0; i < ht->size; ++i) {
        if (ht->items[i] != NULL) {
            ht_item_delete(ht->items[i]);
        }
    }
    free(ht);
}

void ht_put(ht_t* ht, char* key, void* value, size_t siz)
{
    // hash
    unsigned long index = djb2(key) % ht->size;
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
}

void* ht_get(ht_t* ht, char* key)
{
    unsigned long index = djb2(key) % ht->size;
    LOG_DEBUG("index: %lu", index);
    ht_item_t* cur = ht->items[index];
    LOG_DEBUG("ptr: %p", cur);
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
    unsigned long index = djb2(key) % ht->size;
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
        } else {
            // move on
            prev = cur;
            cur = cur->next;
        }
        cur = cur->next;
    }
}
