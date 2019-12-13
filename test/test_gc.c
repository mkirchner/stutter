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

#include "../src/gc.c"

#undef LOGLEVEL
#define LOGLEVEL LOGLEVEL_INFO

static char sentinel[] = "NULL";

void dtor(void* ptr)
{
    ptr = (void*) sentinel;
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


static char* test_gc_allocation_new_delete()
{
    int* ptr = malloc(sizeof(int));
    Allocation* a = gc_allocation_new(ptr, sizeof(int), dtor);
    mu_assert(a != NULL, "Allocation should return non-NULL");
    mu_assert(a->ptr == ptr, "Allocation should contain original pointer");
    mu_assert(a->size == sizeof(int), "Size of mem pointed to should not change");
    mu_assert(a->tag == GC_TAG_NONE, "Annotation should initially be untagged");
    mu_assert(a->dtor == dtor, "Destructor pointer should not change");
    mu_assert(a->next == NULL, "Annotation should initilally be unlinked");
    gc_allocation_delete(a);
    free(ptr);
    return NULL;
}


static char* test_gc_allocation_map_new_delete()
{
    /* Standard invocation */
    AllocationMap* am = gc_allocation_map_new(8, 16, 0.5, 0.2, 0.8);
    mu_assert(am->min_capacity == 11, "True min capacity should be next prime");
    mu_assert(am->capacity == 17, "True capacity should be next prime");
    mu_assert(am->size == 0, "Allocation map should be initialized to empty");
    mu_assert(am->sweep_limit == 8, "Incorrect sweep limit calculation");
    mu_assert(am->downsize_factor == 0.2, "Downsize factor should not change");
    mu_assert(am->upsize_factor == 0.8, "Upsize factor should not change");
    mu_assert(am->allocs != NULL, "Allocation map must not have a NULL pointer");
    gc_allocation_map_delete(am);

    /* Enforce min sizes */
    am = gc_allocation_map_new(8, 4, 0.5, 0.2, 0.8);
    mu_assert(am->min_capacity == 11, "True min capacity should be next prime");
    mu_assert(am->capacity == 11, "True capacity should be next prime");
    mu_assert(am->size == 0, "Allocation map should be initialized to empty");
    mu_assert(am->sweep_limit == 5, "Incorrect sweep limit calculation");
    mu_assert(am->downsize_factor == 0.2, "Downsize factor should not change");
    mu_assert(am->upsize_factor == 0.8, "Upsize factor should not change");
    mu_assert(am->allocs != NULL, "Allocation map must not have a NULL pointer");
    gc_allocation_map_delete(am);

    return NULL;
}


static char* test_gc_allocation_map_basic_get()
{
    AllocationMap* am = gc_allocation_map_new(8, 16, 0.5, 0.2, 0.8);

    /* Ask for something that does not exist */
    int* five = malloc(sizeof(int));
    Allocation* a = gc_allocation_map_get(am, five);
    mu_assert(a == NULL, "Empty allocation map must not contain any allocations");

    /* Create an entry and query it */
    *five = 5;
    a = gc_allocation_map_put(am, five, sizeof(int), NULL);
    mu_assert(a != NULL, "Result of PUT on allocation map must be non-NULL");
    mu_assert(am->size == 1, "Expect size of one-element map to be one");
    mu_assert(am->allocs != NULL, "AllocationMap must hold list of allocations");
    Allocation* b = gc_allocation_map_get(am, five);
    mu_assert(a == b, "Get should return the same result as put");
    mu_assert(a->ptr == b->ptr, "Pointers must not change between calls");
    mu_assert(b->ptr == five, "Get result should equal original pointer");

    /* Update the entry  and query */
    a = gc_allocation_map_put(am, five, sizeof(int), dtor);
    mu_assert(am->size == 1, "Expect size of one-element map to be one");
    mu_assert(a->dtor == dtor, "Setting the dtor should set the dtor");
    b = gc_allocation_map_get(am, five);
    mu_assert(b->dtor == dtor, "Failed to persist the dtor update");

    /* Delete the entry */
    gc_allocation_map_remove(am, five);
    mu_assert(am->size == 0, "After removing last item, map should be empty");
    Allocation* c = gc_allocation_map_get(am, five);
    mu_assert(c == NULL, "Empty allocation map must not contain any allocations");

    return NULL;
}


static char* test_gc_allocation_map_put_get_remove()
{
    /* Create a few data pointers */
    int** ints = malloc(64*sizeof(int*));
    for (size_t i=0; i<64; ++i) {
        ints[i] = malloc(sizeof(int));
    }

    /* Enforce separate chaining by disallowing up/downsizing.
     * The pigeonhole principle then states that we need to have at least one
     * entry in the hash map that has a separare chain with len > 1
     */
    AllocationMap* am = gc_allocation_map_new(32, 32, 1.1, 0.0, 1.1);
    Allocation* a;
    for (size_t i=0; i<64; ++i) {
        a = gc_allocation_map_put(am, ints[i], sizeof(int), NULL);
    }
    mu_assert(am->size == 64, "Maps w/ 64 elements should have size 64");
    /* Now update all of them with a new dtor */
    for (size_t i=0; i<64; ++i) {
        a = gc_allocation_map_put(am, ints[i], sizeof(int), dtor);
    }
    mu_assert(am->size == 64, "Maps w/ 64 elements should have size 64");
    /* Now delete all of them again */
    for (size_t i=0; i<64; ++i) {
        gc_allocation_map_remove(am, ints[i]);
    }
    mu_assert(am->size == 0, "Empty map must have size 0");
    /* And delete the entire map */
    gc_allocation_map_delete(am);

    /* Clean up the data pointers */
    for (size_t i=0; i<64; ++i) {
        free(ints[i]);
    }
    free(ints);

    return NULL;
}

static char* test_gc_basic_alloc_free()
{
    /* Create an array of pointers to an int. Then delete the pointer to
     * the containing array and check if all the contained are garbage
     * collected.
     */
    GarbageCollector gc_;
    int bos;
    gc_start_ext(&gc_, &bos, 32, 32, 0.2, 0.8, 0.5);
    int** ints = gc_calloc(&gc_, 16, sizeof(int*));
    for (size_t i=0; i<16; ++i) {
        ints[i] = gc_malloc(&gc_, sizeof(int));
        *ints[i] = 42;
    }
    for (size_t i=1; i<16; ++i) {
        printf("dist: %lu\n", ints[i]-ints[i-1]);
    }
    mu_assert(gc_.allocs->size == 17, "Wrong allocation map size");
    /* size_t n = gc_run(&gc_); */
    /* printf("Collected %lu bytes.", n); */
    //mu_assert(n == 0, "Wrong number of collected allocations");
    /* Manually delete the outer array */
    gc_free(&gc_, ints);
    mu_assert(gc_.allocs->size == 16, "Wrong allocation map size");
    /* Now kick off garbage collection */
    size_t n = gc_run(&gc_);
    printf("Collected %lu bytes, sizeof(int)=%lu, sizeof(*int)=%lu.", n, sizeof(int), sizeof(int*));
    mu_assert(n == 16 * sizeof(int), "Wrong number of collected allocations");
    return NULL;
}

static char* test_gc()
{
    int bos;
    GarbageCollector gc_;
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
