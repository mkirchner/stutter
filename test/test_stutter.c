/*
 * test_stutter.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "gc.h"
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
    /*
    int bos;
    gc_start(&gc, &bos);
    printf("---=[ AST tests\n");
    mu_run_test(test_ast);
    printf("---=[ Lexer tests\n");
    mu_run_test(test_lexer);
    printf("---=[ DJB2 tests\n");
    mu_run_test(test_djb2);
    printf("---=[ Map tests\n");
    mu_run_test(test_map);
    printf("---=[ Primes tests\n");
    mu_run_test(test_primes);
    printf("---=[ Environment tests\n");
    mu_run_test(test_env);
    printf("---=[ Array tests\n");
    mu_run_test(test_array);
    printf("---=[ List tests\n");
    mu_run_test(test_list);
    printf("---=[ IR tests\n");
    mu_run_test(test_ir);
    gc_stop(&gc);
    */
    printf("---=[ GC tests\n");
    mu_run_test(test_gc_allocation_new_delete);
    mu_run_test(test_gc_allocation_map_new_delete);
    mu_run_test(test_gc_allocation_map_basic_get);
    mu_run_test(test_gc_allocation_map_put_get_remove);
    mu_run_test(test_gc);
    mu_run_test(test_gc_basic_alloc_free);
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

