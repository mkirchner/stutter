/*
 * core.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __CORE_H__
#define __CORE_H__

#include "value.h"
#include "env.h"

typedef struct {
    char *name;
    Value *(*fn)(const Value *args);
} CoreFn;

extern CoreFn core_fns[];

Value *core_add(const Value *args);
Value *core_apply(const Value *args);
Value *core_assert(const Value *args);
Value *core_concat(const Value *args);
Value *core_cons(const Value *args);
Value *core_count(const Value *args);
Value *core_div(const Value *args);
Value *core_eq(const Value *args);
Value *core_geq(const Value *args);
Value *core_gt(const Value *args);
Value *core_is_empty(const Value *args);
Value *core_is_false(const Value *args);
Value *core_is_list(const Value *args);
Value *core_is_nil(const Value *args);
Value *core_is_symbol(const Value *args);
Value *core_is_true(const Value *args);
Value *core_leq(const Value *args);
Value *core_list(const Value *args);
Value *core_lt(const Value *args);
Value *core_map(const Value *args);
Value *core_mul(const Value *args);
Value *core_pr(const Value *args);
Value *core_pr_str(const Value *args);
Value *core_prn(const Value *args);
Value *core_slurp(const Value *args);
Value *core_str(const Value *args);
Value *core_sub(const Value *args);
Value *core_symbol(const Value *args);

/* utility functions */
bool is_truthy(const Value *v);

#endif /* !CORE_H */
