/*
 * eval.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __EVAL_H__
#define __EVAL_H__

#include <env.h>
#include <value.h>

Value* eval(Value* expr, Environment* env);

#endif /* !EVAL_H */
