/*
 * prompt.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#define __STUTTER_VERSION__ "0.0.1-alpha"

int main(int argc, char* argv[])
{
    printf("Stutter version %s\n\n", __STUTTER_VERSION__);

    while(1) {
        char* input = readline("stutter> ");
        if (input == NULL) {
            return 0;
        }
        add_history(input);
        printf("You said: %s\n", input);
        free(input);
    }
    return 0;
}

