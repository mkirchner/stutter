#ifndef __CHUNK_H__
#define __CHUNK_H__

#include "vm/common.h"
#include "vm/instruction.h"
#include "vm/value.h"
#include "vm/location.h"

typedef struct {
    InstructionArray* ins;
    LocationArray *locations;
    ValueArray *constants;
} Chunk;

typedef int Arity;

Chunk* chunk_new();
void chunk_delete(Chunk* ptr);

size_t chunk_add_constant(Chunk*, Value);
void chunk_add_instruction(Chunk *chunk, size_t row, size_t col,
        Bytecode opcode, Arity arity, ... /* operands */);

void chunk_disassemble(const Chunk* ins, const char* title);
size_t chunk_disassemble_at(const Chunk* ins, const size_t pos);

#endif /* !__CHUNK_H__ */
