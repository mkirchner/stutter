/*
 * stack.c
 * Copyright (C) 2019 marc
 *
 * Distributed under terms of the MIT license.
 */

#include "reader_stack.h"
#include <stdlib.h>
#include <assert.h>


const char *reader_stack_token_type_names[] = {
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

ReaderStack *reader_stack_new(size_t capacity)
{
    assert(capacity > 0);
    ReaderStack *stack = (ReaderStack *) malloc(sizeof(ReaderStack));
    *stack = (ReaderStack) {
        .capacity = capacity,
        .size = 0,
        .bos = (ReaderStackToken *) malloc(sizeof(ReaderStackToken) * capacity)
    };
    return stack;
}

void reader_stack_delete(ReaderStack *stack)
{
    free(stack->bos);
    free(stack);
}

void reader_stack_push(ReaderStack *stack, ReaderStackToken item)
{
    if (stack->size >= stack->capacity) {
        stack->bos = realloc(stack->bos, 2 * stack->capacity);
    }
    stack->bos[stack->size++] = item;
}

int reader_stack_pop(ReaderStack *stack, ReaderStackToken *value)
{
    if (stack->size > 0) {
        *value = stack->bos[--stack->size];
        return 0;
    }
    return 1;
}

int reader_stack_peek(ReaderStack *stack, ReaderStackToken *value)
{
    if (stack->size > 0) {
        *value = stack->bos[stack->size - 1];
        return 0;
    }
    return 1;
}

static int _get_stack_symbol_type(ReaderStackToken symbol)
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

bool reader_is_terminal(ReaderStackToken value)
{
    return (_get_stack_symbol_type(value) != 0);
}

bool reader_is_nonterminal(ReaderStackToken value)
{
    return (_get_stack_symbol_type(value) == 0);
}
