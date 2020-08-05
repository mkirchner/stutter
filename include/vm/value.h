#ifndef __VALUE_H__
#define __VALUE_H__

#include "vm/common.h"

typedef double Value; // FIXME

typedef struct {
    size_t size;
    size_t capacity;
    Value *values;
} ValueArray;

ValueArray* values_new();
void values_delete(ValueArray*);
void values_append(ValueArray*, Value);
size_t values_size(ValueArray*);
void value_print(Value);

#endif /* !__VALUE_H__ */
