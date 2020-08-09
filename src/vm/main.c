#include "vm/common.h"
#include "vm/chunk.h"
#include "vm/vm.h"

int main(int argc, const char *argv[])
{
    Chunk *chunk = chunk_new();
    chunk_add_instruction(chunk, 0, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, VM_NUMBER_VAL(42.0)));
    chunk_add_instruction(chunk, 1, 1,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, VM_NUMBER_VAL(42.0)));
    chunk_add_instruction(chunk, 1, 0, OP_NEGATE, 0);
    chunk_add_instruction(chunk, 2, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, VM_NUMBER_VAL(42.0)));
    chunk_add_instruction(chunk, 3, 0,
                          OP_RETURN, 0);
    // chunk_disassemble(chunk, "test");

    // now run
    VM* vm = vm_new();
    vm_interpret(vm, chunk);
    vm_delete(vm);
    return 0;
}
