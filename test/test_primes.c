/*
 * test_primes.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include "minunit.h"
#include "primes.h"

static char* test_primes()
{
    /*
     * Test a few known cases.
     */
    mu_assert(!is_prime(0), "Prime test failure for 0");
    mu_assert(!is_prime(1), "Prime test failure for 1");
    mu_assert(is_prime(2), "Prime test failure for 2");
    mu_assert(is_prime(3), "Prime test failure for 3");
    mu_assert(!is_prime(12742382), "Prime test failure for 12742382");
    mu_assert(is_prime(611953), "Prime test failure for 611953");
    mu_assert(is_prime(479001599), "Prime test failure for 479001599");
    return 0;
}

