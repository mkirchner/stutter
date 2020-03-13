/*
 * apply.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "apply.h"

#include <string.h>

#include "stdbool.h"
#include "eval.h"
#include "list.h"
#include "log.h"


static bool is_builtin_fn(const Value *value)
{
    return value->type == VALUE_BUILTIN_FN;
}

static bool is_compound_fn(const Value *fn)
{
    return fn->type == VALUE_FN || fn->type == VALUE_MACRO_FN;
}

static Value *apply_builtin_fn(Value *fn, Value *args)
{
    if (fn && fn->type == VALUE_BUILTIN_FN && fn->value.builtin_fn) {
        return fn->value.builtin_fn(args);
    }
    LOG_CRITICAL("Could not apply builtin fn");
    return NULL;
}

static Value *apply_compound_fn(Value *fn, Value *args,
                                Value **tco_expr, Environment **tco_env)
{
    if (fn && is_compound_fn(fn) && fn->value.fn) {
        // args are fully evaluated, so bind them to the names in the fn def on
        // top of the closure of f
        const List *arg_names = fn->value.fn->args->value.list;
        const List *arg_values = args->value.list;
        // bind arguments
        Environment *env = env_new(fn->value.fn->env);
        Value *arg_name = list_head(arg_names);
        Value *arg_value = list_head(arg_values);
        bool more = false;
        while(arg_name) {
            if (!is_symbol(arg_name)) {
                return value_make_error("Parameter names must be symbols");
            }
            if (strcmp(SYMBOL(arg_name), "&") == 0) {
                more = true;
                break;
            }
            if (!arg_value) {
                break;
            }
            env_set(env, arg_name->value.str, arg_value);
            arg_names = list_tail(arg_names);
            arg_values = list_tail(arg_values);
            arg_name = list_head(arg_names);
            arg_value = list_head(arg_values);
        }
        if (more) {
            Value* rest_name = list_head(list_tail(arg_names));
            if (!rest_name) {
                return value_make_error("Variadic arg list requires a name");
            }
            Value* rest_value = value_new_list(arg_values);
            env_set(env, SYMBOL(rest_name), rest_value);
            arg_name = list_head(arg_names);
            arg_name = arg_value = NULL;
        }
        if (arg_name != arg_value) {
            return value_make_error("Invalid number of arguments for compound fn");
        }
        // eval via TCO: don't call eval here, return the pointers
        *tco_expr = fn->value.fn->body;
        *tco_env = env;
        return NULL;
    }
    LOG_CRITICAL("Could not apply compound fn");
    return NULL;
}

Value *apply(Value *fn, Value *args, Value **tco_expr, Environment **tco_env)
{
    if (!fn) return NULL;
    *tco_expr = NULL;
    *tco_env = NULL;
    if (is_builtin_fn(fn)) {
        return apply_builtin_fn(fn, args);
    } else if (is_compound_fn(fn)) {
        return apply_compound_fn(fn, args, tco_expr, tco_env);
    } else {
        return value_make_error("apply: not a function");
    }
}

