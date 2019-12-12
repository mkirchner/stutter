/*
 * test_gc.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>
#include "minunit.h"
#include "log.h"

#include "gc.h"

void dtor(void* ptr)
{
    ptr = NULL;
}

void use_some_mem(GarbageCollector* gc)
{
    int *ints = gc_malloc(gc, sizeof(int) * 3);
    ints[3] = 100;
}

void use_some_serious_mem(GarbageCollector* gc, int** ints)
{
    // request the memory, then immediately drop it
    for (size_t i=0; i<32; ++i) {
        ints[i] = gc_malloc(gc, sizeof(int));
    }
}


static char* test_gc()
{
    int bos;
    printf("GC tests\n");
    GarbageCollector gc_;  // do not use the global GC for testing
    gc_start_ext(&gc_, &bos, 4, 4, 0.2, 0.8, 0.5);
    int *ints = gc_malloc(&gc_, sizeof(int) * 5);
    ints[5] = 100;
    use_some_mem(&gc_);
    gc_run(&gc_);

    int** many_ints = gc_malloc(&gc_, 32 * sizeof(int*));
    use_some_serious_mem(&gc_, many_ints);
    size_t n = gc_run(&gc_);
    LOG_DEBUG("Collected %lu bytes", n);
    for (size_t i=0; i<32; ++i) {
        gc_free(&gc_, many_ints[i]);
    }
    LOG_DEBUG("Retrying%s", "");

    many_ints = gc_malloc(&gc_, 32 * sizeof(int*));
    use_some_serious_mem(&gc_, many_ints);
    many_ints = NULL;
    n = gc_run(&gc_);
    LOG_DEBUG("Collected %lu bytes", n);

    gc_stop(&gc_);
    return NULL;
}
