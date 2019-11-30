/*
 * test_stutter.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "test_array.c"
#include "test_ast.c"
#include "test_djb2.c"
#include "test_env.c"
#include "test_gc.c"
#include "test_ir.c"
#include "test_lexer.c"
#include "test_list.c"
#include "test_map.c"
#include "test_primes.c"

int tests_run = 0;

static char* test_suite()
{
    mu_run_test(test_ast);
    mu_run_test(test_lexer);
    mu_run_test(test_djb2);
    mu_run_test(test_map);
    mu_run_test(test_primes);
    mu_run_test(test_env);
    mu_run_test(test_array);
    mu_run_test(test_list);
    mu_run_test(test_ir);
    mu_run_test(test_gc);
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

