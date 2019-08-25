/*
 * core.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "core.h"

#include "log.h"
#include "stdbool.h"

Value* core_sum(Value* args)
{
    if (!args) { // FIXME:  || !_is_list(args)) {
        if (args) {
            LOG_CRITICAL("Not a list: %d", args->type);
        }
        return NULL;
    }
    float sum = 0.0;
    bool all_int = true;
    Value* head;
    List* list = args->value.list;
    LOG_DEBUG("Initial list size: %ld", list_size(list));
    while ((head = list_head(list)) != NULL) {
        if (head->type == VALUE_FLOAT) {
            sum += head->value.float_;
            all_int = false;
        } else if (head->type == VALUE_INT) {
            sum += (float) head->value.int_;
        } else {
            LOG_CRITICAL("core.sum requires numeric arguments, got %d", head->type);
        }
        list = list_tail(list);
    }
    Value* ret;
    if (all_int) {
        ret = value_new_int((int) sum);
        LOG_DEBUG("apply returning: %d\n", ret->value.int_);
    } else {
        ret = value_new_float(sum); // FIXME: who frees this?
        LOG_DEBUG("apply returning: %f\n", ret->value.float_);
    }
    return ret;
}
