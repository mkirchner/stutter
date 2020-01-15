/*
 * test_lexer.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */


#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "../src/lexer.c"


static char* type_names[] = {
    "ERROR", "INT", "FLOAT", "STRING", "SYMBOL",
    "LPAREN", "RPAREN", "QUOTE", "EOF"
};

static char input[] =  "12 ( 34.5 ) \"Hello World!\" abc 23.b (12(23))) \n"
                       "\"this is a string\" vEryC0mplicated->NamE 'symbol ";

static char expected[] = "INT LPAREN FLOAT RPAREN STRING SYMBOL ERROR LPAREN "
                         "INT LPAREN INT RPAREN RPAREN RPAREN STRING SYMBOL "
                         "QUOTE SYMBOL ";

static char* test_lexer()
{
    /* set up lexer to read from input file */
    size_t n = strlen(input);
    FILE* in_fd = fmemopen(input, n, "r");
    mu_assert(in_fd != NULL, "Failed to open lexer test file");
    Lexer* lexer = lexer_new(in_fd);
    mu_assert(lexer != NULL, "Failed to create a lexer object");

    /* at the same time, we'll read the expected symbols
       from  the reference file */
    n = strlen(expected);
    FILE* ref_fd = fmemopen(expected, n, "r");
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
    fclose(in_fd);
    return 0;
}

int tests_run = 0;

static char* test_suite()
{
    mu_run_test(test_lexer);
    return 0;
}

int main()
{
    printf("---=[ Lexer tests\n");
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
