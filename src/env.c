/*
 * env.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "env.h"
#include "log.h"

env_t* env_new(env_t* parent)
{
    env_t* env = malloc(sizeof(env_t));
    env->parent = parent;
    env->kv = ht_new(32);
    return env;
}

void env_delete(env_t* env)
{
    ht_delete(env->kv);
    free(env);
}

void env_set(env_t* env, char* symbol, value_t* value)
{
    ht_put(env->kv, symbol, value, sizeof(value_t));
}

value_t* env_get(env_t* env, char* symbol)
{
    env_t* cur_env = env;
    while(cur_env) {
        value_t* value = ht_get(cur_env->kv, symbol);
        if (value) {
            return value;
        }
        cur_env = cur_env->parent;
    }
    return NULL;
}
