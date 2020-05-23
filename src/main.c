/*
 * prompt.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <editline/readline.h>

#include "ast.h"
#include "core.h"
#include "env.h"
#include "eval.h"
#include "exc.h"
#include "gc.h"
#include "ir.h"
#include "list.h"
#include "log.h"
#include "reader.h"
#include "value.h"

Value *core_read_string(const Value *args);
Value *core_eval(const Value *str);

/* The global environment */
Environment *ENV;

Environment *global_env()
{
    Environment *env = env_new(NULL);

    env_set(env, "nil", VALUE_CONST_NIL);
    env_set(env, "true", VALUE_CONST_TRUE);
    env_set(env, "false", VALUE_CONST_FALSE);
    env_set(env, "nil?", value_new_builtin_fn(core_is_nil));
    env_set(env, "true?", value_new_builtin_fn(core_is_true));
    env_set(env, "false?", value_new_builtin_fn(core_is_false));
    env_set(env, "symbol?", value_new_builtin_fn(core_is_symbol));

    env_set(env, "pr", value_new_builtin_fn(core_pr));
    env_set(env, "pr-str", value_new_builtin_fn(core_pr_str));
    env_set(env, "prn", value_new_builtin_fn(core_prn));

    Value *add = value_new_builtin_fn(core_add);
    env_set(env, "+", add);
    env_set(env, "add", add);
    Value *sub = value_new_builtin_fn(core_sub);
    env_set(env, "sub", sub);
    env_set(env, "-", sub);
    Value *mul = value_new_builtin_fn(core_mul);
    env_set(env, "*", mul);
    env_set(env, "mul", mul);
    Value *div = value_new_builtin_fn(core_div);
    env_set(env, "/", div);
    env_set(env, "div", div);

    Value *eq = value_new_builtin_fn(core_eq);
    env_set(env, "=", eq);
    env_set(env, "eq", eq);
    Value *lt = value_new_builtin_fn(core_lt);
    env_set(env, "<", lt);
    env_set(env, "lt", lt);
    Value *leq = value_new_builtin_fn(core_leq);
    env_set(env, "<=", leq);
    env_set(env, "leq", leq);
    Value *gt = value_new_builtin_fn(core_gt);
    env_set(env, ">", gt);
    env_set(env, "gt", gt);
    Value *geq = value_new_builtin_fn(core_geq);
    env_set(env, ">=", geq);
    env_set(env, "geq", geq);

    env_set(env, "list", value_new_builtin_fn(core_list));
    env_set(env, "list?", value_new_builtin_fn(core_is_list));
    env_set(env, "empty?", value_new_builtin_fn(core_is_empty));
    env_set(env, "count", value_new_builtin_fn(core_count));

    env_set(env, "symbol", value_new_builtin_fn(core_symbol));
    env_set(env, "str", value_new_builtin_fn(core_str));
    env_set(env, "slurp", value_new_builtin_fn(core_slurp));
    env_set(env, "eval", value_new_builtin_fn(core_eval));
    env_set(env, "read-string", value_new_builtin_fn(core_read_string));

    env_set(env, "cons", value_new_builtin_fn(core_cons));
    env_set(env, "concat", value_new_builtin_fn(core_concat));

    env_set(env, "map", value_new_builtin_fn(core_map));
    env_set(env, "apply", value_new_builtin_fn(core_apply));

    env_set(env, "assert", value_new_builtin_fn(core_assert));
    env_set(env, "throw", value_new_builtin_fn(core_throw));

    // add stutter basics
    size_t N_EXPRS = 1;
    const char *exprs[N_EXPRS];
    exprs[0] = "(def load-file"
               "  (lambda (path)"
               "    (eval (read-string (str \"(do \" (slurp path) \")\")))))";
    for (size_t i = 0; i < N_EXPRS; ++i) {
        eval(core_read_string(value_make_list(value_new_string(exprs[i]))), env);
    }
    return env;
}

Value *read_(char *input)
{
    // Get a handle on the input
    size_t n = strlen(input);
    FILE *stream = fmemopen(input, n, "r");
    if (!stream) {
        printf("%s\n", strerror(errno));
        return NULL;
    }

    // Create the initial AST
    Reader *reader = reader_new(stream);
    AstSexpr *ast = reader_read(reader);
    reader_delete(reader);
    fclose(stream);

    // Condense the AST
    Value *ast2 = ir_from_ast(ast);
    ast_delete_sexpr(ast);
    return ast2;
}

Value *core_read_string(const Value *args)
{
    if (is_list(args)) {
        Value *str = list_head(LIST(args));
        return read_(STRING(str));
    }
    return NULL;
}


Value *core_eval(const Value *args)
{
    /* This assumes that everything is loaded in the global env.
     * Otherwise we should implement it as a special form.
     */
    if (is_list(args)) {
        return eval(list_head(LIST(args)), ENV);
    }
    return NULL;
}

#define BOLD         "\033[1m"
#define NO_BOLD      "\033[22m"

const char* banner()
{
    const char* banner =
    "         __        __  __\n"
    "   _____/ /___  __/ /_/ /____  _____\n"
    "  / ___/ __/ / / / __/ __/ _ \\/ ___/\n"
    " (__  ) /_/ /_/ / /_/ /_/  __/ /\n"
    "/____/\\__/\\__,_/\\__/\\__/\\___/_/";
    return banner;
}

void show_help()
{
    char* help =
        " %s\n\n"
        BOLD "USAGE\n" NO_BOLD
        "  stutter [-h] [file]\n"
        "\n"
        BOLD "ARGUMENTS\n" NO_BOLD
        "  file      Execute FILE as a stutter program\n"
        "\n"
        BOLD "OPTIONS\n" NO_BOLD
        "  -h        Show this help text\n";
    fprintf(stderr, "%s", banner());
    fprintf(stderr, help, __STUTTER_VERSION__);
}

int main(int argc, char *argv[])
{
    // set up garbage collection, use extended setup for bigger mem limits
    gc_start_ext(&gc, &argc, 16384, 16384, 0.2, 0.8, 0.5);
    // create env and tell GC to never collect it
    ENV = global_env();
    gc_make_static(&gc, ENV);

    int c;
    while ((c = getopt(argc, argv, "h")) != -1) {
        switch(c) {
            case 'h':
            default:
                show_help();
                exit(0);
        }
    }
    if (argc > 1) {
        /* In order to execute a file, explicitly construct a load-file
         * call to avoid interpretation of the filename. */
        Value *src = value_make_list(value_new_symbol("load-file"));
        src = value_new_list(list_conj(LIST(src), value_new_string(argv[optind])));
        Value *eval_result = eval(src, ENV);
        if (eval_result) {
            core_prn(value_make_list(eval_result));
        } else {
            if (exc_is_pending()) {
                core_prn(exc_get());
                exc_clear();
            } else {
                LOG_CRITICAL("Eval returned NULL.");
            }
        }
        if (!eval_result) {
            return 1;
        } else {
            return 0;
        }
    }

    // REPL
    fprintf(stdout, "%s %s\n\n", banner(), __STUTTER_VERSION__);

    while(true) {
        // char *input = readline("stutter> ");
        char *input = readline("\U000003BB> ");
        if (input == NULL) {
            break;
        }
        if (strcmp(input, "") == 0) {
            continue;
        }
        add_history(input);
        Value *expr = read_(input);
        if (expr) {
            Value *eval_result = eval(expr, ENV);
            if (eval_result) {
                core_prn(value_make_list(eval_result));
            } else {
                if (exc_is_pending()) {
                    core_prn(exc_get());
                    exc_clear();
                } else {
                    LOG_CRITICAL("Eval returned NULL.");
                }
            }
        }
        free(input);
    }
    gc_stop(&gc);
    fprintf(stdout, "\n");
    return 0;
}

