/*
 * stack.c
 * Copyright (C) 2019 marc
 *
 * Distributed under terms of the MIT license.
 */

#include "reader_stack.h"
#include <stdlib.h>
#include <assert.h>


const char* reader_stack_token_type_names[] = { 
    "N_PROG",
    "N_SEXP",
    "N_LIST",
    "N_ATOM",
    "T_EOF",
    "T_LPAREN",
    "T_RPAREN",
    "T_QUOTE",
    "T_INT",
    "T_FLOAT",
    "T_STR",
    "T_SYM"
};

reader_stack_t* reader_stack_new(size_t capacity)
{
    assert(capacity > 0);
    reader_stack_t* stack = (reader_stack_t*) malloc(sizeof(reader_stack_t));
    *stack = (reader_stack_t) {
        .capacity = capacity,
        .size = 0,
        .bos = (reader_stack_token_t*) malloc(sizeof(reader_stack_token_t) * capacity)
    };
    return stack;
}

void reader_stack_delete(reader_stack_t* stack)
{
    free(stack->bos);
    free(stack);
}

void reader_stack_push(reader_stack_t* stack, reader_stack_token_t item)
{
    if (stack->size >= stack->capacity) {
        stack->bos = realloc(stack->bos, 2*stack->capacity);
    }
    stack->bos[stack->size++] = item;
}

int reader_stack_pop(reader_stack_t* stack, reader_stack_token_t* value)
{
    if (stack->size > 0) {
        *value = stack->bos[--stack->size];
        return 0;
    }
    return 1;
}

int reader_stack_peek(reader_stack_t* stack, reader_stack_token_t* value)
{
    if (stack->size > 0) {
        *value = stack->bos[stack->size-1];
        return 0;
    }
    return 1;
}

static int _get_stack_symbol_type(reader_stack_token_t symbol)
{
    // returns 0 for terminals, 1 for non-terminals
    switch(symbol.type) {
        case(N_PROG):
        case(N_SEXP):
        case(N_LIST):
        case(N_ATOM):
            return 0;
        case(T_EOF):
        case(T_LPAREN):
        case(T_RPAREN):
        case(T_QUOTE):
        case(T_INT):
        case(T_FLOAT):
        case(T_STR):
        case(T_SYM):
            return 1;
    }
}

bool reader_is_terminal(reader_stack_token_t value)
{
    return (_get_stack_symbol_type(value) != 0);
}

bool reader_is_nonterminal(reader_stack_token_t value)
{
    return (_get_stack_symbol_type(value) == 0);
}
