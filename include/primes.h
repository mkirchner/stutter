/*
 * primes.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __PRIMES_H__
#define __PRIMES_H__

#include <stdlib.h>
#include <stdbool.h>

bool is_prime(size_t n);
size_t next_prime(size_t n);

#endif /* !__PRIMES_H__ */
