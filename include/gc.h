/*
 * gc.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __GC_H__
#define __GC_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct AllocationMap;

typedef struct GarbageCollector {
    struct AllocationMap* allocs; // allocation map
    bool paused;         // switch gc on/off
    double downsize_load_limit;
    double upsize_load_limit;
    double sweep_load_limit;  // limit for starting a sweep
    void *bos;           // bottom of stack
} GarbageCollector;

extern GarbageCollector gc;

void gc_start(GarbageCollector* gc, void* bos, double downsize_load_factor,
              double upsize_load_factor, double sweep_load_factor);
void gc_stop(GarbageCollector* gc);
void gc_pause(GarbageCollector* gc);
void gc_resume(GarbageCollector* gc);
size_t gc_run(GarbageCollector* gc);

void* gc_malloc(GarbageCollector* gc, size_t size);
void* gc_malloc_opts(GarbageCollector* gc, size_t size, void (*dtor)(void*));
void* gc_calloc(GarbageCollector* gc, size_t count, size_t size);
void* gc_calloc_opts(GarbageCollector* gc, size_t count, size_t size, void (*dtor)(void*));
void* gc_realloc(GarbageCollector* gc, void* ptr, size_t size);
void gc_free(GarbageCollector* gc, void* ptr);
char* gc_strdup (GarbageCollector* gc, const char* s);

#endif /* !__GC_H__ */
