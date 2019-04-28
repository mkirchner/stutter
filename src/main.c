/*
 * prompt.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#define __STUTTER_VERSION__ "0.0.1-alpha"

#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

char* read(char* input) {
    return input;
}

char* eval(char* ast, void* env) {
    return ast;
}


int main(int argc, char* argv[])
{
    printf("Stutter version %s\n\n", __STUTTER_VERSION__);

    while(1) {
        char* input = readline("stutter> ");
        if (input == NULL) {
            return 0;
        }
        add_history(input);
        printf("%s\n", eval(read(input), NULL));
        free(input);
    }
    return 0;
}

