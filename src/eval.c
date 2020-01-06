/*
 * eval.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "eval.h"

#include <string.h>
#include <stdbool.h>
#include "apply.h"
#include "list.h"
#include "log.h"


static bool is_self_evaluating(const Value* value)
{
    return value->type == VALUE_FLOAT
        || value->type == VALUE_INT
        || value->type == VALUE_STRING
        || value->type == VALUE_NIL
        || value->type == VALUE_FN;
}

static bool is_symbol(const Value* value)
{
    return value->type == VALUE_SYMBOL;
}

static bool is_variable(const Value* value)
{
    return is_symbol(value);
}

static bool is_list_that_starts_with(const Value* value, const char* what, size_t len)
{
    if (value && value->type == VALUE_LIST) {
        Value* symbol;
        if ((symbol = list_head(value->value.list)) &&
             symbol->type == VALUE_SYMBOL &&
             strncmp(symbol->value.str, what, len) == 0) {
                return true;
        }
    }
    return false;
}

static bool is_quoted(const Value* value)
{
    return is_list_that_starts_with(value, "quote", 5);
}

static bool is_assignment(const Value* value)
{
    // (set! var value)
    return is_list_that_starts_with(value, "set!", 4);
}

static bool is_definition(const Value* value)
{
    // (define var value)
    // skipping (define (var p1 p2 ... pn) body) for now
    // since we can do that w/ (define var (lambda (p1 ... pn) body))
    return is_list_that_starts_with(value, "define", 6);
}

static bool is_lambda(const Value* value)
{
    // (lambda (p1 ... pn) body)
    return is_list_that_starts_with(value, "lambda", 6);
}

static bool is_if(const Value* value)
{
    return is_list_that_starts_with(value, "if", 2);
}

static bool is_do(const Value* value)
{
    return is_list_that_starts_with(value, "do", 2);
}

static bool is_list(const Value* value)
{
    return value->type == VALUE_LIST;
}

static bool is_application(const Value* value)
{
    return is_list(value);
}

static bool is_true(const Value* v)
{
    if (!v) return false;
    switch(v->type) {
    case VALUE_NIL:
        return false;
    case VALUE_BOOL:
        return v->value.bool_ == true;
    case VALUE_INT:
        return v->value.int_ != 0;
    case VALUE_FLOAT:
        return v->value.float_ != 0.0f;
    case VALUE_STRING:
    case VALUE_SYMBOL:
        return strncmp(v->value.str, "", 1) != 0;
    case VALUE_LIST:
        return list_size(v->value.list) > 0;
    case VALUE_FN:
        return v->value.fn != NULL;
    case VALUE_BUILTIN_FN:
        return v->value.builtin_fn != NULL;
    }
}

static bool has_cardinality(const Value* expr, const size_t cardinality)
{
    return expr && expr->type == VALUE_LIST && list_size(expr->value.list) == cardinality;
}

static Value* lookup_variable_value(Value* expr, Environment* env)
{
    LOG_DEBUG("Symbol: %s\n", expr->value.str);
    Value* sym = NULL;
    if ((sym = env_get(env, expr->value.str)) == NULL) {
        LOG_CRITICAL("Unknown symbol: %s", expr->value.str);
        // FIXME: how to fail? Value type ERROR?
    }
    return sym;
}

static Value* unquote(Value* expr)
{
    // (quote expr)
    if (has_cardinality(expr, 2)) {
        return list_head(list_tail(expr->value.list));
    }
    LOG_CRITICAL("Invalid parameter to built-in unquote");
    return NULL;
}

static Value* eval_assignment(Value* expr, Environment* env)
{
    // (set! var value)
    if (has_cardinality(expr, 3)) {
        Value* name = list_head(list_tail(expr->value.list));  // pos 2
        if (env_contains(env, name->value.str)) {
            Value* value = list_head(list_tail(list_tail(expr->value.list)));  // pos 3
            value = eval(value, env);
            env_set(env, name->value.str, value);
            return value;
        } else {
            LOG_CRITICAL("Could not find symbol %s.", name->value.str);
        }
    }
    return NULL;
}
static Value* eval_definition(Value* expr, Environment* env)
{
    // (def name value)
    if (has_cardinality(expr, 3)) {
        Value* name = list_head(list_tail(expr->value.list));  // pos 2
        Value* value = list_head(list_tail(list_tail(expr->value.list)));  // pos 3
        value = eval(value, env);
        env_set(env, name->value.str, value);
    }
    return NULL;
}

static Value* eval_if(Value* expr, Environment* env)
{
    // (if predicate consequent alternative)
    if (has_cardinality(expr, 4)) {
        Value* predicate = eval(list_head(list_tail(expr->value.list)), env);
        if (is_true(predicate)) {
            Value* consequent = eval(list_head(list_tail(list_tail(expr->value.list))), env);
            return consequent;
        } else {
            Value* alternative = eval(list_head(list_tail(list_tail(list_tail(expr->value.list)))), env);
            return alternative;
        }
    }
    return NULL;
}

static Value* declare_fn(Value* expr, Environment* env)
{
    // (lambda (p1 p2 ..) (expr))
    if (has_cardinality(expr, 3)) {
        Value* args = list_head(list_tail(expr->value.list));
        Value* body = list_head(list_tail(list_tail(expr->value.list)));
        Value* fn = value_new_fn(args, body, env);
        return fn;
    }
    LOG_CRITICAL("Invalid lambda declaration");
    return NULL;
}

static Value* eval_do(Value* expr, Environment* env)
{
    Value* head;
    Value* ret = NULL;
    List* list = LIST(expr);
    while((head = list_head(list)) != NULL) {
        ret = eval(head, env);
        list = list_tail(list);
    }
    return ret;
}

static Value* operator(Value* expr)
{
    Value* op = NULL;
    if (expr && expr->type == VALUE_LIST) {
        op = list_head(expr->value.list);
        if (!op) {
            LOG_CRITICAL("Could not find operator in list");
        }
    }
    return op;
}

static Value* operands(Value* expr)
{
    Value* ops = NULL;
    if (expr && expr->type == VALUE_LIST) {
        ops = value_new_list(list_tail(expr->value.list));
    }
    return ops;
}

static Value* eval_all(Value* expr, Environment* env)
{
    // eval every element of a list
    List* list = expr->value.list;
    List* evaluated_list = list_new();
    Value* head;
    Value* evaluated_head;
    while ((head = list_head(list)) != NULL) {
        evaluated_head = eval(head, env);
        if (!evaluated_head) {
            // eval failed
            LOG_CRITICAL("Eval failed.");
            return NULL;
        }
        evaluated_list = list_append(evaluated_list, evaluated_head);
        list = list_tail(list);
    }
    expr->value.list = evaluated_list;
    return expr;
}

Value* eval(Value* expr, Environment* env)
{
    if (!expr) {
        LOG_CRITICAL("Passed NULL ptr to eval");
        return NULL;
    }
    if (is_self_evaluating(expr)) {
        LOG_DEBUG("is_self_evaluating");
        return expr;
    } else if (is_variable(expr)) {
        LOG_DEBUG("is_variable");
        return lookup_variable_value(expr, env);
    } else if (is_quoted(expr)) {
        LOG_DEBUG("is_quoted");
        return unquote(expr);
    } else if (is_assignment(expr)) {
        LOG_DEBUG("is_assignment");
        return eval_assignment(expr, env);
    } else if (is_definition(expr)) {
        LOG_DEBUG("is_definition");
        return eval_definition(expr, env);
    } else if (is_if(expr)) {
        LOG_DEBUG("is_if");
        return eval_if(expr, env);
    } else if (is_do(expr)) {
        LOG_DEBUG("is_do");
        return eval_do(expr, env);
    } else if (is_lambda(expr)) {
        LOG_DEBUG("is_lambda");
        return declare_fn(expr, env);
    } else if (is_application(expr)) {
        return apply(eval(operator(expr), env), eval_all(operands(expr), env));
    } else {
        LOG_CRITICAL("Unknown expression: %d", expr->type);
    }
    return NULL;
}
