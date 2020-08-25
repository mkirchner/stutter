#include "eval.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "apply.h"
#include "list.h"
#include "log.h"
#include "core.h"
#include "exc.h"

static bool is_self_evaluating(const Value *value)
{
    return value->type == VALUE_FLOAT
           || value->type == VALUE_INT
           || value->type == VALUE_STRING
           || value->type == VALUE_NIL
           || value->type == VALUE_FN;
}

static bool is_variable(const Value *value)
{
    return is_symbol(value);
}

static bool is_list_that_starts_with(const Value *value, const char *what, size_t len)
{
    if (value && is_list(value)) {
        Value *symbol;
        if ((symbol = list_head(LIST(value))) && is_symbol(symbol) &&
                strncmp(SYMBOL(symbol), what, len) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_quoted(const Value *value)
{
    return is_list_that_starts_with(value, "quote", 5);
}

static bool is_quasiquoted(const Value *value)
{
    return is_list_that_starts_with(value, "quasiquote", 10);
}

static bool is_assignment(const Value *value)
{
    // (set! var value)
    return is_list_that_starts_with(value, "set!", 4);
}

static bool is_definition(const Value *value)
{
    // (define var value)
    return is_list_that_starts_with(value, "def", 3);
}

static bool is_macro_definition(const Value *value)
{
    // (define var value)
    return is_list_that_starts_with(value, "defmacro", 8);
}

static bool is_let(const Value *value)
{
    // (let (n1 v1 n2 v2 ...) body)
    return is_list_that_starts_with(value, "let", 4);
}

static bool is_lambda(const Value *value)
{
    // (lambda (p1 ... pn) body)
    return is_list_that_starts_with(value, "lambda", 6);
}

static bool is_if(const Value *value)
{
    return is_list_that_starts_with(value, "if", 2);
}

static bool is_do(const Value *value)
{
    return is_list_that_starts_with(value, "do", 2);
}

static bool is_try(const Value *value)
{
    return is_list_that_starts_with(value, "try", 3);
}

static Value *get_macro_fn(const Value *form, Environment *env)
{
    /*
     * Takes a list, extracts the first element, checks if it is
     * a symbol and if that symbol resolves into a macro function.
     */
    assert(form && env);
    if (is_list(form)) {
        Value *first = list_head(LIST(form));
        if (first && is_symbol(first)) {
            Value *fn = env_get(env, SYMBOL(first));
            if (fn && is_macro(fn))
                return fn;
        }
    }
    return NULL;
}

static bool is_macro_expansion(const Value *value)
{
    return is_list_that_starts_with(value, "macroexpand", 11);
}


static bool is_application(const Value *value)
{
    return is_list(value);
}

static bool has_cardinality(const Value *expr, const size_t cardinality)
{
    return expr && is_list(expr) && list_size(LIST(expr)) == cardinality;
}

static Value *lookup_variable_value(Value *expr, Environment *env)
{
    Value *sym = NULL;
    if ((sym = env_get(env, SYMBOL(expr))) == NULL) {
        exc_set(value_make_exception(expr, "Error at %lu:%lu Undefined symbol: %s"));
        return NULL;
    }
    return sym;
}

static Value *eval_quote(Value *expr)
{
    // (quote expr)
    if (expr && has_cardinality(expr, 2)) {
        return list_nth(LIST(expr), 1);
    }
    exc_set(value_make_exception(expr, "Invalid parameter to built-in quote"));
    return NULL;
}

static Value *eval_assignment(Value *expr, Environment *env)
{
    // (set! var value)
    if (has_cardinality(expr, 3)) {
        Value *name = list_nth(LIST(expr), 1);
        if (env_contains(env, SYMBOL(name))) {
            Value *value = list_nth(LIST(expr), 2);
            value = eval(value, env);
            if (!value) {
                assert(exc_is_pending());
                return NULL;
            }
            env_set(env, SYMBOL(name), value);
            return value;
        }
        exc_set(value_make_exception(expr, "Could not find symbol %s.", SYMBOL(name)));
        return NULL;
    }
    exc_set(value_make_exception(expr, "set! requires 2 args"));
    return NULL;
}

static Value *eval_definition(Value *expr, Environment *env)
{
    // (def name value)
    assert(expr);
    if (has_cardinality(expr, 3)) {
        Value *name = list_nth(LIST(expr), 1);
        Value *value = list_nth(LIST(expr), 2);
        value = eval(value, env);
        if (!value) {
            assert(exc_is_pending());
            return NULL;
        }
        env_set(env, SYMBOL(name), value);
        return value;
    }
    exc_set(value_make_exception(expr, "def requires 2 args"));
    return NULL;
}

static Value *eval_macro_definition(Value *expr, Environment *env)
{
    // (defmacro name parameters expr)
    if (has_cardinality(expr, 4)) {
        Value *name = list_nth(LIST(expr), 1);
        Value *args = list_nth(LIST(expr), 2);
        Value *body = list_nth(LIST(expr), 3);
        Value *macro = value_new_macro(args, body, env);
        env_set(env, SYMBOL(name), macro);
        return macro;
    }
    exc_set(value_make_exception(expr, "Invalid macro declaration"));
    return NULL;
}

static Value *eval_let(Value *expr, Environment *env, Value **tco_expr, Environment **tco_env)
{
    // (let (n1 v1 n2 v2 ...) (body))
    if (has_cardinality(expr, 3)) {
        Environment *inner = env_new(env);
        Value *assignments = list_nth(LIST(expr), 1);
        if (!is_list(assignments) || list_size(LIST(assignments)) % 2 != 0) {
            exc_set(value_make_exception(expr, "Invalid assignment list in let"));
            return NULL;
        }
        const List *list = LIST(assignments);
        Value *name = list_head(list);
        Value *value = list_head(list_tail(list));
        Value *evaluated_value;
        while (name) {
            evaluated_value = eval(value, inner);
            if (!evaluated_value) {
                assert(exc_is_pending());
                return NULL;
            }
            env_set(inner, SYMBOL(name), evaluated_value);
            list = list_tail(list_tail(list)); // +2
            name = list_head(list);
            value = name ? list_head(list_tail(list)) : NULL;
        }
        // TCO
        *tco_expr = list_nth(LIST(expr), 2);
        *tco_env = inner;
        return NULL; // tco must return NULL
    }
    exc_set(value_make_exception(expr, "Invalid let declaration, require 2 args"));
    return NULL;
}

static Value *eval_if(Value *expr, Environment *env, Value **tco_expr, Environment **tco_env)
{
    // (if predicate consequent alternative)
    if (has_cardinality(expr, 4)) {
        Value *predicate = eval(list_nth(LIST(expr), 1), env);
        if (!predicate) {
            assert(exc_is_pending());
            return NULL;
        }
        if (is_truthy(predicate)) {
            *tco_expr = list_nth(LIST(expr), 2);
        } else {
            *tco_expr = list_nth(LIST(expr), 3);
        }
        *tco_env = env;
        return NULL; // tco must return NULL
    }
    exc_set(value_make_exception(expr, "Invalid if declaration, require 3 args"));
    return NULL;
}

static Value *eval_try(Value *expr, Environment *env)
{
    // (try sexpr (catch ex sexpr))
    if (has_cardinality(expr, 3)) {
        Value *catch_form = list_nth(LIST(expr), 2);
        if (!has_cardinality(catch_form, 3)) {
            exc_set(value_make_exception(expr, "Invalid catch declaration, require 2 arguments"));
            return NULL;
        }
        Value *result = eval(list_nth(LIST(expr), 1), env);
        if (!result) {
            assert(exc_is_pending());
            // LOG_CRITICAL("Caught exception: %s", EXCEPTION(exc_get()));
            Environment *ex_env = env_new(env);
            Value *name = list_nth(LIST(catch_form), 1);
            env_set(ex_env, STRING(name), exc_get());
            exc_clear();
            result = eval(list_nth(LIST(catch_form), 2), ex_env);
            if (!result) {
                // catch threw an exception
                assert(exc_is_pending());
                return NULL;
            }
        }
        return result;
    }
    exc_set(value_make_exception(expr, "Invalid try declaration, require 2 arguments"));
    return NULL;
}

static Value *declare_fn(Value *expr, Environment *env)
{
    // (lambda (p1 p2 ..) (expr))
    if (has_cardinality(expr, 3)) {
        Value *args = list_nth(LIST(expr), 1);
        Value *body = list_nth(LIST(expr), 2);
        Value *fn = value_new_fn(args, body, env);
        return fn;
    }
    exc_set(value_make_exception(expr, "Invalid lambda declaration, require 2 arguments"));
    return NULL;
}

static Value *eval_do(Value *expr, Environment *env, Value **tco_expr, Environment **tco_env)
{
    // (do sexpr sexpr ...)
    Value *head;
    const List *list = list_tail(LIST(expr));
    while((head = list_head(list)) != NULL) {
        list = list_tail(list);
        if (list_size(list) == 0) {
            *tco_expr = head;
            *tco_env = env;
            return NULL;
        }
        Value *result = eval(head, env);
        if (!result) {
            assert(exc_is_pending());
            return NULL;
        }
    }
    assert(0); // unreachable
    return NULL;
}

static Value *_quasiquote(Value *arg)
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

    /* require a valid pointer */
    if (!arg) return NULL;

    /* If the argument is not a list then act like quote */
    if (!(is_list(arg) && list_size(LIST(arg)) > 0)) {
        Value *ret = value_make_list(value_new_symbol("quote"));
        LIST(ret) = list_conj(LIST(ret), arg);
        return ret;
    }
    /* arg is a list, let's peek at the first item */
    Value *arg0 = list_head(LIST(arg));
    if (arg0->type == VALUE_SYMBOL && strncmp(STRING(arg0), "unquote", 7) == 0) {
        if (list_size(LIST(arg)) != 2) {
            exc_set(value_make_exception(
                        arg0, "Invalid unquote declaration, require 1 argument"));
            return NULL;
        }
        Value *arg1 = list_nth(LIST(arg), 1);
        return arg1;
    } else if (is_list(arg0)) {
        /* arg is a list that starts with a list. Let's see if it starts with splice-unquote */
        Value *arg00 = list_head(LIST(arg0));
        if (is_symbol(arg00) && strncmp(SYMBOL(arg00), "splice-unquote", 14) == 0) {
            if (list_size(LIST(arg0)) != 2) {
                exc_set(value_make_exception(arg0, "splice-unquote takes a single parameter"));
                return NULL;
            }
            Value *arg01 = list_nth(LIST(arg0), 1);
            Value *ast = value_make_list(value_new_symbol("concat"));
            LIST(ast) = list_conj(LIST(ast), arg01);
            LIST(ast) = list_conj(LIST(ast), _quasiquote(value_new_list(list_tail(LIST(arg)))));
            return ast;
        }
    }
    Value *ast = value_make_list(value_new_symbol("cons"));
    LIST(ast) = list_conj(LIST(ast), _quasiquote(arg0));
    LIST(ast) = list_conj(LIST(ast), _quasiquote(value_new_list(list_tail(LIST(arg)))));
    return ast;
}

static Value *eval_quasiquote(Value *expr, Environment *env,
                              Value **tco_expr, Environment **tco_env)
{
    /* (quasiquote expr) */
    if (!(is_list(expr) && list_size(LIST(expr)) == 2)) {
        exc_set(value_make_exception(expr, "quasiquote requires a single list as parameter"));
        return NULL;
    }
    Value *args = list_nth(LIST(expr), 1);
    *tco_expr = _quasiquote(args);
    *tco_env = env;
    return NULL;
}

static Value *operator(Value *expr)
{
    Value *op = NULL;
    if (expr && is_list(expr)) {
        op = list_head(LIST(expr));
        if (!op) {
            exc_set(value_make_exception(expr, "Could not find operator in list"));
            return NULL;
        }
    }
    return op;
}

static Value *operands(Value *expr)
{
    Value *ops = NULL;
    if (expr && is_list(expr)) {
        ops = value_new_list(list_tail(LIST(expr)));
    }
    return ops;
}

static Value *macroexpand(Value *form, Environment *env)
{
    assert(form && env);
    Value *fn;
    Value *args;
    Value *expr = form;
    Environment *new_env = env;
    while(expr && (fn = get_macro_fn(expr, new_env)) != NULL) {
        args = value_new_list(list_tail(LIST(expr)));
        apply(fn, args, &expr, &new_env);
        expr = eval(expr, new_env);
        if (!expr) {
            assert(exc_is_pending());
            return NULL;
        }
    }
    return expr;
}

static Value *macroexpand_1(Value *expr, Environment *env)
{
    if (!is_list(expr)) { // FIXME: this is checking the outer list
        exc_set(value_make_exception(expr, "Require macro call for expansion"));
        return NULL;
    }
    Value *args = list_head(list_tail(LIST(expr)));
    return macroexpand(args, env);
}

static Value *eval_all(Value *expr, Environment *env)
{
    // eval every element of a list
    const List *list = LIST(expr);
    const List *evaluated_list = list_new();
    Value *head;
    Value *evaluated_head;
    while ((head = list_head(list)) != NULL) {
        evaluated_head = eval(head, env);
        if (!evaluated_head) {
            assert(exc_is_pending());
            return NULL;
        }
        evaluated_list = list_conj(evaluated_list, evaluated_head);
        list = list_tail(list);
    }
    LIST(expr) = evaluated_list;
    return value_new_list(evaluated_list);
}


Value *eval(Value *expr, Environment *env)
{
    Value *tco_expr = NULL;
    Value *ret = NULL;
    Environment *tco_env = NULL;
tco:
    if (!expr) {
        assert(exc_is_pending());
        return NULL;
    }
    if (is_self_evaluating(expr)) {
        return expr;
    } else if (is_variable(expr)) {
        ret = lookup_variable_value(expr, env);
        return ret;
    }
    expr = macroexpand(expr, env);
    if (!expr) {
        LOG_CRITICAL("Macro expansion failed.");
        assert(exc_is_pending());
        return expr;
    }
    if (!is_list(expr)) goto tco;
    if (is_quoted(expr)) {
        return eval_quote(expr);
    } else if (is_quasiquoted(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value *result = eval_quasiquote(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        if (!result) {
            assert(exc_is_pending());
            return NULL;
        }
        return result;
    } else if (is_assignment(expr)) {
        return eval_assignment(expr, env);
    } else if (is_macro_definition(expr)) {
        return eval_macro_definition(expr, env);
    } else if (is_definition(expr)) {
        return eval_definition(expr, env);
    } else if (is_let(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value *result = eval_let(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        if (!result) {
            assert(exc_is_pending());
            return NULL;
        }
        return result;
    } else if (is_if(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value *result = eval_if(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        if (!result) {
            assert(exc_is_pending());
            return NULL;
        }
        return result;
    } else if (is_do(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value *result = eval_do(expr, env, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        if (!result) {
            assert(exc_is_pending());
            return NULL;
        }
        return result;
    } else if (is_try(expr)) {
        return eval_try(expr, env);
    } else if (is_lambda(expr)) {
        return declare_fn(expr, env);
    } else if (is_macro_expansion(expr)) {
        return macroexpand_1(expr, env);
    } else if (is_application(expr)) {
        tco_expr = NULL;
        tco_env = NULL;
        Value *fn = eval(operator(expr), env);
        if (!fn) {
            assert(exc_is_pending());
            return NULL;
        }
        Value *args = eval_all(operands(expr), env);
        if (!args) {
            assert(exc_is_pending());
            return NULL;
        }
        ret = apply(fn, args, &tco_expr, &tco_env);
        if (tco_expr && tco_env) {
            expr = tco_expr;
            env = tco_env;
            goto tco;
        }
        if (!ret) {
            assert(exc_is_pending());
            return NULL;
        }
        return ret;
    }
    LOG_CRITICAL("Unknown expression: %d", expr->type);
    exc_set(value_new_exception("Unknown expression"));
    return NULL;
}
