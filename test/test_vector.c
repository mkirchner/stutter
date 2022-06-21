#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "../src/vector.c"

static char *test_vector()
{
    // creation
    Vector *a = vector_new(sizeof(int));
    mu_assert(a != NULL, "New vector should not be NULL");

    // get/set
    int i = 42;
    vector_push_back(a, &i, 1);
    mu_assert(vector_size(a) == 1, "Size should be 1");
    mu_assert(vector_capacity(a) == 1, "Capacity should be 1");
    i++;
    vector_push_back(a, &i, 1);
    i++;
    vector_push_back(a, &i, 1);
    i++;
    vector_push_back(a, &i, 1);
    mu_assert(vector_size(a) == 4, "Size should be 4");
    mu_assert(vector_capacity(a) == 4, "Capacity should be 4");
    mu_assert(*vector_typed_at(a, 0, int) == 42, "42 != 42");
    mu_assert(*vector_typed_at(a, 1, int) == 43, "43 != 43 1");
    mu_assert(*vector_typed_at(a, 2, int) == 44, "44 != 44");
    mu_assert(*vector_typed_at(a, 3, int) == 45, "45 != 45");

    int i2[4] = {46, 47, 48, 49};
    vector_push_back(a, i2, 4);
    mu_assert(vector_size(a) == 8, "Size should be 8");
    mu_assert(vector_capacity(a) == 8, "Capacity should be 8");
    mu_assert(*vector_typed_at(a, 0, int) == 42, "42 != 42");
    mu_assert(*vector_typed_at(a, 1, int) == 43, "43 != 43");
    mu_assert(*vector_typed_at(a, 2, int) == 44, "44 != 44");
    mu_assert(*vector_typed_at(a, 3, int) == 45, "45 != 45");
    mu_assert(*vector_typed_at(a, 4, int) == 46, "46 != 46");
    mu_assert(*vector_typed_at(a, 5, int) == 47, "47 != 47");
    mu_assert(*vector_typed_at(a, 6, int) == 48, "48 != 48");
    mu_assert(*vector_typed_at(a, 7, int) == 49, "49 != 49");

    int j = 41;
    vector_push_front(a, &j, 1);
    mu_assert(*vector_typed_at(a, 0, int) == 41, "push fail 41");
    j--;
    vector_push_front(a, &j, 1);
    mu_assert(*vector_typed_at(a, 0, int) == 40, "push fail 40");
    mu_assert(*vector_typed_at(a, 1, int) == 41, "assert fail 41");
    j--;
    vector_push_front(a, &j, 1);
    mu_assert(*vector_typed_at(a, 0, int) == 39, "push fail 39");
    mu_assert(*vector_typed_at(a, 1, int) == 40, "assert fail 40");
    mu_assert(*vector_typed_at(a, 2, int) == 41, "assert fail 41");
    j--;
    vector_push_front(a, &j, 1);
    mu_assert(*vector_typed_at(a, 0, int) == 38, "push fail 38");
    mu_assert(*vector_typed_at(a, 1, int) == 39, "assert fail 39");
    mu_assert(*vector_typed_at(a, 2, int) == 40, "assert fail 40");
    mu_assert(*vector_typed_at(a, 3, int) == 41, "assert fail 41");

    mu_assert(vector_size(a) == 12, "Size should be 12");
    mu_assert(vector_capacity(a) == 16, "Capacity should be 16");
    mu_assert(*vector_typed_at(a, 0, int) == 38, "38 != 38");
    mu_assert(*vector_typed_at(a, 1, int) == 39, "39 != 39");
    mu_assert(*vector_typed_at(a, 2, int) == 40, "40 != 40");
    mu_assert(*vector_typed_at(a, 3, int) == 41, "41 != 41");
    mu_assert(*vector_typed_at(a, 4, int) == 42, "42 != 42");
    mu_assert(*vector_typed_at(a, 5, int) == 43, "43 != 43");
    mu_assert(*vector_typed_at(a, 6, int) == 44, "44 != 44");
    mu_assert(*vector_typed_at(a, 7, int) == 45, "45 != 45");
    mu_assert(*vector_typed_at(a, 8, int) == 46, "46 != 46");
    mu_assert(*vector_typed_at(a, 9, int) == 47, "47 != 47");
    mu_assert(*vector_typed_at(a, 10, int) == 48, "48 != 48");
    mu_assert(*vector_typed_at(a, 11, int) == 49, "49 != 49");

    mu_assert(*vector_typed_pop_back(a, int) == 49, "49 pop fail");
    mu_assert(vector_size(a) == 11, "Wrong size after popping 49");
    for (int i = 10; i >= 0; --i) {
        vector_typed_pop_back(a, int);
    }
    mu_assert(vector_size(a) == 0, "Wrong size after emptying vector");
    mu_assert(vector_pop_back(a) == NULL, "Pop from empty vector must return NULL");

    int i3[4] = {0, 1, 2, 3};
    vector_push_back(a, i3, 4);
    mu_assert(vector_size(a) == 4, "Wrong size after refill");
    mu_assert(*vector_typed_pop_front(a, int) == 0, "Pop front 0 fail");
    mu_assert(vector_size(a) == 3, "Pop front 0 size fail");
    mu_assert(*vector_typed_pop_front(a, int) == 1, "Pop front 1 fail");
    mu_assert(*vector_typed_pop_front(a, int) == 2, "Pop front 2 fail");
    mu_assert(*vector_typed_pop_front(a, int) == 3, "Pop front 3 fail");
    mu_assert(vector_typed_pop_front(a, int) == NULL, "Pop on empty vector must return NULL");

    int i4[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    vector_push_back(a, i4, 11);
    Vector *cpy = vector_dup(a);
    mu_assert(vector_size(a) == vector_size(cpy), "Size of copy must equal size of original");
    for (size_t k = 0; k < 11; ++k) {
        vector_pop_front(a);
    }
    for (int k = 0; k < 11; ++k) {
        mu_assert(*vector_typed_pop_front(cpy, int) == k, "Copy must be invaraint to modification of original");
    }


    // clean up
    vector_delete(a);
    return 0;
}

int tests_run = 0;

static char *test_suite()
{
    mu_run_test(test_vector);
    return 0;
}

int main()
{
    printf("---=[ Vector tests\n");
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
