/*
 * apply.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "apply.h"

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
        List *arg_names = fn->value.fn->args->value.list;
        List *arg_values = args->value.list;
        if (list_size(arg_names) != list_size(arg_values)) {
            LOG_CRITICAL("Invalid number of arguments for compound fn");
            return NULL;
        }
        // create new env on top of closure
        Environment *env = env_new(fn->value.fn->env);
        Value *arg_name = list_head(arg_names);
        Value *arg_value = list_head(arg_values);
        while(arg_name != NULL && arg_value != NULL) {
            // FIXME: this *assumes* that all entries in the arg_names list are symbols
            env_set(env, arg_name->value.str, arg_value);

            arg_names = list_tail(arg_names);
            arg_values = list_tail(arg_values);
            arg_name = list_head(arg_names);
            arg_value = list_head(arg_values);
        }
        // eval
        *tco_expr = fn->value.fn->body;
        *tco_env = env;
        return NULL;
    }
    LOG_CRITICAL("Could not apply compound fn");
    return NULL;
}

Value *apply(Value *fn, Value *args, Value **tco_expr, Environment **tco_env)
{
    *tco_expr = NULL;
    *tco_env = NULL;
    if (is_builtin_fn(fn)) {
        return apply_builtin_fn(fn, args);
    } else if (is_compound_fn(fn)) {
        return apply_compound_fn(fn, args, tco_expr, tco_env);
    } else {
        LOG_CRITICAL("Unknown function type in apply: %u.", fn->type);
    }
    return NULL;
}

