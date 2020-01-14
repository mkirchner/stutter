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
#include "core.h"


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

static bool is_quasiquoted(const Value* value)
{
    return is_list_that_starts_with(value, "quasiquote", 10);
}

static bool is_assignment(const Value* value)
{
    // (set! var value)
    return is_list_that_starts_with(value, "set!", 4);
}

static bool is_definition(const Value* value)
{
    // (define var value)
    return is_list_that_starts_with(value, "define", 6);
}

static bool is_let(const Value* value)
{
    // (let (n1 v1 n2 v2 ...) body)
    return is_list_that_starts_with(value, "let*", 4);
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
    // LOG_DEBUG("Symbol: %s\n", expr->value.str);
    Value* sym = NULL;
    if ((sym = env_get(env, expr->value.str)) == NULL) {
        LOG_CRITICAL("Unknown symbol: %s", expr->value.str);
        // FIXME: how to fail? Value type ERROR?
    }
    return sym;
}

static Value* eval_quote(Value* expr)
{
    // (quote expr)
    if (has_cardinality(expr, 2)) {
        return list_head(list_tail(expr->value.list));
    }
    LOG_CRITICAL("Invalid parameter to built-in quote");
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

static Value* eval_let(Value* expr, Environment* env, Value** tco_expr, Environment** tco_env)
{
    // (let (n1 v1 n2 v2 ...) (body))
    if (has_cardinality(expr, 3)) {
        Environment* inner = env_new(env);
        Value* assignments = list_head(list_tail(expr->value.list));  // pos 2
        if (assignments->type != VALUE_LIST || list_size(LIST(assignments)) % 2 != 0) {
            LOG_CRITICAL("Invalid assignment list in let");
            return NULL;
        }
        List* list = LIST(assignments);
        Value* name = list_head(list);
        Value* value = list_head(list_tail(list));
        while (name) {
            env_set(inner, name->value.str, eval(value, inner));
            list = list_tail(list_tail(list)); // +2
            name = list_head(list);
            value = name ? list_head(list_tail(list)) : NULL;
        }
        // TCO
        *tco_expr = list_head(list_tail(list_tail(expr->value.list)));  // pos 3
        *tco_env = inner;
    }
    return NULL;
}

static Value* eval_if(Value* expr, Environment* env, Value** tco_expr, Environment** tco_env)
{
    // (if predicate consequent alternative)
    if (has_cardinality(expr, 4)) {
        Value* predicate = eval(list_head(list_tail(expr->value.list)), env);
        if (is_true(predicate)) {
            *tco_expr = list_head(list_tail(list_tail(expr->value.list)));
        } else {
            *tco_expr = list_head(list_tail(list_tail(list_tail(expr->value.list))));
        }
        *tco_env = env;
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

static Value* eval_do(Value* expr, Environment* env, Value** tco_expr, Environment** tco_env)
{
    // (do sexpr sexpr ...)
    Value* head;
    List* list = list_tail(LIST(expr));
    while((head = list_head(list)) != NULL) {
        list = list_tail(list);
        if (list_size(list) == 0) {
            *tco_expr = head;
            *tco_env = env;
            return NULL;
        }
        eval(head, env);
    }
    return NULL;
}

static Value* _quasiquote(Value* arg)
{
    /*
     * The idea here is to recursively rewrite the syntax tree (the IR form).
     * Note that quasiquote, unquote and splice-unquote forms all take an expression
     * as their single argument (expressions are represented as atoms or lists in
     * IR). In addition, `splice-unquote` is only valid in a sequence context
     * and we expect its argument to return a sequence after evaluation.
     *
     * Hence,
     *
     * 1. If arg is not a list, we return `(quote arg)`
     * 2. If arg is a list and the list starts with the `unquote` symbol, we
     *    return `arg`
     * 3. If arg is a list and it's first item arg[0] is a list that starts with
     *    the `splice-unquote` symbol in arg[0][0], we return
     *    `(concat arg[0][1] (quasiquote (tail arg)))`
     * 4. If the arg is an ordinary list, we cons the quasiquoted first item with
     *    the quasiquotation of the rest: `(cons (quasiquote arg[0]) (quasiquote (tail arg)))`
     *
     * Step 3 basically replaces the `cons` with a `concat` in the right places.
     */

    /* If the argument is not a list then act like quote */
    if (!(is_list(arg) && list_size(LIST(arg)) > 0)) {
        Value* ret = value_make_list(value_new_symbol("quote"));
        LIST(ret) = list_append(LIST(ret), arg);
        return ret;
    }
    /* arg is a list, let's peek at the first item */
    Value* arg0 = list_head(LIST(arg));
    if (arg0->type == VALUE_SYMBOL && strncmp(arg0->value.str, "unquote", 7) == 0) {
        if (list_size(LIST(arg)) != 2) {
            LOG_CRITICAL("unquote takes a single parameter");
            return NULL;
        }
        Value* arg1 = list_head(list_tail(LIST(arg)));
        return arg1;
    } else if (arg0->type == VALUE_LIST) {
        /* arg is a list that starts with a list. Let's see if it starts with splice-unquote */
        Value* arg00 = list_head(LIST(arg0));
        if (arg00->type == VALUE_SYMBOL && strncmp(arg00->value.str, "splice-unquote", 14) == 0) {
            if (list_size(LIST(arg0)) != 2) {
                LOG_CRITICAL("splice-unquote takes a single parameter");
                return NULL;
            }
            Value* arg01 = list_head(list_tail(LIST(arg0)));
            Value* ast = value_make_list(value_new_symbol("concat"));
            LIST(ast) = list_append(LIST(ast), arg01);
            LIST(ast) = list_append(LIST(ast), _quasiquote(value_new_list(list_tail(LIST(arg)))));
            return ast;
        }
    }
    Value* ast = value_make_list(value_new_symbol("cons"));
    LIST(ast) = list_append(LIST(ast), _quasiquote(arg0));
    LIST(ast) = list_append(LIST(ast), _quasiquote(value_new_list(list_tail(LIST(arg)))));
    return ast;
}

static Value* eval_quasiquote(Value* expr, Environment* env,
                              Value** tco_expr, Environment** tco_env)
{
    /* (quasiquote expr) */
    if (!(is_list(expr) && list_size(LIST(expr)) == 2)) {
        LOG_CRITICAL("quasiquote requires a single list as parameter");
        return NULL;
    }
    Value* args = list_head(list_tail(LIST(expr)));
    *tco_expr = _quasiquote(args);
    *tco_env = env;
    return NULL;
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
    Value* tco_expr = NULL;
    Value* ret = NULL;
    Environment* tco_env = NULL;
tco:
    if (!expr) {
        LOG_CRITICAL("Passed NULL ptr to eval");
        return NULL;
    }
    if (is_self_evaluating(expr)) {
        return expr;
    } else if (is_variable(expr)) {
        ret = lookup_variable_value(expr, env);
        return ret;
    } else if (is_quoted(expr)) {
        return eval_quote(expr);
    } else if (is_quasiquoted(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value* result = eval_quasiquote(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        return result;
    } else if (is_assignment(expr)) {
        return eval_assignment(expr, env);
    } else if (is_definition(expr)) {
        return eval_definition(expr, env);
    } else if (is_let(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value* result = eval_let(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        return result;
    } else if (is_if(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value* result = eval_if(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        return result;
    } else if (is_do(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value* result = eval_do(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        return result;
    } else if (is_lambda(expr)) {
        return declare_fn(expr, env);
    } else if (is_application(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        ret = apply(eval(operator(expr), env), eval_all(operands(expr), env),
                    &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        return ret;
    } else {
        LOG_CRITICAL("Unknown expression: %d", expr->type);
    }
    return NULL;
}
