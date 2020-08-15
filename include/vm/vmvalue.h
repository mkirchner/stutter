#ifndef __VALUE_H__
#define __VALUE_H__

#include "common.h"
#include "vm/object.h"

#define VM_IS_NIL(value) ((value).type == VM_VALUE_NIL)
#define VM_IS_BOOL(value) ((value).type == VM_VALUE_BOOL)
#define VM_IS_NUMBER(value) ((value).type == VM_VALUE_NUMBER)
#define VM_IS_OBJ(value) ((value).type == VM_VALUE_OBJ)

#define VM_NIL_VAL ((VmValue){VM_VALUE_NIL, {.number = 0.}})
#define VM_BOOL_VAL(cval) ((VmValue){VM_VALUE_BOOL, {.boolean = cval}})
#define VM_NUMBER_VAL(cval) ((VmValue){VM_VALUE_NUMBER, {.number = cval}})
#define VM_OBJ_VAL(obj_ptr) ((VmValue){VM_VALUE_OBJ, {.obj = (VmObject*) obj_ptr}})

#define VM_AS_BOOL(value) ((value).as.boolean)
#define VM_AS_NUMBER(value) ((value).as.number)
#define VM_AS_OBJ(value) ((value).as.obj)

typedef enum {
    VM_VALUE_NIL,
    VM_VALUE_BOOL,
    VM_VALUE_NUMBER,
    VM_VALUE_OBJ
} VmValueType;


typedef struct {
    VmValueType type;
    union {
        bool boolean;
        double number;
        VmObject *obj;
    } as;
} VmValue;

typedef struct {
    size_t size;
    size_t capacity;
    VmValue *values;
} ValueArray;

VmValue *vm_value_copy(const VmValue *src);
void vm_value_delete(VmValue *val);

ValueArray *values_new();
void values_delete(ValueArray *);
void values_append(ValueArray *, VmValue);
size_t values_size(ValueArray *);
void vm_value_print(VmValue);

#endif /* !__VALUE_H__ */
