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
Vector *vector_dup(const Vector *vec);
void vector_delete(Vector *vec);

#define vector_size(v) (v->size)
#define vector_capacity(v) (v->capacity)

void *vector_at(Vector *v, size_t i);
#define vector_typed_at(v,i,t) ((t*) vector_at(v, i))
void vector_push_back(Vector *v, const void *value, size_t n);
void vector_push_front(Vector *v, const void *value, size_t n);
void *vector_pop_back(Vector *v);
#define vector_typed_pop_back(v,t) ((t*) vector_pop_back(v))
void *vector_pop_front(Vector *v);
#define vector_typed_pop_front(v,t) ((t*) vector_pop_front(v))
void vector_shrink(Vector *v);

#endif /* !__VECTOR_H__ */
