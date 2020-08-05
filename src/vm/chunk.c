#include "vm/chunk.h"

#include <stdio.h>
#include <stdarg.h>
#include "vm/mem.h"
#include "vm/instruction.h"

Chunk* chunk_new()
{
    Chunk* chunk = mem_reallocate(NULL, sizeof(Chunk));
    chunk->ins = instructions_new();
    chunk->locations = locations_new();
    chunk->constants = values_new();
    return chunk;
}

void chunk_delete(Chunk* chunk)
{
    mem_reallocate(chunk->constants, 0);
    mem_reallocate(chunk->locations, 0);
    mem_reallocate(chunk->ins, 0);
    mem_reallocate(chunk, 0);
}

size_t chunk_add_constant(Chunk* chunk, Value val)
{
    values_append(chunk->constants, val);
    return values_size(chunk->constants) - 1;
}

void chunk_add_instruction(Chunk *chunk, size_t row, size_t col,
        Bytecode opcode, Arity arity, ... /* operands */)
{
    Location l = { .row = row, .col = col };
    locations_append(chunk->locations, l);
    instructions_append(chunk->ins, opcode);
    va_list args;
    va_start(args, arity);
    for (Arity i = 0; i < arity; ++i) {
        instructions_append(chunk->ins, (Bytecode) va_arg(args, int));
    }
    va_end(args);
}

void chunk_disassemble(const Chunk* chunk, const char* title)
{
    printf("=== [%s] ===\n", title);
    size_t pos = 0;
    while(pos < chunk->ins->size) {
        pos = chunk_disassemble_at(chunk, pos);
    }
}

static size_t chunk_disassemble_single_byte(const char* mnemonic, const size_t pos)
{
    printf("%s\n", mnemonic);
    return pos + 1;
}

static size_t chunk_disassemble_constant(const char* mnemonic,
                                                const Chunk* chunk,
                                                const size_t pos)
{
    // LOAD_CONST <addr>
    // FIXME: error management?
    size_t index = chunk->ins->bytecode[pos + 1];
    printf("%-16s %4ld [", mnemonic, index);
    value_print(chunk->constants->values[index]);
    printf("]\n");
    return pos + 2;
}

size_t chunk_disassemble_at(const Chunk* chunk, const size_t pos)
{
    printf("%04lx ", pos);
    Bytecode opcode = chunk->ins->bytecode[pos];
    switch(opcode) {
        case OP_RETURN:
            return chunk_disassemble_single_byte("RETURN", pos);
        case OP_LOAD_CONST:
            return chunk_disassemble_constant("OP_LOAD_CONST", chunk, pos);
        default:
            LOG_WARNING("Unknown opcode: %u", opcode);
            // advance pos in the hope to hit something we understand
            return pos + 1;
    }
}



