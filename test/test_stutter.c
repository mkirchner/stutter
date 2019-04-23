/*
 * test_stutter.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "lexer.h"

int tests_run = 0;

static char* type_names[] = {
    "ERROR", "INT", "FLOAT", "STRING", "SYMBOL", "LPAREN", "RPAREN"
};

static char* test_lexer()
{
    FILE* fd = fopen("test/lexer_test_file_01.str", "r");
    mu_assert(fd != NULL, "Failed to open test file");
    lexer_t* lexer = lexer_new(fd);
    mu_assert(lexer != NULL, "Failed to create a lexer object");
    lexer_token_t* tok = lexer_get_token(lexer);
    size_t row = lexer->line_no;
    while (tok != NULL) {
        if (lexer->line_no > row) {
            row = lexer->line_no;
            printf("\n");
        }
        printf("%s ", type_names[tok->type]);
        if (tok->type == ERROR) {
            printf("[line %lu, pos %lu] ", lexer->line_no, lexer->char_no);
        }
        lexer_delete_token(tok);
        tok = lexer_get_token(lexer);
    }
    printf("\n");
    lexer_delete(lexer);
    fclose(fd);
    return 0;
}

static char * test_suite()
{
    mu_run_test(test_lexer);
    return 0;
}

int main()
{
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}

