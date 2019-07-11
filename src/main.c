/*
 * prompt.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#define __STUTTER_VERSION__ "0.0.1-alpha"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <editline/readline.h>

#include "ast.h"
#include "env.h"
#include "ir.h"
#include "list.h"
#include "log.h"
#include "reader.h"
#include "value.h"

Value* read_(char* input) {
    // Get a handle on the input
    size_t n = strlen(input);
    FILE* stream = fmemopen(input, n, "r");
    if (!stream) {
        printf("%s\n", strerror(errno));
        return NULL;
    }

    // Create the initial AST
    Reader* reader = reader_new(stream);
    AstSexpr* ast = reader_read(reader);
    reader_delete(reader);
    // ast_print(ast);

    // Condense the AST
    Value* ast2 = ir_from_ast(ast);
    ast_delete_sexpr(ast);
    return ast2;
}

Value* eval(Value* ast, Environment* env) {
    return ast;
}


int main(int argc, char* argv[])
{
    printf("Stutter version %s\n\n", __STUTTER_VERSION__);

    Environment* env = env_new(NULL);
    while(1) {
        char* input = readline("stutter> ");
        if (input == NULL) {
            break;
        }
        add_history(input);
        Value* eval_result = eval(read_(input), env);
        value_print(eval_result);
        printf("\n");
        value_delete(eval_result);
        free(input);
    }
    env_delete(env);
    return 0;
}

