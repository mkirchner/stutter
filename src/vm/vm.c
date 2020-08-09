#include "vm/vm.h"
#include "vm/mem.h"
#include "vm/bytecode_index.h"

#define READ_BYTE(vm) (*vm->ip++)
#define READ_CONSTANT(vm) (vm->chunk->constants->values[READ_BYTE(vm)])

VM *vm_new()
{
    VM *vm = mem_reallocate(NULL, sizeof(VM));
    vm->tos = vm->stack;
    return vm;
}

void vm_delete(VM *vm)
{
    mem_reallocate(vm, 0);
}

static void vm_stack_push(VM *vm, Value value)
{
    *vm->tos = value;
    vm->tos++;
}

static Value vm_stack_pop(VM *vm)
{
    vm->tos--;
    return *vm->tos;
}

static Value vm_stack_peek(VM *vm)
{
    return *(vm->tos - 1);
}

static void vm_stack_print(VM *vm)
{
    printf("STCK [");
    for (Value *addr = vm->stack; addr < vm->tos; ++addr) {
        printf("[");
        value_print(*addr);
        printf("]");
    }
    printf("]\n");
}

static VMError vm_run(VM *vm);

VMError vm_interpret(VM *vm, const Chunk *chunk)
{
    vm->chunk = chunk;
    vm->ip = chunk->code->bytecode;
    return vm_run(vm);
}

static VMError vm_run(VM *vm)
{
    Bytecode op;
    while(1) {
        op = READ_BYTE(vm);
        // debugging support; remove later, this is O(log n)
        vm_stack_print(vm);
        chunk_disassemble_instruction(
            vm->chunk,
            bytecode_index_get_instruction_index(
                vm->chunk->instructions,
                vm->ip - vm->chunk->code->bytecode - 1));
        switch(op) {
        case OP_RETURN:
            return VM_OK;
        case OP_LOAD_CONST: {
            Value val = READ_CONSTANT(vm);
            vm_stack_push(vm, val);
            break;
        }
        case OP_NEGATE:
            vm_stack_push(vm, VM_NUMBER_VAL(-VM_AS_NUMBER(vm_stack_pop(vm))));
            break;
        default:
            LOG_WARNING("Unknown bytecode instruction: %u", op);
            break;
        }
    }
}
