/*
 * env.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __ENV_H__
#define __ENV_H__

#include <stdlib.h>
#include "ht.h"

typedef struct env_t {
    ht_t* kv;
    struct env_t* parent;
} env_t;

env_t* env_new(env_t* parent);
void env_delete(env_t* env);

typedef void value_t; // FIXME: need value type

void env_set(env_t* env, char* symbol, value_t* value);
value_t* env_get(env_t* env, char* symbol);

#endif /* !__ENV_H__ */
