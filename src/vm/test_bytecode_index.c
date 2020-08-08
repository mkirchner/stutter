#include "vm/bytecode_index.h"
#include "log.h"
#include <assert.h>

int main(void)
{
    BytecodeIndex indexes[8] = {0, 1, 4, 6, 7, 10, 11, 14};
    BytecodeIndexArray *bia = bytecode_index_new();
    for (int i = 0; i < 8; ++i) {
        bytecode_index_append(bia, indexes[i]);
    }

    // find existing
    LOG_INFO("Starting tests...");
    assert(bytecode_index_get_instruction_index(bia, 0) == 0 && "0 failed");
    assert(bytecode_index_get_instruction_index(bia, 1) == 1 && "1 failed");
    assert(bytecode_index_get_instruction_index(bia, 4) == 2 && "4 failed");
    assert(bytecode_index_get_instruction_index(bia, 6) == 3 && "6 failed");
    assert(bytecode_index_get_instruction_index(bia, 7) == 4 && "7 failed");
    assert(bytecode_index_get_instruction_index(bia, 10) == 5 && "10 failed");
    assert(bytecode_index_get_instruction_index(bia, 11) == 6 && "11 failed");
    assert(bytecode_index_get_instruction_index(bia, 14) == 7 && "14 failed");

    assert(bytecode_index_get_instruction_index(bia, 2) == 1 && "2 failed");
    assert(bytecode_index_get_instruction_index(bia, 5) == 2 && "5 failed");
    assert(bytecode_index_get_instruction_index(bia, 9) == 4 && "9 failed");
    assert(bytecode_index_get_instruction_index(bia, 16) == 7 && "16 failed");

    LOG_INFO("Done. Success.");
    bytecode_index_delete(bia);
    return 0;
}


