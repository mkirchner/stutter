/*
 * array.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "array.h"

#include <stdlib.h>
#include <string.h>


Array* array_new(const size_t item_size)
{
    // default to 2 elements for empty arrays
    return array_new_with_capacity(item_size, 2);
}

Array* array_new_with_capacity(const size_t item_size, const size_t capacity)
{
    Array* array = malloc(sizeof(Array));
    array->p = calloc(capacity, item_size);
    array->bytes = item_size;
    array->capacity = capacity;
    array->size = 0;
    return array;
}

void array_delete(Array* a)
{
    free(a->p);
    free(a);
}

static uint64_t next_power_of_2(uint64_t v)
{
    // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

static void array_resize(Array* a, size_t requested_capacity)
{
    size_t new_capacity = next_power_of_2(requested_capacity);
    if (a->size > new_capacity) a->size = new_capacity;
    a->p = realloc(a->p, new_capacity*a->bytes);
    a->capacity = new_capacity;
}

void* array_at(Array* a, size_t i)
{
    return (void*) (a->p + i * a->bytes);
}

void array_push_back(Array* a, const void* value, size_t n)
{
    size_t total = n + a->size;
    array_resize(a, total);
    char* pos = a->p + (a->size * a->bytes);
    memcpy(pos, value, n * a->bytes);
    a->size += n;
}

void array_push_front(Array* a, const void* value, size_t n)
{
    size_t total = n + a->size;
    // allocate sufficient memory
    array_resize(a, total);
    // shift contents away from the front
    size_t width = n * a->bytes;
    char* pos = a->p + width;
    memmove(pos, a->p, a->size*a->bytes);
    // insert at the front
    memcpy(a->p, value, width);
    // update the size info
    a->size += n;
}

void* array_pop_back(Array* a)
{
    if (a->size == 0) return NULL;
    a->size--;
    return a->p + a->size * a->bytes;
}

void* array_pop_front(Array* a)
{
    if (a->size == 0)
        return NULL;
    // swap
    char tmp[a->bytes];
    memcpy(&tmp, a->p, a->bytes);
    memmove(a->p, a->p + a->bytes, (a->size - 1) * a->bytes);
    a->size--;
    memcpy(a->p + a->size * a->bytes, &tmp, a->bytes);
    return (void*) (a->p + a->size * a->bytes);
}

void array_shrink(Array* a)
{
   array_resize(a, a->size);
}
