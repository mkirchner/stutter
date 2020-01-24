/*
 * env.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "env.h"
#include "gc.h"
#include "log.h"
#include "value.h"

Environment *env_new(Environment *parent)
{
    Environment *env = gc_malloc(&gc, sizeof(Environment));
    env->parent = parent;
    env->map = map_new(32);
    return env;
}

void env_set(Environment *env, char *symbol, const Value *value)
{
    map_put(env->map, symbol, (void *) value, sizeof(Value));
}

Value *env_get(Environment *env, char *symbol)
{
    Environment *cur_env = env;
    Value *value;
    while(cur_env) {
        if (cur_env->map) {
            if ((value = (Value *) map_get(cur_env->map, symbol))) {
                return value;
            }
        }
        cur_env = cur_env->parent;
    }
    return NULL;
}

bool env_contains(Environment *env, char *symbol)
{
    return env_get(env, symbol) != NULL;
}
