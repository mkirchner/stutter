#include "vm/instruction.h"

#include <stdio.h>
#include "vm/mem.h"


InstructionArray* instructions_new()
{
    InstructionArray* ins = mem_reallocate(NULL, sizeof(InstructionArray));
    ins->size = 0;
    ins->capacity = 0;
    ins->bytecode= NULL;
    return ins;
}

void instructions_delete(InstructionArray* ins)
{
    mem_reallocate(ins->bytecode, 0);
    mem_reallocate(ins, 0);
}

static void instructions_upsize(InstructionArray* ins)
{
   // up-size in powers of 2
   size_t capacity = ins->capacity < 16 ? 16 : ins->capacity * 2;
   ins->bytecode= mem_reallocate(ins->bytecode, capacity * sizeof(Bytecode));
   ins->capacity = capacity;
}

void instructions_append(InstructionArray *ins, Bytecode byte)
{
    // Check if the array is large enough to hold the next byte,
    // up-size if required
    if (ins->capacity < ins->size + 1) {
        instructions_upsize(ins);
    }
    // Store byte and advance
    ins->bytecode[ins->size] = byte;
    ins->size++;
}
