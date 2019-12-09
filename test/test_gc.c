/*
 * test_gc.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>
#include "minunit.h"

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


static char* test_gc()
{
    int bos;
    printf("GC tests\n");
    GarbageCollector gc_;  // do not use the global GC for testing
    gc_start(&gc_, &bos, 0.0, 0.0, 0.6);
    int *ints = gc_malloc(&gc_, sizeof(int) * 5);
    ints[5] = 100;
    use_some_mem(&gc_);
    gc_run(&gc_);
    gc_stop(&gc_);
    return NULL;
}
