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

Value* core_div(Value* args);
Value* core_eq(Value* args);
Value* core_list(Value* args);
Value* core_minus(Value* args);
Value* core_mul(Value* args);
Value* core_plus(Value* args);
Value* core_prn(Value* args);

#endif /* !CORE_H */
