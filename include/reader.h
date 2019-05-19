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
    lexer_t* lexer;
} reader_t;

/*
 * Macros to simplify AST interaction
 */

/*
 * The reader interface
 */
reader_t* reader_new(FILE* stream);
void reader_delete(reader_t* r);
ast_sexpr_t* reader_read(reader_t* r);

#endif /* !__READER_H__ */
