/*
 * array.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stddef.h>

/*
 * Contiguous, indexable chunk of memory.
 */
typedef struct array {
    char* p;
    size_t size;
    size_t capacity;
    size_t bytes;
} array_t;


array_t* array_new(const size_t item_size);
array_t* array_new_with_capacity(const size_t isize, const size_t capacity);
void array_delete(array_t* a);

#define array_size(a) (a->size)
#define array_capacity(a) (a->capacity)

void* array_at(array_t* a, size_t i);
#define array_typed_at(a,i,t) ((t*) array_at(a, i))
void array_push_back(array_t* a, const void* value, size_t n);
void array_push_front(array_t* a, const void* value, size_t n);
void* array_pop_back(array_t* a);
#define array_typed_pop_back(a,t) ((t*) array_pop_back(a))
void* array_pop_front(array_t* a);
#define array_typed_pop_front(a,t) ((t*) array_pop_front(a))
void array_shrink(array_t* a);

#endif /* !__ARRAY_H__ */
