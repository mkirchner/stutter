#ifndef __VM_H__
#define __VM_H__

#include "vm/common.h"
#include "vm/chunk.h"
#include "vm/bytecode.h"
#include "vm/value.h"

#define VM_STACK_SIZE 512  // FIXME: pull from env or make dynamic

typedef enum {
    VM_OK,
    VM_ERROR
} VMError;


typedef struct {
    const Chunk *chunk;
    const Bytecode *ip;
    VmValue stack[VM_STACK_SIZE];
    VmValue *tos;  // top of stack
} VM;

VM *vm_new();
void vm_delete(VM *vm);

VMError vm_interpret(VM *vm, const Chunk *chunk);

#endif /* !__VM_H__ */
