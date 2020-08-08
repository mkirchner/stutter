#include "vm/bytecode_index.h"
#include "vm/mem.h"

#include <assert.h>

BytecodeIndexArray *bytecode_index_new()
{
    BytecodeIndexArray *ins = mem_reallocate(NULL, sizeof(BytecodeIndexArray));
    ins->size = 0;
    ins->capacity = 0;
    ins->ixs = NULL;
    return ins;
}


void bytecode_index_delete(BytecodeIndexArray *bia)
{
    mem_reallocate(bia->ixs, 0);
    mem_reallocate(bia, 0);
}

/* Size of the bytecode index, i.e. # of instructions */
size_t bytecode_index_size(BytecodeIndexArray *bia)
{
    return bia->size;
}

static void bytecode_index_upsize(BytecodeIndexArray *bia)
{
    // up-size in powers of 2
    size_t capacity = bia->capacity < 16 ? 16 : bia->capacity * 2;
    bia->ixs = mem_reallocate(bia->ixs, capacity * sizeof(BytecodeIndex));
    bia->capacity = capacity;
}

void bytecode_index_append(BytecodeIndexArray *bia, BytecodeIndex instruction_index)
{
    // Check if the array is large enough to hold the next index
    // up-size if required
    if (bia->capacity < bia->size + 1) {
        bytecode_index_upsize(bia);
    }
    bia->ixs[bia->size] = instruction_index;
    bia->size++;
}

/* Get the bytecode position for instruction at pos instruction_index */
BytecodeIndex bytecode_index_get_index(BytecodeIndexArray *bia, size_t instruction_index)
{
    return bia->ixs[instruction_index];
}

/* Get the instruction index for bytecode at pos bytecode_index */
size_t bytecode_index_get_instruction_index(BytecodeIndexArray *bia, BytecodeIndex bytecode_index)
{
    // O(log n) binary interval search with assumptions:
    // 1. Leftmost element in bia is always 0
    // 2. The rightmost interval is open
    size_t l = 0;
    size_t r = bia->size - 1;
    size_t m = 0;
    while (l <= r) {
        if ((r - l == 1
                && bia->ixs[l] <= bytecode_index
                && bia->ixs[r] > bytecode_index)) {
            return l;
        }
        m = (l + r) / 2;
        if (bia->ixs[m] <= bytecode_index) {
            l = l == m ? m + 1 : m; // special case for the rightmost interval
        } else if (bytecode_index < bia->ixs[m]) {
            r = m;
        }
    }
    // rightmost interval
    return m;
}

