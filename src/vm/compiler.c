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
#include "vm/object.h"

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
    if (compile_ast(ast2, chunk) == OK) {
        chunk_add_instruction(chunk, 0, 0,
                              OP_RETURN, 0);
        return OK;
    }
    return COMPILER_ERROR;
}

static Bytecode make_constant(Chunk *chunk, VmValue value)
{
    size_t index = chunk_add_constant(chunk, value);
    if (index > BYTECODE_MAX) {
        // FIXME: this is bad error handling
        LOG_CRITICAL("Too many constants in chunk. Dropping.");
        return 0;
    }
    return index;
}

static CompilerError compile_self_evaluating(Value *expr, Chunk *chunk)
{
    /*
     * All constants are loaded by putting them into the chunk's
     * values array and then issuing the LOAD instruction that indexes
     * the correct array field.
     *
     * This is slower for small ints and single chars but allows us to
     * treat all values uniformly.
     */
    VmValue v = VM_NIL_VAL;
    switch(expr->type) {
    case VALUE_NIL: {
        // FIXME: the parser is not correctly reporting nil right now
        v = VM_NIL_VAL;
        break;
    }
    case VALUE_FLOAT: {
        v = VM_NUMBER_VAL(FLOAT(expr));
        break;
    }
    case VALUE_INT: {
        v = VM_NUMBER_VAL(INT(expr));
        break;
    }
    case VALUE_STRING:
    case VALUE_SYMBOL: {
        char* str = STRING(expr);
        v = VM_OBJ_VAL(obj_string_new(strlen(str), str));
        break;
    }
    case VALUE_BOOL: {
        // FIXME: the parser is not correctly reporting bools right now
        v = VM_BOOL_VAL(BOOL(expr));
        break;
    }
    default:
        return COMPILER_ERROR;
    }

    chunk_add_instruction(
            chunk,
            expr->loc.row, expr->loc.col,
            OP_LOAD_CONST, 1,
            chunk_add_constant(chunk, v));
    return OK;
}

static CompilerError compile_application(Value *expr, Chunk *chunk)
{
    Value *fn = operator(expr);
    Value *args = operands(expr);

    /*
     * evaluate the operands, push on stack
     */
    const List *list = LIST(args);
    size_t nargs = list_size(list); // FIXME: do we need this?
    Value *head;
    while ((head = list_head(list)) != NULL) {
        CompilerError err = compile_ast(head, chunk);
        if (err != OK) {
            return err;
        }
        list = list_tail(list);
    }

    /*
     * evaluate the operator and push to TOS
     */
    CompilerError err = compile_ast(fn, chunk);
    if (err != OK) {
        return err;
    }

    /*
     * emit bytcode to call the operator
     */
    chunk_add_instruction(chunk, expr->loc.row, expr->loc.col, OP_CALL, 0);
    return OK;

    /*
    // hard-code fn to ADD2
    for (size_t i = 1; i < nargs; ++i) {
        chunk_add_instruction(chunk, expr->loc.row, expr->loc.col, OP_ADD2, 0);
    }
    return OK;
    */
}

CompilerError compile_ast(Value *expr, Chunk *chunk)
{
    // value_print(expr);
    // printf("\n");
    if (!expr) {
        return OK;
    }
    if (is_self_evaluating(expr)) {
        return compile_self_evaluating(expr, chunk);
    } else if (is_variable(expr)) {
        return NOT_IMPLEMENTED;
    } else if (is_quoted(expr)) {
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
        return compile_application(expr, chunk);
    }
    LOG_CRITICAL("Unknown expression: %d", expr->type);
    return COMPILER_ERROR;
}

