#include "vm/chunk.h"

#include <stdio.h>
#include <stdarg.h>
#include "vm/mem.h"

Chunk *chunk_new()
{
    Chunk *chunk = mem_reallocate(NULL, sizeof(Chunk));
    chunk->code = bytecode_new();
    chunk->instructions = bytecode_index_new();
    chunk->locations = locations_new();
    chunk->constants = values_new();
    return chunk;
}

void chunk_delete(Chunk *chunk)
{
    mem_reallocate(chunk->constants, 0);
    mem_reallocate(chunk->locations, 0);
    mem_reallocate(chunk->instructions, 0);
    mem_reallocate(chunk, 0);
}

size_t chunk_add_constant(Chunk *chunk, VmValue val)
{
    values_append(chunk->constants, val);
    return values_size(chunk->constants) - 1;
}

void chunk_add_instruction(Chunk *chunk, size_t row, size_t col,
                           Bytecode opcode, Arity arity, ... /* operands */)
{
    Location l = { .row = row, .col = col };
    locations_append(chunk->locations, l);
    bytecode_index_append(chunk->instructions, bytecode_size(chunk->code));
    bytecode_append(chunk->code, opcode);
    va_list args;
    va_start(args, arity);
    for (Arity i = 0; i < arity; ++i) {
        bytecode_append(chunk->code, (Bytecode) va_arg(args, int));
    }
    va_end(args);
}

void chunk_disassemble(const Chunk *chunk, const char *title)
{
    printf("; === %s ===\n", title);
    size_t n = bytecode_index_size(chunk->instructions);
    for (size_t i = 0; i < n; ++i) {
        chunk_disassemble_instruction(chunk, i);
    }
}

static void chunk_disassemble_single_byte(const char *mnemonic)
{
    printf("%s\n", mnemonic);
}

static void chunk_disassemble_constant(const char *mnemonic,
                                       const Chunk *chunk,
                                       const size_t pos)
{
    // LOAD_CONST <addr>
    // FIXME: error management?
    size_t index = chunk->code->bytecode[pos + 1];
    printf("%-16s %4ld [", mnemonic, index);
    value_print(chunk->constants->values[index]);
    printf("]\n");
}

void chunk_disassemble_instruction(const Chunk *chunk, const size_t n)
{
    // code location of the instruction
    const Location *cur_loc = locations_at(chunk->locations, n);
    // position of the instruction in the bytecode
    size_t byte_pos = bytecode_index_get_index(chunk->instructions, n);
    printf("%04lX ", byte_pos);
    if (n > 0) {
        const Location *prev_loc = locations_at(chunk->locations, n - 1);
        if (prev_loc && cur_loc->col == prev_loc->col && cur_loc->row == prev_loc->row) {
            printf("           ");
        } else {
            printf("%4lu, %4lu ", cur_loc->row, cur_loc->col);
        }
    } else {
        printf("%4lu, %4lu ", cur_loc->row, cur_loc->col);
    }
    Bytecode opcode = chunk->code->bytecode[byte_pos]; // FIXME: wrap access in fn
    switch(opcode) {
    case OP_RETURN:
        chunk_disassemble_single_byte("RETURN");
        break;
    case OP_LOAD_CONST:
        chunk_disassemble_constant("OP_LOAD_CONST", chunk, byte_pos);
        break;
    case OP_NEGATE:
        chunk_disassemble_single_byte("NEGATE");
        break;
    default:
        LOG_WARNING("Unknown opcode: %u", opcode);
    }
}

