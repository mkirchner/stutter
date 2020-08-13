#include <assert.h>

#include "value_map.c" /* to get access to the static decls */

int main() {
    LOG_INFO("Testing VM value map");
    ValueMap* map = value_map_new();
    
    // store/retrieve single value
    VmValue val = VM_NUMBER_VAL(1.0);
    VmString *key = obj_string_new(3, "key");
    value_map_insert(map, key, &val);
    obj_string_delete(key);
    assert(VM_AS_NUMBER(*value_map_get(map, key)) == VM_AS_NUMBER(val)
            && "Stored value is not correct");

    /*
    // multiple
    VmValue vals[] = {
        VM_NUMBER_VAL(1.),
        VM_NUMBER_VAL(2.),
        VM_NUMBER_VAL(3.),
        VM_NUMBER_VAL(4.),
        VM_NUMBER_VAL(5.)
    };
    for (size_t i = 0; i < 5; ++i) {
        key = 
        value_map_insert(map, key, &val);
        vm_stack_push(vm, vals[i]);
    }
    for (int j = 4; j >= 0; --j) {
        assert(VM_AS_NUMBER(vm_stack_pop(vm)) == VM_AS_NUMBER(vals[j])
                && "push/pop mismatch");
    }
    */
    LOG_INFO("Testing VM stack: SUCCESS");

    value_map_delete(map);
    return 0;
}


