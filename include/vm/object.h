#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "common.h"

#define VM_AS_VMSTRING(obj) ((VmString*) obj)
#define VM_AS_CSTRING(obj) (((VmString*) obj)->str)

typedef enum {
    VM_OBJ_STRING
} VmObjectType;


typedef struct {
    VmObjectType type;
} VmObject;


typedef struct {
    VmObject obj;
    size_t len;
    char *str;
    uint32_t hash;
} VmString;

VmObject *obj_new(size_t size, VmObjectType type);
VmObject *obj_copy(const VmObject *src);
void obj_delete(VmObject *obj);

void obj_print(VmObject *obj);

VmString *obj_string_new(size_t len, const char *str);
VmString *obj_string_copy(const VmString *src);
void obj_string_delete(VmString *str);
#endif /* !__OBJECT_H__ */
