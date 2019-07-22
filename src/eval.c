/*
 * eval.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "eval.h"

#include "stdbool.h"
#include "list.h"
#include "log.h"

static bool _is_self_evaluating(const Value* value)
{
    return value->type == VALUE_FLOAT
        || value->type == VALUE_INT
        || value->type == VALUE_STRING
        || value->type == VALUE_NIL
        || value->type == VALUE_FN;
}

static bool _is_symbol(const Value* value)
{
    return value->type == VALUE_SYMBOL;
}

static bool _is_fn(const Value* value)
{
    return value->type == VALUE_FN;
}

static bool _is_list(const Value* value)
{
    return value->type == VALUE_LIST;
}

Value* eval(Value* expr, Environment* env)
{
    if (!expr) return NULL;
    if (_is_self_evaluating(expr) || _is_fn(expr)) {
        // atoms and built-ins self-evaluate
        LOG_DEBUG("Atom/Builtin: %d\n", expr->type);
        return expr;
    } else if (_is_symbol(expr)) {
        LOG_DEBUG("Symbol: %s\n", expr->value.str);
        // resolve symbols or fail
        Value* sym;
        if ((sym = env_get(env, expr->value.str)) == NULL) {
            LOG_CRITICAL("Unknown symbol: %s", expr->value.str);
        }
        return sym;
    } else if (_is_list(expr)) {
        LOG_DEBUG("List: %d\n", expr->type);
        // eval every element of a list
        List* list = expr->value.list;
        List* evaluated_list = list_new();
        Value* head;
        Value* evaluated_head;
        while ((head = list_head(list)) != NULL) {
            // printf("Evaluating%s ", ":"); value_print(head); printf("\n");
            evaluated_head = eval(head, env);
            // printf("Returns%s ", ":"); value_print(evaluated_head); printf("\n");
            if (!evaluated_head) {
                // eval failed
                LOG_DEBUG("Eval %s", "failed");
                return NULL; // FIXME: mem managment
            }
            list_append(evaluated_list, evaluated_head, sizeof(Value));
            list = list_tail(list);
            // FIXME: we should delete head here
            // value_print(evaluated_head); printf("\n");
        }
        expr->value.list = evaluated_list; // FIXME: and delete the old list here
        // ok, all elements have been evaluated, so let's apply
        return apply(expr, env);
    } else {
        LOG_CRITICAL("Unknown expression: %d", expr->type);
    }
    return NULL;
}

Value* apply(Value* expr, Environment* env)
{
    // we expect a list with (fn arg1 arg2 ...)
    if (!expr || !_is_list(expr)) {
        if (expr) {
            LOG_CRITICAL("Not a list: %d", expr->type);
        }
        return NULL;
    }
    // value_print(expr); printf("\n");
    Value* fn = list_head(expr->value.list);
    if (fn->type != VALUE_FN) {
        LOG_CRITICAL("Cannot apply non-function value.%s", "");
        return NULL;
    }
    // value_print(fn); printf("\n");
    Value* args = value_new_list();
    args->value.list = list_tail(expr->value.list); // FIXME: mem management
    return fn->value.fn(args);
}

