#include <assert.h>

#include "vm.c" /* to get access to the static decls */

int main() {
    VM* vm = vm_new();

    LOG_INFO("Testing VM stack");
    VmValue val = VM_NUMBER_VAL(1.0);
    vm_stack_push(vm, val);
    assert(VM_AS_NUMBER(vm_stack_peek(vm)) == VM_AS_NUMBER(val)
            && "Peeked value is not correct");
    assert(VM_AS_NUMBER(vm_stack_pop(vm)) == VM_AS_NUMBER(val)
            && "Popped value is not correct");
    VmValue vals[] = {
        VM_NUMBER_VAL(1.),
        VM_NUMBER_VAL(2.),
        VM_NUMBER_VAL(3.),
        VM_NUMBER_VAL(4.),
        VM_NUMBER_VAL(5.)
    };
    for (size_t i = 0; i < 5; ++i) {
        vm_stack_push(vm, vals[i]);
    }
    for (int j = 4; j >= 0; --j) {
        assert(VM_AS_NUMBER(vm_stack_pop(vm)) == VM_AS_NUMBER(vals[j])
                && "push/pop mismatch");
    }
    LOG_INFO("Testing VM stack: SUCCESS");

    vm_delete(vm);
    return 0;
}


