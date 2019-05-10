/*
 * stack.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __STACK_H__
#define __STACK_H__

#include <sys/types.h>

typedef enum {
    N_PROG,
    N_SEXP,
    N_LIST,
    N_ATOM,
    T_EOF,
    T_LPAREN,
    T_RPAREN,
    T_QUOTE,
    T_INT,
    T_FLOAT,
    T_STR,
    T_SYM
} reader_stack_alphabet_t;

typedef struct reader_stack_t {
    size_t capacity;
    size_t size;
    reader_stack_alphabet_t* bos;  /* bottom of stack */
} reader_stack_t;

reader_stack_t* reader_stack_new(size_t capacity);
void reader_stack_delete(reader_stack_t* stack);

void reader_stack_push(reader_stack_t* stack, reader_stack_alphabet_t item);
int reader_stack_pop(reader_stack_t* stack, reader_stack_alphabet_t* value);
int reader_stack_peek(reader_stack_t* stack, reader_stack_alphabet_t* value);

#endif /* !__STACK_H__ */
