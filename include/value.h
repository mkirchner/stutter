#ifndef VALUE_H
#define VALUE_H

#include "env.h"
#include "gc.h"
#include "list.h"
#include "map.h"
#include "vector.h"

#define BOOL(v) (v->value.bool_)
#define BUILTIN_FN(v) (v->value.builtin_fn)
#define EXCEPTION(v) (v->value.str)
#define FLOAT(v) (v->value.float_)
#define FN(v) (v->value.fn)
#define INT(v)  (v->value.int_)
#define LIST(v) (v->value.list)
#define STRING(v) (v->value.str)
#define SYMBOL(v) (v->value.str)
#define VECTOR(v) (v->value.vector)

typedef enum {
    VALUE_BOOL       = 1 << 0,
    VALUE_BUILTIN_FN = 1 << 1,
    VALUE_EXCEPTION  = 1 << 2,
    VALUE_FLOAT      = 1 << 3,
    VALUE_FN         = 1 << 4,
    VALUE_INT        = 1 << 5,
    VALUE_LIST       = 1 << 6,
    VALUE_MACRO_FN   = 1 << 7,
    VALUE_NIL        = 1 << 8,
    VALUE_STRING     = 1 << 9,
    VALUE_SYMBOL     = 1 << 10,
    VALUE_VECTOR     = 1 << 11
} ValueType;

extern const char *value_type_names[];

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
        Vector *vector;
        const List *list;
        Map *map;
        struct Value *(*builtin_fn)(const struct Value *);
        CompositeFunction *fn;
    } value;
} Value;

/*
 * constants
 */
extern  Value *VALUE_CONST_TRUE;
extern  Value *VALUE_CONST_FALSE;
extern  Value *VALUE_CONST_NIL;

/*
 * functions
 */
bool is_symbol(const Value *value);
bool is_macro(const Value *value);
bool is_list(const Value *value);
bool is_vector(const Value *value);
bool is_exception(const Value *value);
Value *value_new_nil();
Value *value_new_bool(const bool bool_);
Value *value_new_exception(const char *str);
Value *value_make_exception(const char *fmt, ...);
Value *value_new_int(int int_);
Value *value_new_float(float float_);
Value *value_new_builtin_fn(Value * (fn)(const Value *));
Value *value_new_fn(Value *args, Value *body, Environment *env);
Value *value_new_macro(Value *args, Value *body, Environment *env);
Value *value_new_string(const char *str);
Value *value_new_symbol(const char *str);
Value *value_new_list(const List *l);
Value *value_make_list(Value *v);
Value *value_new_list(const List *l);
Value *value_make_list(Value *v);
Value *value_new_vector(const List *l);
Value *value_make_vector(Value *v);
Value *value_head(const Value *v);
Value *value_tail(const Value *v);
void value_delete(Value *v);
void value_print(const Value *v);


#endif /* !VALUE_H */
