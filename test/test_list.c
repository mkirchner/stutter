#include "minunit.h"
#include "log.h"
#include "value.h"

#include "../src/list.c"

static char *test_list()
{
    Value *numbers[4] = {
        value_new_int(1),
        value_new_int(2),
        value_new_int(3),
        value_new_int(4)
    };

    const List *l = list_new();
    mu_assert(list_size(l) == 0, "Empty list should have length 0");

    /* empty copy */
    List *l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l->head == NULL, "head ptr must be NULL");
    mu_assert(l2->head == NULL, "head ptr must be NULL");

    /* list of size 1 */
    l = list_new();
    for (size_t i = 0; i < 1; ++i) {
        l = list_append(l, numbers[i]);
        mu_assert(list_size(l) == i + 1, "List should grow by one in every step");
    }
    l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l2->head != l->head, "head ptrs must be different");

    l = list_new();
    for (size_t i = 0; i < 4; ++i) {
        l = list_append(l, numbers[i]);
        mu_assert(list_size(l) == i + 1, "List should grow by one in every step");
    }
    /* list_mutable_copy: List object and list items need to be deep copies
     *            pointing to the same content
     */
    l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l2->head != l->head, "head ptrs must be different");

    ListItem *cur, *cur2;
    cur = l->head;
    cur2 = l2->head;
    size_t i = 0;

    while (cur != NULL && cur2 != NULL) {
        mu_assert(cur->val == numbers[i], "Wrong data reference in src");
        mu_assert(cur2->val == numbers[i], "Wrong data reference in dst");
        cur = cur->next;
        cur2 = cur2->next;
        ++i;
    }
    mu_assert(cur == NULL && cur2 == NULL, "copy has different length");

    mu_assert(list_size(l) == 4, "Number  of appended elemets should be 4");
    mu_assert(list_head(l)->value.int_ == 1, "First element should be 1");
    const List *tail = list_tail(l);
    mu_assert(list_size(tail) == 3, "Tail should have size 3");
    mu_assert(list_head(tail)->value.int_ == 2, "First element of tail should be 2");

    l = list_new();
    for (size_t i = 0; i < 4; ++i) {
        l = list_prepend(l, numbers[i]);
        mu_assert(list_size(l) == i + 1, "List should grow by one in every step");
    }
    mu_assert(list_size(l) == 4, "Number  of prepended elemets should be 4");
    mu_assert(list_head(l)->value.int_ == 4, "First element should be 4");

    l2 = list_mutable_copy(l);
    mu_assert(list_size(l) == list_size(l2), "Copied list must have equal length");

    mu_assert(l2 != l, "Copies need to be different!");
    mu_assert(l2->head != l->head, "head ptrs must be different");

    cur = l->head;
    cur2 = l2->head;
    i = 0;
    while (cur != NULL && cur2 != NULL) {
        mu_assert(cur->val == numbers[3 - i], "Wrong data reference in src");
        mu_assert(cur2->val == numbers[3 - i], "Wrong data reference in dst");
        cur = cur->next;
        cur2 = cur2->next;
        ++i;
    }
    mu_assert(cur == NULL && cur2 == NULL, "copy has different length");

    l = list_new();
    mu_assert(list_head(l) == NULL, "Empty list should have a NULL head");
    mu_assert(list_size(list_tail(l)) == 0, "Empty list should have an empty tail");
    l = list_append(l, numbers[0]);
    mu_assert(list_head(l)->value.int_ == 1, "Head of one-element list should be 1");
    mu_assert(list_size(list_tail(l)) == 0, "One-element list should have an empty tail");

    /* iterate over list using combination of head/tail calls */
    const List *a = list_new();
    const List *p = list_new();
    // use two lists, one built w/ prepending, one with appending
    for (size_t i = 0; i < 10; ++i) {
        a = list_append(a, numbers[0]);
        p = list_prepend(p, numbers[0]);
    }
    const Value *head;
    size_t j = 10;
    while((head = list_head(a)) != NULL) {
        mu_assert(list_size(a) == j, "tail size should decrease linearly");
        a = list_tail(a);
        j--;
        mu_assert(list_size(a) == j, "tail size should decrease linearly");
    }
    mu_assert(list_size(a) == 0, "Empty tail should have size zero");
    j = 10;
    while((head = list_head(p)) != NULL) {
        mu_assert(list_size(p) == j, "tail size should decrease linearly");
        p = list_tail(p);
        j--;
        mu_assert(list_size(p) == j, "tail size should decrease linearly");
    }
    mu_assert(list_size(p) == 0, "Empty tail should have size zero");

    /* list_nth: standard case */
    l = list_new();
    for (size_t i = 0; i < 4; ++i) {
        l = list_append(l, numbers[i]);
    }
    mu_assert(list_nth(l, 0) == numbers[0], "Wrong index offset");
    mu_assert(list_nth(l, 1) == numbers[1], "Wrong index offset");
    mu_assert(list_nth(l, 2) == numbers[2], "Wrong index offset");
    mu_assert(list_nth(l, 3) == numbers[3], "Wrong index offset");
    mu_assert(list_nth(l, 4) == NULL, "Out of range index should return NULL");

    /* list_nth: empty list */
    l = list_new();
    mu_assert(list_nth(l, 0) == NULL, "Out of range index should return NULL");
    mu_assert(list_nth(l, 4) == NULL, "Out of range index should return NULL");

    return 0;
}

int tests_run = 0;

static char *test_suite()
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

