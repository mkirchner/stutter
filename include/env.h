/*
 * env.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __ENV_H__
#define __ENV_H__

#include <stdlib.h>
#include "map.h"

struct Value;

typedef struct Environment {
    Map* map;
    struct Environment* parent;
} Environment;

Environment* env_new(Environment* parent);
void env_delete(Environment* env);

void env_set(Environment* env, char* symbol, const struct Value* value);
struct Value* env_get(Environment* env, char* symbol);
bool env_contains(Environment* env, char* symbol);

#endif /* !__ENV_H__ */
