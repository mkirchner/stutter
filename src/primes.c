/*
 * primes.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include "primes.h"


bool is_prime(size_t n)
{
    // https://stackoverflow.com/questions/1538644/c-determine-if-a-number-is-prime
    if (n <= 1)
        return false;
    else if (n <= 3 && n > 1)
        return true;            // as 2 and 3 are prime
    else if (n % 2==0 || n % 3==0)
        return false;     // check if n is divisible by 2 or 3
    else {
        for (size_t i=5; i*i<=n; i+=6) {
            if (n % i == 0 || n%(i + 2) == 0)
                return false;
        }
        return true;
    }
}

size_t next_prime(size_t n)
{
    while (!is_prime(n)) ++n;
    return n;
}
