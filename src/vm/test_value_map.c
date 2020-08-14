#include <assert.h>

#include "value_map.c" /* to get access to the static decls */

int main()
{
    LOG_INFO("Testing VM hash map");
    ValueMap *map = value_map_new();
    VmString *key = obj_string_new(3, "key");

    // empty map is empty
    assert(value_map_get(map, key) == NULL && "Empty map is not empty");

    // store/retrieve single value
    VmValue val = VM_NUMBER_VAL(1.0);
    value_map_put(map, key, &val);
    assert(VM_AS_NUMBER(*value_map_get(map, key)) == VM_AS_NUMBER(val)
           && "Stored value is not correct");

    // overwrite value
    val = VM_NUMBER_VAL(2.0);
    value_map_put(map, key, &val);
    assert(VM_AS_NUMBER(*value_map_get(map, key)) == VM_AS_NUMBER(val)
           && "Stored value is not correct");

    // remove value
    value_map_remove(map, key);
    assert(value_map_get(map, key) == NULL && "Removed item still present");

    // remove non-existent key
    value_map_remove(map, key);

    obj_string_delete(key);

    LOG_INFO("Testing VM hash map: SUCCESS");

    value_map_delete(map);
    return 0;
}


