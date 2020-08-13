#include "vm/object.h"
#include <string.h>
#include "vm/mem.h"
#include "vm/fnv1a.h"

VmObject *obj_new(size_t size, VmObjectType type)
{
    VmObject *obj = (VmObject *) mem_reallocate(NULL, size);
    obj->type = type;
    return obj;
}

VmObject *obj_copy(const VmObject *src)
{
    switch(src->type) {
    case VM_OBJ_STRING:
        return (VmObject *) obj_string_copy((const VmString *) src);
    default:
        LOG_WARNING("Unknown VM object type: %u", src->type);
        return NULL;
    }
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

void obj_print(VmObject *obj)
{
    switch(obj->type) {
    case VM_OBJ_STRING:
        printf("%s", ((VmString *) obj)->str);
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
    obj->hash = fnv1a(str, len);  // hash without terminal \0
    return obj;
}


VmString *obj_string_copy(const VmString *src)
{
    VmString *obj = (VmString *) obj_new(sizeof(VmString), VM_OBJ_STRING);
    obj->str = mem_reallocate(NULL, src->len + 1);
    obj->len = src->len;
    strncpy(obj->str, src->str, src->len);
    obj->str[obj->len] = '\0';
    obj->hash = src->hash;
    return obj;
}

void obj_string_delete(VmString *obj)
{
    mem_reallocate(obj->str, 0);
    mem_reallocate(obj, 0);
}
