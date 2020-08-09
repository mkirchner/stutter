#ifndef __BYTECODE_H__
#define __BYTECODE_H__

#include "vm/common.h"
#include "vm/value.h"

/*
 * It's cleaner not to use an enum here since enums are
 * supposed to be represented as ints in C99 and we'll end up with
 * all these implicit typecasts to uint8_t.
 */
#define OP_RETURN     0
#define OP_LOAD_CONST 1
#define OP_NEGATE     2

typedef uint8_t Bytecode;

typedef struct {
    size_t size;
    size_t capacity;
    Bytecode *bytecode;
} BytecodeArray;

BytecodeArray *bytecode_new();
size_t bytecode_size(const BytecodeArray *);
void bytecode_delete(BytecodeArray *);
void bytecode_append(BytecodeArray *, Bytecode);

#endif
