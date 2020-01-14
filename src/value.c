/*
 * value.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "value.h"
#include <string.h>
#include "log.h"
#include "gc.h"


static Value* value_new(ValueType type)
{
    Value* v = (Value*) gc_malloc(&gc, sizeof(Value));
    v->type = type;
    return v;
}

Value* value_new_nil()
{
    Value* v = value_new(VALUE_NIL);
    return v;
}

Value* value_new_bool(bool bool_)
{
    Value* v = value_new(VALUE_BOOL);
    v->value.bool_ = bool_;
    return v;
}

Value* value_new_int(int int_)
{
    Value* v = value_new(VALUE_INT);
    v->value.int_ = int_;
    return v;
}

Value* value_new_float(float float_)
{
    Value* v = value_new(VALUE_FLOAT);
    v->value.float_ = float_;
    return v;
}

Value* value_new_builtin_fn(Value* (fn)(const Value*))
{
    Value* v = value_new(VALUE_BUILTIN_FN);
    v->value.builtin_fn = fn;
    return v;
}

Value* value_new_fn(Value* args, Value* body, Environment* env)
{
    Value* v = value_new(VALUE_FN);
    v->value.fn = gc_calloc(&gc, 1, sizeof(CompositeFunction));
    v->value.fn->args = args;
    v->value.fn->body = body;
    v->value.fn->env = env;
    return v;
}

Value* value_new_string(const char* str)
{
    Value* v = value_new(VALUE_STRING);
    v->value.str = gc_strdup(&gc, str);
    return v;
}

Value* value_new_symbol(const char* str)
{
    Value* v = value_new(VALUE_SYMBOL);
    v->value.str = gc_strdup(&gc, str);
    return v;
}

Value* value_new_list(List* l)
{
    Value* v = value_new(VALUE_LIST);
    if (l) {
        v->value.list = list_copy(l);
    } else {
        v->value.list = list_new();
    }
    return v;
}

Value* value_make_list(Value* v)
{
    Value* r = value_new_list(NULL);
    LIST(r) = list_append(LIST(r), v);
    return r;
}

void value_print(const Value* v)
{
    if (!v) return;
    switch(v->type) {
    case VALUE_NIL:
        fprintf(stderr, "NIL");
        break;
    case VALUE_BOOL:
        fprintf(stderr, "%s", v->value.bool_ ? "true" : "false");
        break;
    case VALUE_INT:
        fprintf(stderr, "%d", v->value.int_);
        break;
    case VALUE_FLOAT:
        fprintf(stderr, "%f", v->value.float_);
        break;
    case VALUE_STRING:
    case VALUE_SYMBOL:
        fprintf(stderr, "%s", v->value.str);
        break;
    case VALUE_LIST:
        fprintf(stderr, "( ");
        Value* head;
        List* tail = v->value.list;
        while((head = list_head(tail)) != NULL) {
            value_print(head);
            fprintf(stderr, " ");
            tail = list_tail(tail);
        }
        fprintf(stderr, ")");
        break;
    case VALUE_FN:
        fprintf(stderr, "lambda: ");
        value_print(FN(v)->args);
        value_print(FN(v)->body);
        break;
    case VALUE_BUILTIN_FN:
        fprintf(stderr, "#<@%p>", (void*) v->value.builtin_fn);
        break;
    }

}
