#include "vm/value.h"
#include "vm/mem.h"

ValueArray *values_new()
{
    ValueArray *vals = (ValueArray *) mem_reallocate(NULL, sizeof(ValueArray));
    *vals = (ValueArray) {
        .size = 0, .capacity = 0, .values = NULL
    };
    return vals;
}

void values_delete(ValueArray *vals)
{
    mem_reallocate(vals->values, 0);
    mem_reallocate(vals, 0);
}

static void values_upsize(ValueArray *vals)
{
    // up-size in powers of 2
    size_t capacity = vals->capacity < 16 ? 16 : vals->capacity * 2;
    vals->values = mem_reallocate(vals->values, capacity * sizeof(Value));
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
    switch(value.type) {
        case VM_VALUE_NIL:
            printf("NIL");
            break;
        case VM_VALUE_BOOL:
            printf(VM_AS_BOOL(value) ? "True" : "False");
            break;
        case VM_VALUE_NUMBER:
            printf("%g", VM_AS_NUMBER(value));
            break;
        case VM_VALUE_OBJ:
            printf("Object@%p", VM_AS_OBJ(value));
    }
}
