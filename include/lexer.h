/*
 * lexer.h
 * Copyright (C) 2019 marc
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdio.h>

typedef enum {INT, FLOAT, STRING, SYMBOL, LPAREN, RPAREN} token_type_t;

typedef struct {
    token_type_t type;
    void* value;
} token_t;

#define LEXER_TOKEN_VAL_AS_STR(t) ((char*) (t)->value)
#define LEXER_TOKEN_VAL_AS_INT(t) ((int*) (t)->value)
#define LEXER_TOKEN_VAL_AS_FLOAT(t) ((float*) (t)->value)

typedef enum {LEXER_INIT, LEXER_X} lexer_state_t;

typedef struct {
    FILE* fp;
    lexer_state_t state;
    unsigned int line_no;
} lexer_t;

/* object lifecycle */
lexer_t* lexer_new(FILE* fp);
void lexer_delete(lexer_t* l);

/* interface */
token_t lexer_get_token(lexer_t* l);

#endif /* !__LEXER_H__ */
