#include "value.h"
#include <string.h>
#include "log.h"
#include <assert.h>
#include <stdarg.h>


const char *value_type_names[] = {
    "VALUE_BOOL",
    "VALUE_BUILTIN_FN",
    "VALUE_EXCEPTION",
    "VALUE_FLOAT",
    "VALUE_FN",
    "VALUE_INT",
    "VALUE_LIST",
    "VALUE_MACRO_FN",
    "VALUE_NIL",
    "VALUE_STRING",
    "VALUE_SYMBOL",
    "VALUE_VECTOR"
};


Value *VALUE_CONST_TRUE = &((Value)
{
    .type = VALUE_BOOL, .value = { .bool_ = true }
});
Value *VALUE_CONST_FALSE = &((Value)
{
    .type = VALUE_BOOL, .value = { .bool_ = false }
});
Value *VALUE_CONST_NIL = &((Value)
{
    .type = VALUE_NIL, .value = { .float_ = 0.0 }
});

bool is_exception(const Value *value)
{
    return value->type == VALUE_EXCEPTION;
}

bool is_symbol(const Value *value)
{
    return value->type == VALUE_SYMBOL;
}

bool is_macro(const Value *value)
{
    return value->type == VALUE_MACRO_FN;
}

bool is_list(const Value *value)
{
    return value->type == VALUE_LIST;
}

bool is_vector(const Value *value)
{
    return value->type == VALUE_VECTOR;
}

static Value *value_new(ValueType type)
{
    Value *v = (Value *) gc_malloc(&gc, sizeof(Value));
    v->type = type;
    return v;
}

Value *value_new_nil()
{
    Value *v = value_new(VALUE_NIL);
    return v;
}

Value *value_new_bool(bool bool_)
{
    Value *v = value_new(VALUE_BOOL);
    v->value.bool_ = bool_;
    return v;
}

Value *value_new_int(int int_)
{
    Value *v = value_new(VALUE_INT);
    v->value.int_ = int_;
    return v;
}

Value *value_new_float(float float_)
{
    Value *v = value_new(VALUE_FLOAT);
    v->value.float_ = float_;
    return v;
}

Value *value_new_builtin_fn(Value * (fn)(const Value *))
{
    Value *v = value_new(VALUE_BUILTIN_FN);
    v->value.builtin_fn = fn;
    return v;
}

Value *value_new_fn(Value *args, Value *body, Environment *env)
{
    Value *v = value_new(VALUE_FN);
    v->value.fn = gc_calloc(&gc, 1, sizeof(CompositeFunction));
    v->value.fn->args = args;
    v->value.fn->body = body;
    v->value.fn->env = env;
    return v;
}

Value *value_new_macro(Value *args, Value *body, Environment *env)
{
    Value *v = value_new(VALUE_MACRO_FN);
    v->value.fn = gc_calloc(&gc, 1, sizeof(CompositeFunction));
    v->value.fn->args = args;
    v->value.fn->body = body;
    v->value.fn->env = env;
    return v;
}

Value *value_new_string(const char *str)
{
    Value *v = value_new(VALUE_STRING);
    v->value.str = gc_strdup(&gc, str);
    return v;
}

Value *value_new_exception(const char *str)
{
    Value *v = value_new(VALUE_EXCEPTION);
    v->value.str = gc_strdup(&gc, str);
    return v;
}

Value *value_make_exception(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *message = NULL;
    vasprintf(&message, fmt, args);
    va_end(args);
    Value *ex = value_new_exception(message);
    free(message);
    return ex;
}

Value *value_new_symbol(const char *str)
{
    Value *v = value_new(VALUE_SYMBOL);
    v->value.str = gc_strdup(&gc, str);
    return v;
}

Value *value_new_list(const List *l)
{
    Value *v = value_new(VALUE_LIST);
    if (l) {
        v->value.list = list_dup(l);
    } else {
        v->value.list = list_new();
    }
    return v;
}

Value *value_make_list(Value *v)
{
    Value *r = value_new_list(NULL);
    LIST(r) = list_append(LIST(r), v);
    return r;
}

Value *value_new_vector(const List *l)
{
    Value *v = value_new(VALUE_VECTOR);
    VECTOR(v) = vector_new(sizeof(Value));
    if (l) {
        for (const ListItem *i = l->head; i != NULL; i = i->next) {
            vector_push_back(VECTOR(v), i->val, 1);
        }
    }
    return v;
}

Value *value_make_vector(Value *v)
{
    Value *r = value_new_vector(NULL);
    if (!v) return r;
    if (v->type == VALUE_VECTOR) {
        VECTOR(r) = vector_dup(VECTOR(v));
    } else if (v->type == VALUE_LIST) {
        for (const ListItem *i = LIST(v)->head;
                i != NULL; i = i->next) {
            vector_push_back(VECTOR(r), i->val, 1);
        }
    }
    return r;
}

void value_print(const Value *v)
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
    case VALUE_EXCEPTION:
    case VALUE_STRING:
    case VALUE_SYMBOL:
        fprintf(stderr, "%s", v->value.str);
        break;
    case VALUE_LIST:
        fprintf(stderr, "( ");
        Value *head;
        const List *tail = v->value.list;
        while((head = list_head(tail)) != NULL) {
            value_print(head);
            fprintf(stderr, " ");
            tail = list_tail(tail);
        }
        fprintf(stderr, ")");
        break;
    case VALUE_VECTOR:
        fprintf(stderr, "[ ");
        for (size_t i = 0; i < vector_size(VECTOR(v)); ++i) {
            value_print(vector_typed_at(VECTOR(v), i, Value));
        }
        fprintf(stderr, "]");
        break;
    case VALUE_FN:
        fprintf(stderr, "lambda: ");
        value_print(FN(v)->args);
        value_print(FN(v)->body);
        break;
    case VALUE_MACRO_FN:
        fprintf(stderr, "macro: ");
        value_print(FN(v)->args);
        value_print(FN(v)->body);
        break;
    case VALUE_BUILTIN_FN:
        fprintf(stderr, "#<@%p>", (void *) v->value.builtin_fn);
        break;
    }

}

Value *value_head(const Value *v)
{
    assert(v->type == VALUE_LIST && "Invalid argument: require list");
    return list_head(LIST(v));
}

Value *value_tail(const Value *v)
{
    assert(v->type == VALUE_LIST && "Invalid argument: require list");
    return value_new_list(list_tail(LIST(v)));
}

