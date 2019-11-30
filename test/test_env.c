/*
 * test_env.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include "minunit.h"
#include "env.h"
#include "value.h"

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

    env_delete(env2);
    env_delete(env1);
    env_delete(env0);

    return 0;
}
