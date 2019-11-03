/*
 * test_gc.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>

#include "gc.h"

void dtor(void* ptr)
{
    ptr = NULL;
}

static char* test_gc()
{
    int bos;
    printf("GC tests\n");
    GarbageCollector gc;
    gc_start(&gc, &bos);
    int *ints = gc_malloc(&gc, sizeof(int)*10);
    ints[5] = 100;
    gc_stop(&gc);
    return NULL;
}
