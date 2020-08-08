#include "vm/location.h"

#include "vm/mem.h"


LocationArray* locations_new()
{
    LocationArray* ins = mem_reallocate(NULL, sizeof(LocationArray));
    ins->size = 0;
    ins->capacity = 0;
    ins->locs= NULL;
    return ins;
}

void locations_delete(LocationArray* ls)
{
    mem_reallocate(ls->locs, 0);
    mem_reallocate(ls, 0);
}

static void locations_upsize(LocationArray* ls)
{
   // up-size in powers of 2
   size_t capacity = ls->capacity < 16 ? 16 : ls->capacity * 2;
   ls->locs= mem_reallocate(ls->locs, capacity * sizeof(Location));
   ls->capacity = capacity;
}

void locations_append(LocationArray *ls, Location loc)
{
    // Check if the array is large enough to hold the next loc,
    // up-size if required
    if (ls->capacity < ls->size + 1) {
        locations_upsize(ls);
    }
    // Store loc and advance
    ls->locs[ls->size] = loc;
    ls->size++;
}

const Location* locations_at(LocationArray* ls, size_t n)
{
    if (n <= ls->size) {
        return &ls->locs[n];
    }
    return NULL;
}

