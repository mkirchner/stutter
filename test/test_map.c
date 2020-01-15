/*
 * test_hashtable.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include "minunit.h"

#include <string.h>
#include "gc.h"
#include "log.h"

#include "../src/map.c"


static char* test_map()
{
    Map* ht = map_new(3);
    LOG_DEBUG("Capacity: %lu", ht->capacity);
    mu_assert(ht->capacity == 3, "Capacity sizing failure");
    map_put(ht, "key", "value", strlen("value") + 1);
    // set/get item
    char* value = (char*) map_get(ht, "key");
    mu_assert(value != NULL, "Query must find inserted key");
    mu_assert(strcmp(value, "value") == 0, "Query must return inserted value");

    // update item
    map_put(ht, "key", "other", strlen("other") + 1);
    value = (char*) map_get(ht, "key");
    mu_assert(value != NULL, "Query must find key");
    mu_assert(strcmp(value, "other") == 0, "Query must return updated value");

    // delete item
    map_remove(ht, "key");
    value = (char*) map_get(ht, "key");
    mu_assert(value == NULL, "Query must NOT find deleted key");

    map_delete(ht);
    return 0;
}

int tests_run = 0;

static char* test_suite()
{
    void* bos = NULL;
    gc_start(&gc, &bos);
    mu_run_test(test_map);
    gc_stop(&gc);
    return 0;
}

int main()
{
    printf("---=[ map tests\n");
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
