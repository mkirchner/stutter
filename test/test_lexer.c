/*
 * test_lexer.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */


#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "lexer.h"


static char* type_names[] = {
    "ERROR", "INT", "FLOAT", "STRING", "SYMBOL",
    "LPAREN", "RPAREN", "QUOTE", "EOF"
};

static char* test_lexer()
{
    /* set up lexer to read from input file */
    FILE* input_fd = fopen("test/data/lexer_test.str", "r");
    mu_assert(input_fd!= NULL, "Failed to open lexer test file");
    Lexer* lexer = lexer_new(input_fd);
    mu_assert(lexer != NULL, "Failed to create a lexer object");

    /* at the same time, we'll read the expected symbols
       from  the reference file */
    FILE* ref_fd = fopen("test/data/lexer_reference.txt", "r");
    mu_assert(ref_fd!= NULL, "Failed to open lexer test reference file");
    char *ref_line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    LexerToken* tok = lexer_get_token(lexer);
    linelen = getdelim(&ref_line, &linecap, ' ', ref_fd);
    while (tok != NULL && tok->type != LEXER_TOK_EOF && linelen > 0) {
        ref_line[linelen-1] = '\0';
        // printf("'%s' =?= '%s'\n", type_names[tok->type], ref_line);
        mu_assert(strcmp(type_names[tok->type], ref_line) == 0,
                  "Unexpected symbol");
        lexer_delete_token(tok);
        tok = lexer_get_token(lexer);
        linelen = getdelim(&ref_line, &linecap, ' ', ref_fd);
    }
    mu_assert(tok != NULL && tok->type == LEXER_TOK_EOF
              && linelen == -1, "Incorrect number of symbols");
    lexer_delete(lexer);
    fclose(ref_fd);
    fclose(input_fd);
    return 0;
}

