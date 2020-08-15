#include "eval_utils.h"

#include <string.h>

bool is_self_evaluating(const Value *value)
{
    return value->type == VALUE_FLOAT
           || value->type == VALUE_INT
           || value->type == VALUE_STRING
           || value->type == VALUE_NIL
           || value->type == VALUE_FN;
}

bool is_variable(const Value *value)
{
    return is_symbol(value);
}

bool is_list_that_starts_with(const Value *value, const char *what, size_t len)
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

bool is_quoted(const Value *value)
{
    return is_list_that_starts_with(value, "quote", 5);
}

bool is_quasiquoted(const Value *value)
{
    return is_list_that_starts_with(value, "quasiquote", 10);
}

bool is_assignment(const Value *value)
{
    // (set! var value)
    return is_list_that_starts_with(value, "set!", 4);
}

bool is_definition(const Value *value)
{
    // (define var value)
    return is_list_that_starts_with(value, "def", 3);
}

bool is_macro_definition(const Value *value)
{
    // (define var value)
    return is_list_that_starts_with(value, "defmacro", 8);
}

bool is_let(const Value *value)
{
    // (let (n1 v1 n2 v2 ...) body)
    return is_list_that_starts_with(value, "let", 4);
}

bool is_lambda(const Value *value)
{
    // (lambda (p1 ... pn) body)
    return is_list_that_starts_with(value, "lambda", 6);
}

bool is_if(const Value *value)
{
    return is_list_that_starts_with(value, "if", 2);
}

bool is_do(const Value *value)
{
    return is_list_that_starts_with(value, "do", 2);
}

bool is_try(const Value *value)
{
    return is_list_that_starts_with(value, "try", 3);
}

bool is_macro_expansion(const Value *value)
{
    return is_list_that_starts_with(value, "macroexpand", 11);
}


bool is_application(const Value *value)
{
    return is_list(value);
}

bool has_cardinality(const Value *expr, const size_t cardinality)
{
    return expr && is_list(expr) && list_size(LIST(expr)) == cardinality;
}


