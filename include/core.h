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

Value* core_sum(Value* args);
Value* core_list(Value* args);

#endif /* !CORE_H */
