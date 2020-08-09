#ifndef __VALUE_H__
#define __VALUE_H__

#include "vm/common.h"

#define VM_IS_NIL(value) ((value).type == VM_VALUE_NIL)
#define VM_IS_BOOL(value) ((value).type == VM_VALUE_BOOL)
#define VM_IS_NUMBER(value) ((value).type == VM_VALUE_NUMBER)
#define VM_IS_OBJ(value) ((value).type == VM_VALUE_OBJ)

#define VM_NIL_VAL ((Value){VM_VALUE_NIL, {.number = 0.}})
#define VM_BOOL_VAL(cval) ((Value){VM_VALUE_BOOL, {.boolean = cval}})
#define VM_NUMBER_VAL(cval) ((Value){VM_VALUE_NUMBER, {.number = cval}})
#define VM_OBJ_VAL(obj_ptr) ((Value){VM_VALUE_OBJ, {.obj = obj_ptr}})

#define VM_AS_BOOL(value) ((value).as.boolean)
#define VM_AS_NUMBER(value) ((value).as.number)
#define VM_AS_OBJ(value) ((value).as.obj)

typedef enum {
    VM_VALUE_NIL,
    VM_VALUE_BOOL,
    VM_VALUE_NUMBER,
    VM_VALUE_OBJ
} VmValueType;

typedef struct {} Object;  // FIXME

typedef struct {
    VmValueType type;
    union {
        bool boolean;
        double number;
        Object* obj;
    } as;
} Value;

typedef struct {
    size_t size;
    size_t capacity;
    Value *values;
} ValueArray;

ValueArray *values_new();
void values_delete(ValueArray *);
void values_append(ValueArray *, Value);
size_t values_size(ValueArray *);
void value_print(Value);

#endif /* !__VALUE_H__ */
