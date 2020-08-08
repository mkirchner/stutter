#ifndef __CHUNK_H__
#define __CHUNK_H__

#include "vm/common.h"
#include "vm/bytecode.h"
#include "vm/value.h"
#include "vm/location.h"
#include "vm/bytecode_index.h"

typedef struct {
    BytecodeArray* code;
    BytecodeIndexArray* instructions;
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
void chunk_disassemble_instruction(const Chunk* chunk, const size_t n);

#endif /* !__CHUNK_H__ */
