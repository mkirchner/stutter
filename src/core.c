/*
 * core.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "core.h"

#include <stdbool.h>
#include <string.h>
#include "log.h"

Value* CORE_TRUE = &((Value){ .type = VALUE_BOOL, .value = { .bool_ = true } });
Value* CORE_FALSE = &((Value){ .type = VALUE_BOOL, .value = { .bool_ = false } });
Value* CORE_NIL = &((Value){ .type = VALUE_NIL, .value = { .float_ = 0.0 } });

static bool is_true(const Value* v)
{
    return v == CORE_TRUE;
}

static bool is_false(const Value* v)
{
    return v == CORE_FALSE;
}

static bool is_nil(const Value* v)
{
    return v == CORE_NIL;
}

Value* core_list(Value* args)
{
    List* list = args->value.list;
    LOG_DEBUG("Initial list size: %ld", list_size(list));
    return value_new_list(args->value.list);
}

static float acc_plus(float acc, float x)
{
    return acc + x;
}

static float acc_minus(float acc, float x)
{
    return acc - x;
}

static float acc_mul(float acc, float x)
{
    return acc * x;
}

static float acc_div(float acc, float x)
{
    return acc / x;
}


static Value* core_acc(Value* args, float (*accumulate)(float, float))
{
    if (!args || list_size(args->value.list) == 0) {
        if (args) {
            LOG_CRITICAL("Not a list: %d", args->type);
        }
        return NULL;
    }
    bool all_int = true;
    List* list = args->value.list;
    Value* head = list_head(list);
    float acc;
    if (head->type == VALUE_FLOAT) {
        acc = head->value.float_;
        all_int = false;
    } else if (head->type == VALUE_INT) {
        acc = (float) head->value.int_;
    } else {
        LOG_CRITICAL("plus builtin requires numeric arguments, got %d", head->type);
    }
    list = list_tail(list);
    while ((head = list_head(list)) != NULL) {
        if (head->type == VALUE_FLOAT) {
            acc = accumulate(acc, head->value.float_);
            all_int = false;
        } else if (head->type == VALUE_INT) {
            acc = accumulate(acc, (float) head->value.int_);
        } else {
            LOG_CRITICAL("plus builtin requires numeric arguments, got %d", head->type);
        }
        list = list_tail(list);
    }
    Value* ret;
    if (all_int) {
        ret = value_new_int((int) acc);
    } else {
        ret = value_new_float(acc);
    }
    return ret;
}

Value* core_plus(Value* args)
{
    return core_acc(args, acc_plus);
}

Value* core_minus(Value* args)
{
    return core_acc(args, acc_minus);
}

Value* core_mul(Value* args)
{
    return core_acc(args, acc_mul);
}

Value* core_div(Value* args)
{
    return core_acc(args, acc_div);
}

static bool is_equal(Value* a, Value* b) {
    // FIXME: be very clear about identity vs. equality
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            /* NIL equals NIL */
            return true;
        case VALUE_BOOL:
            return BOOL(a) == BOOL(b) ? true : false;
        case VALUE_INT:
            return INT(a) == INT(b) ? true : false;
        case VALUE_FLOAT:
            return FLOAT(a) == FLOAT(b) ? true : false;
        case VALUE_STRING:
        case VALUE_SYMBOL:
            return strcmp(STRING(a), STRING(b)) == 0;
        case VALUE_BUILTIN_FN:
            /* built-in fns are equal if they point to the same address */
            return BUILTIN_FN(a) == BUILTIN_FN(b) ? true : false;
        case VALUE_FN:
            /* fns are equal if they point to the same composite fn object */
            /* FIXME: this is identity... */
            return FN(a) == FN(b) ? true : false;
        case VALUE_LIST:
            if (list_size(LIST(a)) == list_size(LIST(b))) {
                /* empty lists can be equal */
                if (list_size(LIST(a)) == 0) {
                    return true;
                }
                /* else compare contents */
                List* list_a = LIST(a);
                List* list_b = LIST(b);
                Value* head_a;
                Value* head_b;
                while ((head_a = list_head(list_a)) && (head_b = list_head(list_b))) {
                    if (!is_equal(head_a, head_b)) return false;
                    list_a = list_tail(list_a);
                    list_b = list_tail(list_b);
                }
                return true;
            }
            return false;
        }
    }
    return false;
}

Value* core_eq(Value* args)
{
    // (= a b c)
    if (!args) {
        LOG_CRITICAL("Require an argument");
        return NULL;
    }
    if (!(args->type == VALUE_LIST && list_size(args->value.list) >= 2)) {
        LOG_CRITICAL("Require a list argument of size greater or equal to 2");
        return NULL;
    }
    List* list = args->value.list;
    Value* head;
    Value* prev = NULL;
    while ((head = list_head(list)) != NULL) {
        if (prev && !is_equal(prev, head)) {
            return CORE_FALSE;
        }
        prev = head;
        list = list_tail(list);
    }
    return CORE_TRUE;
}


Value* core_prn(Value* args)
{
    value_print(args);
    return CORE_NIL;
}
