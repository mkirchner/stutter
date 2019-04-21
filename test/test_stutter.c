/*
 * test_stutter.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "minunit.h"

int tests_run = 0;

static char* test_some_function()
{
    printf("%s... ", __func__);
    mu_assert(1 == 1, "The universe is broken");
    printf("OK\n");
    return 0;
}

static char * test_suite()
{
    mu_run_test(test_some_function);
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

