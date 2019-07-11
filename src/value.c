/*
 * value.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "value.h"
#include <string.h>
#include "log.h"

static Value* value_new(ValueType type)
{
    Value* v = (Value*) malloc(sizeof(Value));
    v->type = type;
    return v;
}

Value* value_new_nil()
{
    Value* v = value_new(VALUE_NIL);
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

Value* value_new_string(char* str)
{
    Value* v = value_new(VALUE_STRING);
    v->value.str = strdup(str);
    return v;
}

Value* value_new_symbol(char* str)
{
    Value* v = value_new(VALUE_SYMBOL);
    v->value.str = strdup(str);
    return v;
}

Value* value_new_list()
{
    Value* v = value_new(VALUE_LIST);
    v->value.list = list_new();
    return v;
}

void value_delete(Value* v)
{
    switch(v->type) {
    case VALUE_NIL:
    case VALUE_INT:
    case VALUE_FLOAT:
        break;
    case VALUE_STRING:
    case VALUE_SYMBOL:
        free(v->value.str);
        break;
    case VALUE_LIST:
        list_delete(v->value.list);
        break;
    case VALUE_FN:
        // not implemented yet
        LOG_WARNING("%s", "value_delete() for VALUE_FN not implemented");
        break;
    }
    free(v);
}

void value_print(Value* v)
{
    if (!v) return;
    switch(v->type) {
    case VALUE_NIL:
        printf("NIL");
        break;
    case VALUE_INT:
        printf("%d", v->value.int_);
        break;
    case VALUE_FLOAT:
        printf("%f", v->value.float_);
        break;
    case VALUE_STRING:
    case VALUE_SYMBOL:
        printf("%s", v->value.str);
        break;
    case VALUE_LIST:
        printf("( ");
        Value* head;
        List* tail = v->value.list;
        while((head = list_head(tail)) != NULL) {
            value_print(head);
            printf(" ");
            tail = list_tail(tail);
        }
        printf(")");
        break;
    case VALUE_FN:
        // not implemented yet
        LOG_WARNING("%s", "value_print() for VALUE_FN not implemented");
        break;
    }

}
