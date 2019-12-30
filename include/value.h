/*
 * value.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef VALUE_H
#define VALUE_H

#include "array.h"
#include "env.h"
#include "map.h"
#include "list.h"

typedef enum {
    VALUE_NIL,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
    VALUE_SYMBOL,
    VALUE_LIST,
    VALUE_BUILTIN_FN,
    VALUE_FN
} ValueType;


typedef struct CompositeFunction {
    struct Value* args;
    struct Value* body;
    Environment* env;
} CompositeFunction;

typedef struct Value {
    ValueType type;
    union {
        int int_;
        double float_;
        char* str;
        Array* vector;
        List* list;
        Map* map;
        struct Value* (*builtin_fn)(struct Value*);
        CompositeFunction* fn;
    } value;
} Value;

//
// functions
//
Value* value_new_nil();
Value* value_new_int(int int_);
Value* value_new_float(float float_);
Value* value_new_builtin_fn(Value* (fn)(Value*));
Value* value_new_fn(Value* args, Value* body, Environment* env);
Value* value_new_string(char* str);
Value* value_new_symbol(char* str);
Value* value_new_list(List* l);
void value_delete(Value* v);
void value_print(Value* v);


#endif /* !VALUE_H */
