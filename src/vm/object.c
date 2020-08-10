#include "vm/object.h"
#include <string.h>
#include "vm/mem.h"

VmObject *obj_new(size_t size, VmObjectType type)
{
    VmObject *obj = (VmObject *) mem_reallocate(NULL, size);
    obj->type = type;
    return obj;
}

void obj_delete(VmObject *obj)
{
    switch(obj->type) {
    case VM_OBJ_STRING:
        obj_string_delete((VmString *) obj); // downcast
        break;
    default:
        LOG_WARNING("Unknown VM object type: %u", obj->type);
    }
}

void obj_print(VmObject* obj)
{
    switch(obj->type) {
    case VM_OBJ_STRING:
        printf("%s", ((VmString*) obj)->str);
        break;
    default:
      LOG_WARNING("Unknown VM object type: %u", obj->type);
    }
}


VmString *obj_string_new(size_t len, const char *str)
{
    VmString *obj = (VmString *) obj_new(sizeof(VmString), VM_OBJ_STRING);
    obj->str = mem_reallocate(NULL, len + 1);
    strncpy(obj->str, str, len);
    obj->str[len] = '\0';
    return obj;
}

void obj_string_delete(VmString *obj)
{
    mem_reallocate(obj->str, 0);
    mem_reallocate(obj, 0);
}
