#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <stddef.h>

typedef struct {
    size_t line;
    size_t col;
} Location;

#endif /* !__LOCATION_H__ */
