/*
 * stack.c
 * Copyright (C) 2019 marc
 *
 * Distributed under terms of the MIT license.
 */

#include "reader_stack.h"
#include <stdlib.h>
#include <assert.h>

reader_stack_t* reader_stack_new(size_t capacity)
{
    assert(capacity > 0);
    reader_stack_t* stack = (reader_stack_t*) malloc(sizeof(reader_stack_t));
    *stack = (reader_stack_t) {
        .capacity = capacity,
        .size = 0,
        .bos = (reader_stack_alphabet_t*) malloc(sizeof(reader_stack_alphabet_t) * capacity)
    };
    return stack;
}

void reader_stack_delete(reader_stack_t* stack)
{
    free(stack->bos);
    free(stack);
}

void reader_stack_push(reader_stack_t* stack, reader_stack_alphabet_t item)
{
    if (stack->size >= stack->capacity) {
        stack->bos = realloc(stack->bos, 2*stack->capacity);
    }
    stack->bos[stack->size++] = item;
}

int reader_stack_pop(reader_stack_t* stack, reader_stack_alphabet_t* value)
{
    if (stack->size > 0) {
        *value = stack->bos[--stack->size];
        return 0;
    }
    return 1;
}

int reader_stack_peek(reader_stack_t* stack, reader_stack_alphabet_t* value)
{
    if (stack->size > 0) {
        *value = stack->bos[stack->size-1];
        return 0;
    }
    return 1;
}
