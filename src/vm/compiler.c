#include "vm/compiler.h"

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "reader.h"
#include "ast.h"
#include "value.h"
#include "ir.h"
#include "log.h"
#include "eval_utils.h"

CompilerError compile(char* input, Chunk* chunk)
{
    // Get a handle on the input
    size_t n = strlen(input);
    FILE *stream = fmemopen(input, n, "r");
    if (!stream) {
        LOG_CRITICAL("%s\n", strerror(errno));
        return SYSTEM_ERROR;
    }

    // Create the initial AST
    Reader *reader = reader_new(stream);
    AstSexpr *ast = reader_read(reader);
    reader_delete(reader);
    fclose(stream);
    if (!ast) {
        LOG_CRITICAL("Could not parse input into AST.");
        return PARSER_ERROR;
    }
    ast_print((AstNode*) ast);
    // Condense the AST
    Value *ast2 = ir_from_ast(ast);
    ast_delete_sexpr(ast);
    if (!ast2) {
        LOG_CRITICAL("Could not condense AST.");
        return PARSER_ERROR;
    }
    return compile_ast(ast2, chunk);
}

CompilerError compile_ast(Value *expr, Chunk *chunk)
{
    value_print(expr);
    printf("\n");
    // FIXME: constant ""program" to test end-to-end integration
    chunk_add_instruction(chunk, 1, 1,
                          OP_LOAD_CONST, 1,
                          chunk_add_constant(chunk, VM_NUMBER_VAL(42.0)));
    chunk_add_instruction(chunk, 1, 0, OP_NEGATE, 0);
    chunk_add_instruction(chunk, 0, 0,
                          OP_RETURN, 0);
    return OK;
    if (!expr) {
        return NOT_IMPLEMENTED;
    }
    if (is_self_evaluating(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_variable(expr)) {
        return NOT_IMPLEMENTED;
    }
    // expr = macroexpand(expr, env);
    // if (!is_list(expr)) goto tco;
    if (is_quoted(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_quasiquoted(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_assignment(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_macro_definition(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_definition(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_let(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_if(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_do(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_try(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_lambda(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_macro_expansion(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_application(expr)) {
        return NOT_IMPLEMENTED;
    }
    LOG_CRITICAL("Unknown expression: %d", expr->type);
    return COMPILER_ERROR;
}

