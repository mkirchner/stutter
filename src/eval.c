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

static bool _is_list(const Value* value)
{
    return value->type == VALUE_LIST;
}

Value* eval(Value* expr, Environment* env)
{
    if (!expr) return NULL;
    if (_is_self_evaluating(expr)) {
        // atoms self-evaluate
        LOG_DEBUG("Atom: %d\n", expr->type);
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
            evaluated_head = eval(head, env);
            if (!evaluated_head) {
                // eval failed
                return NULL; // FIXME: mem managment
            }
            list_append(evaluated_list, evaluated_head, sizeof(Value));
            list = list_tail(list);
            // FIXME: we should delete head here
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
    if (!expr || !_is_list(expr)) {
        if (expr) {
            LOG_CRITICAL("Not a list: %d", expr->type);
        }
        return NULL;
    }
    // FIXME: hardcoded implementation of (sum ...)
    LOG_DEBUG("Summing list: %d\n", expr->type);
    float sum = 0.0;
    Value* head;
    List* list = list_tail(expr->value.list); // slice off the symbol
    LOG_DEBUG("Initial list size: %ld", list_size(list));
    while ((head = list_head(list)) != NULL) {
        if (head->type == VALUE_FLOAT) {
            sum += head->value.float_;
        } else if (head->type == VALUE_INT) {
            sum += (float) head->value.int_;
        } else {
            LOG_WARNING("Skipping non-number type: %d", head->type);
        }
        list = list_tail(list);
    }
    Value* ret = value_new_float(sum); // FIXME: who frees this?
    LOG_DEBUG("apply returning: %f\n", ret->value.float_);
    return ret;
}

