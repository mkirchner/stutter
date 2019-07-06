/*
 * env.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "env.h"
#include "log.h"

Environment* env_new(Environment* parent)
{
    Environment* env = malloc(sizeof(Environment));
    env->parent = parent;
    env->kv = map_new(32);
    return env;
}

void env_delete(Environment* env)
{
    map_delete(env->kv);
    free(env);
}

void env_set(Environment* env, char* symbol, void* value)
{
    map_put(env->kv, symbol, value, sizeof(void));
}

void* env_get(Environment* env, char* symbol)
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
