#include "vm/value.h"
#include "vm/mem.h"

ValueArray* values_new()
{
    ValueArray* vals = (ValueArray*) mem_reallocate(NULL, sizeof(ValueArray));
    *vals = (ValueArray) { .size = 0, .capacity = 0, .values = NULL };
    return vals;
}

void values_delete(ValueArray* vals)
{
    mem_reallocate(vals->values, 0);
    mem_reallocate(vals, 0);
}

static void values_upsize(ValueArray* vals)
{
   // up-size in powers of 2
   size_t capacity = vals->capacity < 16 ? 16 : vals->capacity * 2;
   vals->values= mem_reallocate(vals->values, capacity * sizeof(Value));
   vals->capacity = capacity;
}

void values_append(ValueArray *vals, Value byte)
{
    // Check if the array is large enough to hold the next byte,
    // up-size if required
    if (vals->capacity < vals->size + 1) {
        values_upsize(vals);
    }
    vals->values[vals->size] = byte;
    vals->size++;
}

size_t values_size(ValueArray *vals)
{
    return vals->size;
}

void value_print(Value value)
{
    printf("%g", value);
}
