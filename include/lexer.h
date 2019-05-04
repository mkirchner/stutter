/*
 * lexer.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdio.h>

typedef enum {
    ERROR, INT, FLOAT, STRING, SYMBOL,
    LPAREN, RPAREN, QUOTE
} token_type_t;

typedef struct {
    token_type_t type;
    void* value;
} lexer_token_t;

#define LEXER_TOKEN_VAL_AS_STR(t) ((char*) (t)->value)
#define LEXER_TOKEN_VAL_AS_INT(t) ((int*) (t)->value)
#define LEXER_TOKEN_VAL_AS_FLOAT(t) ((double*) (t)->value)

typedef enum {
    LEXER_STATE_ZERO,
    LEXER_STATE_NUMBER,
    LEXER_STATE_FLOAT,
    LEXER_STATE_SYMBOL,
    LEXER_STATE_STRING
} lexer_state_t;

typedef struct {
    FILE* fp;
    lexer_state_t state;
    size_t line_no;
    size_t char_no;
} lexer_t;

/* object lifecycle */
lexer_t* lexer_new(FILE* fp);
void lexer_delete(lexer_t* l);

/* interface */
lexer_token_t* lexer_get_token(lexer_t* l);
void lexer_delete_token(lexer_token_t* tok);

#endif /* !__LEXER_H__ */
