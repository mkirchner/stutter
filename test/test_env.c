/*
 * test_env.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include "minunit.h"
#include "value.h"

#include "../src/env.c"

static char* test_env()
{
    /*
     * creation
     */
    Environment* env0 = env_new(NULL);
    mu_assert(env_get(env0, "some_key") == NULL, "New env should be empty");
    /*
     * get/set
     */
    Value* val0 = value_new_int(42);
    env_set(env0, "key1", val0);
    Value* ret0 = env_get(env0, "key1");
    mu_assert(ret0->type = VALUE_INT, "value type must not change");
    mu_assert(42 == ret0->value.int_, "Value must not change");
    /*
     * nesting
     */
    Environment* env1 = env_new(env0);
    mu_assert(env1->parent == env0, "Failed to set parent");
    Environment* env2 = env_new(env1);
    mu_assert(env2->parent == env1, "Failed to set parent");
    ret0 = env_get(env2, "key1");
    mu_assert(ret0 != NULL, "Should find key in nested env");
    mu_assert(ret0->type = VALUE_INT, "Value type must not change");
    mu_assert(42 == ret0->value.int_, "Value must not change");

    return 0;
}

int tests_run = 0;

static char* test_suite()
{
    mu_run_test(test_env);
    return 0;
}

int main()
{
    printf("---=[ Environment tests\n");
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
