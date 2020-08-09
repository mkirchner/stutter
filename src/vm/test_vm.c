#include <assert.h>

#include "vm.c" /* to get access to the static decls */

int main() {
    VM* vm = vm_new();

    LOG_INFO("Testing VM stack");
    Value val = 42.0;
    vm_stack_push(vm, val);
    assert(vm_stack_peek(vm) == val && "Peeked value is not correct");
    assert(vm_stack_pop(vm) == val && "Popped value is not correct");
    Value vals[] = {1., 2., 3., 4., 5.};
    for (size_t i = 0; i < 5; ++i) {
        vm_stack_push(vm, vals[i]);
    }
    for (int j = 4; j >= 0; --j) {
        assert(vm_stack_pop(vm) == vals[j] && "push/pop mismatch");
    }
    LOG_INFO("Testing VM stack: SUCCESS");

    vm_delete(vm);
    return 0;
}


