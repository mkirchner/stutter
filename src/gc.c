/*
 * gc.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "gc.h"
#include "log.h"

#include <errno.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "primes.h"

/*
 * Store allocations in a hash map with the pointer address
 * as the key.
 */

#define GC_TAG_NONE 0x00
#define GC_TAG_MARK 0x01


typedef struct Allocation {
    void* ptr;                // mem pointer (tagged for mark&sweep)
    size_t size;              // allocated size in bytes
    char tag;
    void (*dtor)(void*);      // destructor
    struct Allocation* next;  // separate chaining
} Allocation;

typedef struct AllocationMap {
    size_t capacity;
    size_t size;
    Allocation** allocs;
} AllocationMap;

static double gc_allocation_map_load_factor(AllocationMap* am)
{
    return (double) am->size / (double) am->capacity;
}

static Allocation* gc_allocation_new(void* ptr, size_t size, void (*dtor)(void*))
{
    Allocation* a = (Allocation*) malloc(sizeof(Allocation));
    a->ptr = ptr;
    a->size = size;
    a->tag = GC_TAG_NONE;
    a->dtor = dtor;
    a->next = NULL;
    return a;
}

static void gc_allocation_delete(Allocation* a)
{
    free(a);
}

static AllocationMap* gc_allocation_map_new(size_t capacity)
{
    AllocationMap* am = (AllocationMap*) malloc(sizeof(AllocationMap));
    am->capacity = next_prime(capacity);
    am->size = 0;
    am->allocs = (Allocation**) calloc(capacity, sizeof(Allocation*));
    LOG_DEBUG("Created allocation map (cap=%ld, siz=%ld)", am->capacity, am->size);
    return am;
}

static void gc_allocation_map_delete(AllocationMap* am)
{
    // Iterate over the map
    LOG_DEBUG("Deleting allocation map (cap=%ld, siz=%ld)",
              am->capacity, am->size);
    Allocation *alloc, *tmp;
    for (size_t i = 0; i < am->capacity; ++i) {
        if ((alloc = am->allocs[i])) {
            // Make sure to follow the chain inside a bucket
            while (alloc) {
                tmp = alloc;
                alloc = alloc->next;
                // free the management structure
                gc_allocation_delete(tmp);
            }
        }
    }
    free(am->allocs);
    free(am);
}

static size_t gc_hash(void *ptr)
{
    return ((uintptr_t)ptr) >> 3;
}

static void gc_allocation_map_resize(AllocationMap* am, size_t new_capacity)
{
    // Replaces the existing items array in the hash table
    // with a resized one and pushes items into the new, correct buckets
    LOG_DEBUG("Resizing allocation map (cap=%ld, siz=%ld) -> (cap=%ld)",
              am->capacity, am->size, new_capacity);
    Allocation** resized_allocs = calloc(new_capacity, sizeof(Allocation*));

    for (size_t i = 0; i < am->capacity; ++i) {
        Allocation* alloc = am->allocs[i];
        while(alloc) {
            Allocation* next_alloc = alloc->next;
            size_t new_index = gc_hash(alloc->ptr) % new_capacity;
            alloc->next = resized_allocs[new_index];
            resized_allocs[new_index] = alloc;
            alloc = next_alloc;
        }
    }
    free(am->allocs);
    am->capacity = new_capacity;
    am->allocs = resized_allocs;
}


static Allocation* gc_allocation_map_put(AllocationMap* am, void* ptr,
        size_t size, void (*dtor)(void*))
{
    // hash
    size_t index = gc_hash(ptr) % am->capacity;
    LOG_DEBUG("PUT request for allocation ix=%ld", index);
    // create item
    Allocation* alloc = gc_allocation_new(ptr, size, dtor);
    Allocation* cur = am->allocs[index];
    // update if exists
    Allocation* prev = NULL;
    while(cur != NULL) {
        if (cur->ptr == ptr) {
            // found it
            alloc->next = cur->next;
            if (!prev) {
                // position 0
                am->allocs[index] = alloc;
            } else {
                // in the list
                prev->next = alloc;
            }
            gc_allocation_delete(cur);
            LOG_DEBUG("Successful UPDATE for allocation ix=%ld", index);
            return alloc;
        }
        prev = cur;
        cur = cur->next;
    }
    // insert (at front of list)
    cur = am->allocs[index];
    alloc->next = cur;
    am->allocs[index] = alloc;
    am->size++;
    LOG_DEBUG("Successful PUT for allocation ix=%ld", index);
    double load_factor = gc_allocation_map_load_factor(am);
    if (load_factor > 0.7) {
        LOG_DEBUG("Load factor %0.3g > 0.7. Triggering resize.", load_factor);
        gc_allocation_map_resize(am, next_prime(am->capacity * 2));
    }
    return alloc;
}

static Allocation* gc_allocation_map_get(AllocationMap* am, void* ptr)
{
    size_t index = gc_hash(ptr) % am->capacity;
    // LOG_DEBUG("GET request for allocation ix=%ld (ptr=%p)", index, ptr);
    Allocation* cur = am->allocs[index];
    while(cur) {
        if (cur->ptr == ptr) return cur;
        cur = cur->next;
    }
    return NULL;
}

static void gc_allocation_map_remove(AllocationMap* am, void* ptr)
{
    // ignores unknown keys
    size_t index = gc_hash(ptr) % am->capacity;
    Allocation* cur = am->allocs[index];
    Allocation* prev = NULL;
    while(cur != NULL) {
        if (cur->ptr == ptr) {
            // found it
            if (!prev) {
                // first item in list
                am->allocs[index] = cur->next;
            } else {
                // not the first item in the list
                prev->next = cur->next;
            }
            gc_allocation_delete(cur);
            am->size--;
        } else {
            // move on
            prev = cur;
            cur = cur->next;
        }
        cur = cur->next;
    }
    // FIXME: move resizing into a gc function (which has the GC params available)
    double load_factor = gc_allocation_map_load_factor(am);
    if (load_factor < 0.1) {
        LOG_DEBUG("Load factor %0.3g < 0.1. Triggering resize.", load_factor);
        gc_allocation_map_resize(am, next_prime(am->capacity / 2));
    }
}

void* gc_malloc(GarbageCollector* gc, size_t size)
{
    return gc_malloc_opts(gc, size, NULL);
}

void* gc_malloc_opts(GarbageCollector* gc, size_t size, void(*dtor)(void*))
{
    // FIXME: add call to GC here
    void* ptr = malloc(size);
    if (ptr) {
        LOG_DEBUG("Allocated %zu bytes at %p", size, (void*) ptr);
        Allocation* alloc = gc_allocation_map_put(gc->allocs, ptr, size, dtor);
        LOG_DEBUG("Returning %zu managed bytes at %p", size, (void*) alloc->ptr);
        if (alloc) {
            return alloc->ptr;
        }
    }
    return ptr;
}

void* gc_calloc(GarbageCollector* gc, size_t count, size_t size)
{
    return gc_calloc_opts(gc, count, size, NULL);
}

void* gc_calloc_opts(GarbageCollector* gc, size_t count, size_t size,
                     void(*dtor)(void*))
{
    void* ptr = calloc(count, size);
    if (ptr) {
        Allocation* alloc = gc_allocation_map_put(gc->allocs, ptr, size, dtor);
        if (alloc) {
            return alloc->ptr;
        }
    }
    return ptr;
}

void* gc_realloc(GarbageCollector* gc, void* p, size_t size)
{
    // FIXME: this assumes that the gc_allocation_map_* calls cannot fail
    Allocation* alloc = gc_allocation_map_get(gc->allocs, p);
    if (p && !alloc) {
        // the user passed an unknown pointer
        errno = EINVAL;
        return NULL;
    }
    void* q = realloc(p, size);
    if (!q) {
        // realloc failed but p is still valid
        return NULL;
    }
    if (!p) {
        // allocation, not reallocation
        Allocation* alloc = gc_allocation_map_put(gc->allocs, q, size, NULL);
        return alloc->ptr;
    }
    if (p == q) {
        // successful reallocation w/o copy
        alloc->size = size;
    } else {
        // successful reallocation w/ copy
        gc_allocation_map_remove(gc->allocs, p);
        gc_allocation_map_put(gc->allocs, p, size, alloc->dtor);
    }
    return q;
}

void gc_free(GarbageCollector* gc, void* ptr)
{
    Allocation* alloc = gc_allocation_map_get(gc->allocs, ptr);
    if (alloc->dtor) {
        alloc->dtor(ptr);
    }
    free(ptr);
    gc_allocation_map_remove(gc->allocs, ptr);
}

void gc_start(GarbageCollector* gc, void* bos)
{
    gc->allocs = gc_allocation_map_new(1024);
    gc->paused = false;
    gc->load_factor = 0.9;
    gc->sweep_factor = 0.5;
    gc->bos = bos;
    LOG_DEBUG("Created new garbage collector (cap=%ld, siz=%ld).",
              gc->allocs->capacity, gc->allocs->size);
}

void gc_stop(GarbageCollector* gc)
{
    // FIXME
    gc_allocation_map_delete(gc->allocs);
    return;
}

void gc_pause(GarbageCollector* gc)
{
    gc->paused = true;
}

void gc_resume(GarbageCollector* gc)
{
    gc->paused = false;
}

void gc_mark_ptr(GarbageCollector* gc, void* ptr)
{
    // FIXME: should be called mark_alloc
    Allocation* alloc = gc_allocation_map_get(gc->allocs, ptr);
    if (alloc) {
        LOG_DEBUG("Marking allocation (ptr=%p)", ptr);
        alloc->tag = GC_TAG_MARK;  // FIXME
    }
}

void gc_mark_stack(GarbageCollector* gc)
{
    LOG_DEBUG("Marking the stack (gc@%p) in increments of %ld", (void*) gc, sizeof(void*));
    char dummy;
    void **tos = (void**) &dummy;
    void **bos = gc->bos;
    if (tos > bos) {
        void** tmp = tos;
        tos = gc->bos;
        bos = tmp;
    }
    for (void** p = tos; p < bos; ++p) {
        gc_mark_ptr(gc, *p);
    }
}

void gc_mark_heap(GarbageCollector* gc)
{
    LOG_DEBUG("Marking the heap (gc@%p, cap=%ld)", (void*) gc, gc->allocs->capacity);
    // go through all allocated chunks and see if any of them contain
    // a pointer to another chunk
    for (size_t i = 0; i < gc->allocs->capacity; ++i) {
        Allocation* chunk = gc->allocs->allocs[i];
        // iterate over open addressing
        while (chunk) {
            LOG_DEBUG("Checking contents of allocation: %p=(ptr=%p, siz=%ld, tag=%c)",
                      (void*) chunk, (void*) chunk->ptr, chunk->size, chunk->tag);
            for (void** p = (void**) chunk->ptr;
                    p < (void**) chunk->ptr + chunk->size;
                    ++p) {
                gc_mark_ptr(gc, *p);
            }
            chunk = chunk->next;
        }
    }
}

void gc_mark(GarbageCollector* gc)
{
    // FIXME: also scan BSS ?

    LOG_DEBUG("Initiating GC mark (gc@%p)", (void*) gc);
    // scan the heap
    gc_mark_heap(gc);

    // dump registers onto stack and scan the stack
    void (*volatile _mark_stack)(GarbageCollector*) = gc_mark_stack;
    jmp_buf ctx;
    memset(&ctx, 0, sizeof(jmp_buf));
    setjmp(ctx);
    _mark_stack(gc);
}

void gc_sweep(GarbageCollector* gc)
{
    LOG_DEBUG("Initiating GC sweep (gc@%p)", (void*) gc);
    for (size_t i = 0; i < gc->allocs->capacity; ++i) {
        Allocation* chunk = gc->allocs->allocs[i];
        // iterate over open addressing
        while (chunk) {
            if (chunk->tag == GC_TAG_MARK) {
                LOG_DEBUG("Found used allocation %p (ptr=%p)", (void*) chunk, (void*) chunk->ptr);
                // unmark
                chunk->tag = GC_TAG_NONE;
            } else {
                LOG_DEBUG("Found unused allocation %p (ptr=%p)", (void*) chunk, (void*) chunk->ptr);
                // no reference to this chunk, hence delete it
                free(chunk->ptr);
                // and remove it from the bookkeeping
                gc_allocation_map_remove(gc->allocs, chunk->ptr);
            }
            chunk = chunk->next;
        }
    }
}

void gc_run(GarbageCollector* gc)
{
    LOG_DEBUG("Initiating GC run (gc@%p)", (void*) gc);
    gc_mark(gc);
    gc_sweep(gc);
}

