/*
 * test_list.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */


#include "minunit.h"
#include "list.h"

static char* test_list()
{
    int numbers[4] = {1, 2, 3, 4};

    List* l = list_new();
    mu_assert(list_size(l) == 0, "Empty list should have length 0");
    for (size_t i = 0; i < 4; ++i) {
        list_append(l, numbers + i, sizeof(int));
    }
    mu_assert(list_size(l) == 4, "Number  of appended elemets should be 4");
    mu_assert(*(int*)list_head(l) == 1, "First element should be 1");
    List* tail = list_tail(l);
    mu_assert(list_size(tail) == 3, "Tail should have size 3");
    mu_assert(*(int*)list_head(tail) == 2, "First element of tail should be 2");
    list_delete(l);

    l = list_new();
    for (size_t i = 0; i < 4; ++i) {
        list_prepend(l, numbers + i, sizeof(int));
    }
    mu_assert(list_size(l) == 4, "Number  of prepended elemets should be 4");
    mu_assert(*(int*)list_head(l) == 4, "First element should be 4");
    list_delete(l);

    l = list_new();
    mu_assert(list_head(l) == NULL, "Empty list should have a NULL head");
    mu_assert(list_size(list_tail(l)) == 0, "Empty list should have an empty tail");
    list_append(l, numbers, sizeof(int));
    mu_assert(*(int*)list_head(l) == 1, "Head of one-element list should be 1");
    mu_assert(list_size(list_tail(l)) == 0, "One-element list should have an empty tail");
    list_delete(l);

    return 0;
}

