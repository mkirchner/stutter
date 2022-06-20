#include "vector.h"

#include <stdlib.h>
#include <string.h>


Vector *vector_new(const size_t item_size)
{
    // default to 2 elements for empty vectors
    return vector_new_with_capacity(item_size, 2);
}

Vector *vector_new_with_capacity(const size_t item_size, const size_t capacity)
{
    Vector *vector = malloc(sizeof(Vector));
    vector->p = calloc(capacity, item_size);
    vector->bytes = item_size;
    vector->capacity = capacity;
    vector->size = 0;
    return vector;
}

void vector_delete(Vector *a)
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

static void vector_resize(Vector *a, size_t requested_capacity)
{
    size_t new_capacity = next_power_of_2(requested_capacity);
    if (a->size > new_capacity) a->size = new_capacity;
    a->p = realloc(a->p, new_capacity * a->bytes);
    a->capacity = new_capacity;
}

void *vector_at(Vector *a, size_t i)
{
    return (void *) (a->p + i * a->bytes);
}

void vector_push_back(Vector *a, const void *value, size_t n)
{
    size_t total = n + a->size;
    vector_resize(a, total);
    char *pos = a->p + (a->size * a->bytes);
    memcpy(pos, value, n * a->bytes);
    a->size += n;
}

void vector_push_front(Vector *a, const void *value, size_t n)
{
    size_t total = n + a->size;
    // allocate sufficient memory
    vector_resize(a, total);
    // shift contents away from the front
    size_t width = n * a->bytes;
    char *pos = a->p + width;
    memmove(pos, a->p, a->size * a->bytes);
    // insert at the front
    memcpy(a->p, value, width);
    // update the size info
    a->size += n;
}

void *vector_pop_back(Vector *a)
{
    if (a->size == 0) return NULL;
    a->size--;
    return a->p + a->size * a->bytes;
}

void *vector_pop_front(Vector *a)
{
    if (a->size == 0)
        return NULL;
    // swap
    char tmp[a->bytes];
    memcpy(&tmp, a->p, a->bytes);
    memmove(a->p, a->p + a->bytes, (a->size - 1) * a->bytes);
    a->size--;
    memcpy(a->p + a->size * a->bytes, &tmp, a->bytes);
    return (void *) (a->p + a->size * a->bytes);
}

void vector_shrink(Vector *a)
{
    vector_resize(a, a->size);
}
