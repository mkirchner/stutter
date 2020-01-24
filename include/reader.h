/*
 * reader.h
 * Copyright (C) 2019 marc
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __READER_H__
#define __READER_H__

#include <stdio.h>

#include "lexer.h"
#include "ast.h"

#define READER_SUCCESS 0
#define READER_FAILURE 1

typedef struct {
    Lexer *lexer;
} Reader;

/*
 * Macros to simplify AST interaction
 */

/*
 * The reader interface
 */
Reader *reader_new(FILE *stream);
void reader_delete(Reader *r);
AstSexpr *reader_read(Reader *r);

#endif /* !__READER_H__ */
