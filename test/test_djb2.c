/*
 * test_djb2.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include "djb2.h"
#include "minunit.h"


static char* test_djb2()
{
    /* Basic testing for the djb2 hash: can we call it and
     * does it return a reasonable result?
     */
    unsigned long hash = djb2("");
    mu_assert(hash == 5381, "djb2 implementation error");
    hash = djb2("Hello World!");
    mu_assert(hash != 5381, "djb2 addition failure");
    return 0;
}

