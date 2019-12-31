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

Environment* env_new(Environment* parent)
{
    Environment* env = gc_malloc(&gc, sizeof(Environment));
    env->parent = parent;
    env->kv = map_new(32);
    return env;
}

void env_delete(Environment* env)
{
    map_delete(env->kv);
    gc_free(&gc, env);
}

void env_set(Environment* env, char* symbol, const Value* value)
{
    map_put(env->kv, symbol, (void*) value, sizeof(Value));
}

Value* env_get(Environment* env, char* symbol)
{
    Environment* cur_env = env;
    while(cur_env) {
        void* value = map_get(cur_env->kv, symbol);
        if (value) {
            return value;
        }
        cur_env = cur_env->parent;
    }
    return NULL;
}

bool env_contains(Environment* env, char* symbol)
{
    return env_get(env, symbol) != NULL;
}
