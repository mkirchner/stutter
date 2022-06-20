#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stddef.h>

/*
 * Contiguous, indexable chunk of memory.
 */
typedef struct vector {
    char *p;
    size_t size;
    size_t capacity;
    size_t bytes;
} Vector;


Vector *vector_new(const size_t item_size);
Vector *vector_new_with_capacity(const size_t isize, const size_t capacity);
void vector_delete(Vector *a);

#define vector_size(a) (a->size)
#define vector_capacity(a) (a->capacity)

void *vector_at(Vector *a, size_t i);
#define vector_typed_at(a,i,t) ((t*) vector_at(a, i))
void vector_push_back(Vector *a, const void *value, size_t n);
void vector_push_front(Vector *a, const void *value, size_t n);
void *vector_pop_back(Vector *a);
#define vector_typed_pop_back(a,t) ((t*) vector_pop_back(a))
void *vector_pop_front(Vector *a);
#define vector_typed_pop_front(a,t) ((t*) vector_pop_front(a))
void vector_shrink(Vector *a);

#endif /* !__VECTOR_H__ */
