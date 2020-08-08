#ifndef __LOCATION_H__
#define __LOCATION_H__

#include "vm/common.h"

typedef struct {
    size_t row;
    size_t col;
} Location;

typedef struct {
    size_t size;
    size_t capacity;
    Location *locs;
} LocationArray;;

LocationArray* locations_new();
void locations_delete(LocationArray*);
void locations_append(LocationArray*, Location);
const Location* locations_at(LocationArray*, size_t n);

#endif /* !__LOCATION_H__ */
