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

extern  Value* CORE_TRUE;
extern  Value* CORE_FALSE;
extern  Value* CORE_NIL;

Value* core_count(const Value* args);
Value* core_div(const Value* args);
Value* core_eq(const Value* args);
Value* core_geq(const Value* args);
Value* core_gt(const Value* args);
Value* core_is_empty(const Value* args);
Value* core_is_list(const Value* args);
Value* core_leq(const Value* args);
Value* core_list(const Value* args);
Value* core_lt(const Value* args);
Value* core_minus(const Value* args);
Value* core_mul(const Value* args);
Value* core_plus(const Value* args);
Value* core_pr(const Value* args);
Value* core_pr_str(const Value* args);
Value* core_prn(const Value* args);
Value* core_str(const Value* args);

#endif /* !CORE_H */
