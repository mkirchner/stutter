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
#include "gc.h"
#include "map.h"
#include "list.h"

#define BOOL(v) (v->value.bool_)
#define INT(v)  (v->value.int_)
#define FLOAT(v) (v->value.float_)
#define STRING(v) (v->value.str)
#define SYMBOL(v) (v->value.str)
#define LIST(v) (v->value.list)
#define BUILTIN_FN(v) (v->value.builtin_fn)
#define FN(v) (v->value.fn)

typedef enum {
    VALUE_NIL,
    VALUE_BOOL,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
    VALUE_SYMBOL,
    VALUE_LIST,
    VALUE_BUILTIN_FN,
    VALUE_FN,
    VALUE_MACRO_FN
} ValueType;


typedef struct CompositeFunction {
    struct Value *args;
    struct Value *body;
    Environment *env;
} CompositeFunction;

typedef struct Value {
    ValueType type;
    union {
        bool bool_;
        int int_;
        double float_;
        char *str;
        Array *vector;
        const List *list;
        Map *map;
        struct Value *(*builtin_fn)(const struct Value *);
        CompositeFunction *fn;
    } value;
} Value;

//
// functions
//
bool is_symbol(const Value *value);
bool is_macro(const Value *value);
bool is_list(const Value *value);
Value *value_new_nil();
Value *value_new_bool(const bool bool_);
Value *value_new_int(int int_);
Value *value_new_float(float float_);
Value *value_new_builtin_fn(Value * (fn)(const Value *));
Value *value_new_fn(Value *args, Value *body, Environment *env);
Value *value_new_macro(Value *args, Value *body, Environment *env);
Value *value_new_string(const char *str);
Value *value_new_symbol(const char *str);
Value *value_new_list(const List *l);
Value *value_make_list(Value *v);
Value *value_head(const Value *v);
Value *value_tail(const Value *v);
void value_delete(Value *v);
void value_print(const Value *v);


#endif /* !VALUE_H */
