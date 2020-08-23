#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "common.h"
#include "vm/chunk.h"
#include "ast.h"
#include "../value.h"

typedef enum {
    OK,
    SYSTEM_ERROR,
    PARSER_ERROR,
    COMPILER_ERROR,
    NOT_IMPLEMENTED
} CompilerError;

CompilerError compile(char* input, Chunk* chunk);
CompilerError compile_ast(Value* ast, Chunk* chunk);

#endif /* !__COMPILER_H__ */
