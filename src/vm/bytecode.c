#include "vm/bytecode.h"

#include <stdio.h>
#include "vm/mem.h"


BytecodeArray* bytecode_new()
{
    BytecodeArray* ins = mem_reallocate(NULL, sizeof(BytecodeArray));
    ins->size = 0;
    ins->capacity = 0;
    ins->bytecode= NULL;
    return ins;
}

void bytecode_delete(BytecodeArray* ins)
{
    mem_reallocate(ins->bytecode, 0);
    mem_reallocate(ins, 0);
}

size_t bytecode_size(const BytecodeArray* bc)
{
    return bc->size;
}

static void bytecode_upsize(BytecodeArray* ins)
{
   // up-size in powers of 2
   size_t capacity = ins->capacity < 16 ? 16 : ins->capacity * 2;
   ins->bytecode= mem_reallocate(ins->bytecode, capacity * sizeof(Bytecode));
   ins->capacity = capacity;
}

void bytecode_append(BytecodeArray *ins, Bytecode byte)
{
    // Check if the array is large enough to hold the next byte,
    // up-size if required
    if (ins->capacity < ins->size + 1) {
        bytecode_upsize(ins);
    }
    // Store byte and advance
    ins->bytecode[ins->size] = byte;
    ins->size++;
}
