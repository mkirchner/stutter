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
#include "core.h"
#include "env.h"
#include "eval.h"
#include "gc.h"
#include "ir.h"
#include "list.h"
#include "log.h"
#include "reader.h"
#include "value.h"

Environment* global_env()
{
    Environment* env = env_new(NULL);

    env_set(env, "nil", CORE_NIL);
    env_set(env, "true", CORE_TRUE);
    env_set(env, "false", CORE_FALSE);

    env_set(env, "prn", value_new_builtin_fn(core_prn));

    Value* plus = value_new_builtin_fn(core_plus);
    env_set(env, "+", plus);
    env_set(env, "plus", plus);
    Value* minus = value_new_builtin_fn(core_minus);
    // FIXME: no "-", need to extend lexer w/ negative numbers and unary minus
    env_set(env, "minus", minus);
    Value* mul = value_new_builtin_fn(core_mul);
    env_set(env, "*", mul);
    env_set(env, "mul", mul);
    Value* div = value_new_builtin_fn(core_div);
    env_set(env, "/", div);
    env_set(env, "div", div);

    Value* eq = value_new_builtin_fn(core_eq);
    env_set(env, "=", eq);
    env_set(env, "eq", eq);
    /*
    Value* lt = value_new_builtin_fn(core_lt);
    env_set(env, "<", lt);
    env_set(env, "lt", lt);
    Value* leq = value_new_builtin_fn(core_leq);
    env_set(env, "<=", leq);
    env_set(env, "leq", leq);
    Value* gt = value_new_builtin_fn(core_gt);
    env_set(env, ">", gt);
    env_set(env, "gt", gt);
    Value* geq = value_new_builtin_fn(core_geq);
    env_set(env, ">=", geq);
    env_set(env, "geq", geq);
    */

    env_set(env, "list", value_new_builtin_fn(core_list));
    return env;
}

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

    // Condense the AST
    Value* ast2 = ir_from_ast(ast);
    ast_delete_sexpr(ast);
    return ast2;
}

int main(int argc, char* argv[])
{
    fprintf(stderr, "Stutter version %s\n\n", __STUTTER_VERSION__);

    // set up garbage collection
    gc_start(&gc, &argc);
    // create env
    Environment* env = global_env();

    while(true) {
        char* input = readline("stutter> ");
        if (input == NULL) {
            break;
        }
        add_history(input);
        Value* expr = read_(input);
        Value* eval_result = eval(expr, env);
        value_print(eval_result);
        fprintf(stdout, "\n");
        free(input);
    }
    gc_stop(&gc);
    return 0;
}

