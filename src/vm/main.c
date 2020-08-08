#include "vm/common.h"
#include "vm/chunk.h"

int main(int argc, const char* argv[]) {
    Chunk* chunk = chunk_new();
    chunk_add_instruction(chunk, 0, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, 42.0));
    chunk_add_instruction(chunk, 1, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, 42.0));
    chunk_add_instruction(chunk, 2, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, 42.0));
    chunk_add_instruction(chunk, 1, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, 42.0));
    chunk_add_instruction(chunk, 2, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, 42.0));
    chunk_add_instruction(chunk, 1, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, 42.0));
    chunk_add_instruction(chunk, 2, 0,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, 42.0));
    chunk_add_instruction(chunk, 3, 0,
                          OP_RETURN, 0);
    chunk_disassemble(chunk, "test");
    return 0;
}
