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
    LEXER_TOK_ERROR,
    LEXER_TOK_INT,
    LEXER_TOK_FLOAT,
    LEXER_TOK_STRING,
    LEXER_TOK_SYMBOL,
    LEXER_TOK_LPAREN,
    LEXER_TOK_RPAREN,
    LEXER_TOK_QUOTE,
    LEXER_TOK_EOF
} TokenType;

extern const char* token_type_names[];

typedef struct {
    TokenType type;
    void* value;
} LexerToken;

#define LEXER_TOKEN_VAL_AS_STR(t) ((char*) (t)->value)
#define LEXER_TOKEN_VAL_AS_INT(t) ((int*) (t)->value)
#define LEXER_TOKEN_VAL_AS_FLOAT(t) ((double*) (t)->value)

typedef enum {
    LEXER_STATE_ZERO,
    LEXER_STATE_NUMBER,
    LEXER_STATE_FLOAT,
    LEXER_STATE_SYMBOL,
    LEXER_STATE_STRING,
    LEXER_STATE_MINUS
} LexerState;

typedef struct {
    FILE* fp;
    LexerState state;
    size_t line_no;
    size_t char_no;
} Lexer;

/* object lifecycle */
Lexer* lexer_new(FILE* fp);
void lexer_delete(Lexer* l);

/* interface */
LexerToken* lexer_get_token(Lexer* l);
void lexer_delete_token(LexerToken* tok);

#endif /* !__LEXER_H__ */
