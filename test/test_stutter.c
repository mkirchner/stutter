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

static char* test_lexer()
{
    lexer_t* lexer = lexer_new(NULL);
    mu_assert(lexer != NULL, "Failed to create a lexer object");
    lexer_delete(lexer);
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

