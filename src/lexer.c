/*
 * lexer.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "lexer.h"

#include <stdlib.h>

lexer_t* lexer_new(FILE* fp)
{
    lexer_t* lexer = malloc(sizeof(lexer_t));
    lexer->fp = fp;
    return lexer;
}

void lexer_delete(lexer_t* l)
{
    free(l);
}

token_t lexer_get_token(lexer_t* l)
{
    token_t tok = { SYMBOL, NULL };
    return tok;
}

