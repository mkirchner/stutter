/*
 * prompt.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

// #define __STUTTER_VERSION__ "0.0.1-alpha"

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

Value* core_read_string(const Value* args);
Value* core_eval(const Value* str);

/* The global environment */
Environment* ENV;

Environment* global_env()
{
    Environment* env = env_new(NULL);

    env_set(env, "nil", CORE_NIL);
    env_set(env, "true", CORE_TRUE);
    env_set(env, "false", CORE_FALSE);

    env_set(env, "pr", value_new_builtin_fn(core_pr));
    env_set(env, "pr-str", value_new_builtin_fn(core_pr_str));
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

    env_set(env, "list", value_new_builtin_fn(core_list));
    env_set(env, "list?", value_new_builtin_fn(core_is_list));
    env_set(env, "empty?", value_new_builtin_fn(core_is_empty));
    env_set(env, "count", value_new_builtin_fn(core_count));

    env_set(env, "str", value_new_builtin_fn(core_str));
    env_set(env, "slurp", value_new_builtin_fn(core_slurp));
    env_set(env, "eval", value_new_builtin_fn(core_eval));
    env_set(env, "read-string", value_new_builtin_fn(core_read_string));

    env_set(env, "cons", value_new_builtin_fn(core_cons));
    env_set(env, "concat", value_new_builtin_fn(core_concat));

    // add stutter basics
    size_t N_EXPRS = 1;
    const char* exprs[N_EXPRS];
    exprs[0] = "(def load-file"
               "  (lambda (path)"
               "    (eval (read-string (str \"(do \" (slurp path) \")\")))))";
    for (size_t i = 0; i < N_EXPRS; ++i) {
        eval(core_read_string(value_make_list(value_new_string(exprs[i]))), env);
    }
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
    fclose(stream);

    // Condense the AST
    Value* ast2 = ir_from_ast(ast);
    ast_delete_sexpr(ast);
    return ast2;
}

Value* core_read_string(const Value* args)
{
    if (args->type == VALUE_LIST) {
        // FIXME: check argn == 1
        Value* str = list_head(LIST(args));
        return read_(str->value.str);
    }
    return NULL;
}


Value* core_eval(const Value* args)
{
    /* This assumes that everything is loaded in the global env.
     * Otherwise we should implement it as a special form.
     */
    if (args->type == VALUE_LIST) {
        return eval(list_head(LIST(args)), ENV);
    }
    return NULL;
}


int main(int argc, char* argv[])
{
    fprintf(stderr, "Stutter %s (clang %d.%d.%d on darwin)\n", __STUTTER_VERSION__,
            __clang_major__, __clang_minor__, __clang_patchlevel__);

    // set up garbage collection
    gc_start(&gc, &argc);
    // create env
    ENV = global_env();

    while(true) {
        char* input = readline("stutter> ");
        if (input == NULL) {
            break;
        }
        if (strcmp(input, "") == 0) {
            continue;
        }
        add_history(input);
        Value* expr = read_(input);
        Value* eval_result = eval(expr, ENV);
        core_prn(value_make_list(eval_result));
        // fprintf(stdout, "\n");
        free(input);
    }
    gc_stop(&gc);
    return 0;
}

