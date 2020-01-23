/*
 * test_list.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */


#include "minunit.h"
#include "log.h"

#include "../src/list.c"

static char* test_list()
{
    int numbers[4] = {1, 2, 3, 4};

    const List* l = list_new();
    mu_assert(list_size(l) == 0, "Empty list should have length 0");

    /* empty copy */
    List* l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l->begin == NULL, "begin ptr must be NULL");
    mu_assert(l2->begin == NULL, "begin ptr must be NULL");
    mu_assert(l->end == NULL, "end ptr must be NULL");
    mu_assert(l2->end == NULL, "end ptr must be NULL");

    /* list of size 1 */
    l = list_new();
    for (size_t i = 0; i < 1; ++i) {
        l = list_conj(l, numbers + i);
        mu_assert(list_size(l) == i+1, "List should grow by one in every step");
    }
    l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l2->begin != l->begin, "begin ptrs must be different");
    mu_assert(l2->end != l->end, "end ptrs must be different");
    mu_assert(l->begin->prev == NULL, "Wrong prev ptr for first element in src");
    mu_assert(l2->begin->prev == NULL, "Wrong prev ptr for first element in dst");
    mu_assert(l->end->next== NULL, "Wrong next ptr for last element in src");
    mu_assert(l2->end->next== NULL, "Wrong next ptr for last element in dst");

    l = list_new();
    for (size_t i = 0; i < 4; ++i) {
        l = list_conj(l, numbers + i);
        mu_assert(list_size(l) == i+1, "List should grow by one in every step");
    }
    /* list_mutable_copy: List object and list items need to be deep copies
     *            pointing to the same content
     */
    l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l2->begin != l->begin, "begin ptrs must be different");
    mu_assert(l2->end != l->end, "end ptrs must be different");
    mu_assert(l->begin->prev == NULL, "Wrong prev ptr for first element in src");
    mu_assert(l2->begin->prev == NULL, "Wrong prev ptr for first element in dst");
    mu_assert(l->end->next== NULL, "Wrong next ptr for last element in src");
    mu_assert(l2->end->next== NULL, "Wrong next ptr for last element in dst");

    ListItem *cur, *cur2;
    cur = l->begin;
    cur2 = l2->begin;
    size_t i = 0;
    while (cur != NULL && cur2 != NULL) {
        mu_assert(cur->p == numbers + i, "Wrong data reference in src");
        mu_assert(cur2->p == numbers + i, "Wrong data reference in dst");
        if (cur->next) {
            mu_assert(cur->next->prev == cur, "Wrong prev ptr in src");
        }
        if (cur2->next) {
            mu_assert(cur2->next->prev == cur2, "Wrong prev ptr in dst");
        }
        cur = cur->next;
        cur2 = cur2->next;
        ++i;
    }
    mu_assert(cur == NULL && cur2 == NULL, "copy has different length");



    mu_assert(list_size(l) == 4, "Number  of appended elemets should be 4");
    mu_assert(*(int*)list_head(l) == 1, "First element should be 1");
    const List* tail = list_tail(l);
    mu_assert(list_size(tail) == 3, "Tail should have size 3");
    mu_assert(*(int*)list_head(tail) == 2, "First element of tail should be 2");

    l = list_new();
    for (size_t i = 0; i < 4; ++i) {
        l = list_cons(l, numbers + i);
        mu_assert(list_size(l) == i+1, "List should grow by one in every step");
    }
    mu_assert(list_size(l) == 4, "Number  of prepended elemets should be 4");
    mu_assert(*(int*)list_head(l) == 4, "First element should be 4");

    l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l2->begin != l->begin, "begin ptrs must be different");
    mu_assert(l2->end != l->end, "end ptrs must be different");
    mu_assert(l->begin->prev == NULL, "Wrong prev ptr for first element in src");
    mu_assert(l2->begin->prev == NULL, "Wrong prev ptr for first element in dst");
    mu_assert(l->end->next== NULL, "Wrong next ptr for last element in src");
    mu_assert(l2->end->next== NULL, "Wrong next ptr for last element in dst");

    cur = l->begin;
    cur2 = l2->begin;
    i = 0;
    while (cur != NULL && cur2 != NULL) {
        mu_assert(cur->p == numbers + 3 - i, "Wrong data reference in src");
        mu_assert(cur2->p == numbers + 3 - i, "Wrong data reference in dst");
        if (cur->next) {
            mu_assert(cur->next->prev == cur, "Wrong prev ptr in src");
        }
        if (cur2->next) {
            mu_assert(cur2->next->prev == cur2, "Wrong prev ptr in dst");
        }
        cur = cur->next;
        cur2 = cur2->next;
        ++i;
    }
    mu_assert(cur == NULL && cur2 == NULL, "copy has different length");

    l = list_new();
    mu_assert(list_head(l) == NULL, "Empty list should have a NULL head");
    mu_assert(list_size(list_tail(l)) == 0, "Empty list should have an empty tail");
    l = list_conj(l, numbers);
    mu_assert(*(int*)list_head(l) == 1, "Head of one-element list should be 1");
    mu_assert(list_size(list_tail(l)) == 0, "One-element list should have an empty tail");

    return 0;
}

int tests_run = 0;

static char* test_suite()
{
    int bos;
    gc_start(&gc, &bos);
    mu_run_test(test_list);
    gc_stop(&gc);
    return 0;
}

int main()
{
    printf("---=[ List tests\n");
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}

