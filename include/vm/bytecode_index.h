#ifndef __BYTECODE_INDEX_H__
#define __BYTECODE_INDEX_H__

#include "vm/common.h"

// Use actual indexes instead of pointers to avoid issues w/ realloc
// shifting around the base address of the bytecode array.
typedef size_t BytecodeIndex;

typedef struct {
    size_t size;
    size_t capacity;
    BytecodeIndex* ixs;
} BytecodeIndexArray;

BytecodeIndexArray* bytecode_index_new();
void bytecode_index_delete(BytecodeIndexArray* bia);

/* Size of the bytecode index, i.e. # of instructions */
size_t bytecode_index_size(BytecodeIndexArray* bia);
/* Append an instruction position to the index */
void bytecode_index_append(BytecodeIndexArray* bia, BytecodeIndex instruction_index);
/* Get the bytecode position for instruction at pos instruction_index */
BytecodeIndex bytecode_index_get_index(BytecodeIndexArray* bia, size_t instruction_index);
/* Get the instruction index for bytecode at pos bytecode_index */
size_t bytecode_index_get_instruction_index(BytecodeIndexArray* bia, BytecodeIndex bytecode_index);

#endif /* !__BYTECODE_INDEX_H__ */
