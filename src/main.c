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
#include "reader.h"

ast_sexpr_t* read_(char* input) {
    size_t n = strlen(input);
    FILE* stream = fmemopen(input, n, "r");
    if (!stream) {
        printf("%s\n", strerror(errno));
        return NULL;
    }
    reader_t* reader = reader_new(stream);
    ast_sexpr_t* ast = reader_read(reader);
    reader_delete(reader);
    return ast;
}

char* eval(ast_sexpr_t* ast, void* env) {
    if (ast) ast_print(ast);
    ast_delete_sexpr(ast);
    return "";
}


int main(int argc, char* argv[])
{
    printf("Stutter version %s\n\n", __STUTTER_VERSION__);

    while(1) {
        char* input = readline("stutter> ");
        if (input == NULL) {
            break;
        }
        add_history(input);
        printf("%s\n", eval(read_(input), NULL));
        free(input);
    }
    return 0;
}

