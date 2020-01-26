/*
* stack.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __STACK_H__
#define __STACK_H__

#include <stdbool.h>
#include <sys/types.h>

#include "ast.h"

typedef enum {
    N_PROG,
    N_SEXP,
    N_LIST,
    N_ATOM,
    T_EOF,
    T_LPAREN,
    T_RPAREN,
    T_QUOTE,
    T_QUASIQUOTE,
    T_INT,
    T_FLOAT,
    T_STR,
    T_SYM
} ReaderStackTokenType;

extern const char *reader_stack_token_type_names[];

typedef struct ReaderStackToken {
    ReaderStackTokenType type;
    union {
        struct AstList *list;
        struct AstAtom *atom;
        struct AstSexpr *quoted;
        struct AstSexpr *sexp;
    } ast;
} ReaderStackToken;


typedef struct ReaderStack {
    size_t capacity;
    size_t size;
    ReaderStackToken *bos;  /* bottom of stack */
} ReaderStack;

ReaderStack *reader_stack_new(size_t capacity);
void reader_stack_delete(ReaderStack *stack);

void reader_stack_push(ReaderStack *stack, ReaderStackToken item);
int reader_stack_pop(ReaderStack *stack, ReaderStackToken *value);
int reader_stack_peek(ReaderStack *stack, ReaderStackToken *value);
bool reader_is_terminal(ReaderStackToken value);
bool reader_is_nonterminal(ReaderStackToken value);

#endif /* !__STACK_H__ */
