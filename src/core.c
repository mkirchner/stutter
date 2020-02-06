/*
 * core.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "core.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "apply.h"
#include "eval.h"
#include "log.h"

static bool is_true(const Value *v)
{
    return v->type == VALUE_BOOL && v->value.bool_;
}

static bool is_false(const Value *v)
{
    return v->type == VALUE_BOOL && !v->value.bool_;
}

static bool is_nil(const Value *v)
{
    return v->type == VALUE_NIL;
}

Value *core_list(const Value *args)
{
    return value_new_list(args->value.list);
}

Value *core_is_list(const Value *args)
{
    // FIXME: needs to be a list of lists to be a list
    return (args && args->type == VALUE_LIST) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
}

Value *core_is_empty(const Value *args)
{
    if (args && args->type == VALUE_LIST) {
        return list_size(args->value.list) == 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    }
    LOG_CRITICAL("Do not know how to determine emptyness of given type");
    return NULL;
}

static float acc_add(float acc, float x)
{
    return acc + x;
}

static float acc_sub(float acc, float x)
{
    return acc - x;
}

static float acc_mul(float acc, float x)
{
    return acc * x;
}

static float acc_div(float acc, float x)
{
    return acc / x;
}


static Value *core_acc(const Value *args, float (*accumulate)(float, float))
{
    if (!args || list_size(args->value.list) == 0) {
        if (args) {
            LOG_CRITICAL("Not a list: %d", args->type);
        }
        return NULL;
    }
    bool all_int = true;
    const List *list = args->value.list;
    Value *head = list_head(list);
    float acc;
    if (head->type == VALUE_FLOAT) {
        acc = head->value.float_;
        all_int = false;
    } else if (head->type == VALUE_INT) {
        acc = (float) head->value.int_;
    } else {
        LOG_CRITICAL("Require numeric arguments, got %d", head->type);
    }
    list = list_tail(list);
    while ((head = list_head(list)) != NULL) {
        if (head->type == VALUE_FLOAT) {
            acc = accumulate(acc, head->value.float_);
            all_int = false;
        } else if (head->type == VALUE_INT) {
            acc = accumulate(acc, (float) head->value.int_);
        } else {
            LOG_CRITICAL("Require numeric arguments, got %d", head->type);
        }
        list = list_tail(list);
    }
    Value *ret;
    if (all_int) {
        ret = value_new_int((int) acc);
    } else {
        ret = value_new_float(acc);
    }
    return ret;
}

Value *core_add(const Value *args)
{
    return core_acc(args, acc_add);
}

Value *core_sub(const Value *args)
{
    return core_acc(args, acc_sub);
}

Value *core_mul(const Value *args)
{
    return core_acc(args, acc_mul);
}

Value *core_div(const Value *args)
{
    return core_acc(args, acc_div);
}

static Value *cmp_eq(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            /* NIL equals NIL */
            return VALUE_CONST_TRUE;
        case VALUE_BOOL:
            return BOOL(a) == BOOL(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_INT:
            return INT(a) == INT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_FLOAT:
            return FLOAT(a) == FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_STRING:
        case VALUE_SYMBOL:
            return strcmp(STRING(a), STRING(b)) == 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_BUILTIN_FN:
            /* For built-in functions we currently use identity == equality */
            return BUILTIN_FN(a) == BUILTIN_FN(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_FN:
        case VALUE_MACRO_FN:
            /* For composite  functions we currently use identity == equality */
            return FN(a) == FN(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_LIST:
            if (list_size(LIST(a)) == list_size(LIST(b))) {
                /* empty lists can be equal */
                if (list_size(LIST(a)) == 0) {
                    return VALUE_CONST_TRUE;
                }
                /* else compare contents */
                const List *list_a = LIST(a);
                const List *list_b = LIST(b);
                Value *head_a;
                Value *head_b;
                while ((head_a = list_head(list_a)) && (head_b = list_head(list_b))) {
                    Value *cmp_result = cmp_eq(head_a, head_b);
                    if (!(cmp_result == VALUE_CONST_TRUE)) {
                        return cmp_result;  /* NULL or VALUE_CONST_FALSE */
                    }
                    list_a = list_tail(list_a);
                    list_b = list_tail(list_b);
                }
                return VALUE_CONST_TRUE;
            }
            return VALUE_CONST_FALSE;
        }
    }
    LOG_CRITICAL("Comparing incompatible types");
    return NULL;
}

static Value *cmp_lt(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            LOG_CRITICAL("Cannot order NIL values");
            return NULL;
        case VALUE_BOOL:
            LOG_CRITICAL("Cannot order boolean values");
            return NULL;
        case VALUE_INT:
            return INT(a) < INT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_FLOAT:
            return FLOAT(a) < FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_STRING:
        case VALUE_SYMBOL:
            return strcmp(STRING(a), STRING(b)) < 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_BUILTIN_FN:
        case VALUE_FN:
        case VALUE_MACRO_FN:
            LOG_CRITICAL("Cannot compare functions");
            return NULL;
        case VALUE_LIST:
            LOG_CRITICAL("Cannot order lists");
            return NULL;
        }
    }
    LOG_CRITICAL("Comparing incompatible types");
    return NULL;
}

static Value *cmp_leq(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            LOG_CRITICAL("Cannot less-equal compare NIL");
            return NULL;
        case VALUE_BOOL:
            LOG_CRITICAL("Cannot less-equal compare booleans");
            return NULL;
        case VALUE_INT:
            return INT(a) <= INT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_FLOAT:
            return FLOAT(a) <= FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_STRING:
        case VALUE_SYMBOL:
            return strcmp(STRING(a), STRING(b)) <= 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_BUILTIN_FN:
        case VALUE_FN:
        case VALUE_MACRO_FN:
            LOG_CRITICAL("Cannot less-equal compare functions");
            return NULL;
        case VALUE_LIST:
            LOG_CRITICAL("Cannot order lists");
            return NULL;
        }
    }
    LOG_CRITICAL("Comparing incompatible types");
    return NULL;
}

static Value *cmp_gt(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            LOG_CRITICAL("Cannot order NIL values");
            return NULL;
        case VALUE_BOOL:
            LOG_CRITICAL("Cannot order boolean values");
            return NULL;
        case VALUE_INT:
            return INT(a) > INT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_FLOAT:
            return FLOAT(a) > FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_STRING:
        case VALUE_SYMBOL:
            return strcmp(STRING(a), STRING(b)) > 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_BUILTIN_FN:
        case VALUE_MACRO_FN:
        case VALUE_FN:
            LOG_CRITICAL("Cannot compare functions");
            return NULL;
        case VALUE_LIST:
            LOG_CRITICAL("Cannot greater-compare lists");
            return NULL;
        }
    }
    LOG_CRITICAL("Comparing incompatible types");
    return NULL;
}

static Value *cmp_geq(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            LOG_CRITICAL("Cannot order NIL values");
            return NULL;
        case VALUE_BOOL:
            LOG_CRITICAL("Cannot order boolean values");
            return NULL;
        case VALUE_INT:
            return INT(a) >= INT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_FLOAT:
            return FLOAT(a) >= FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_STRING:
        case VALUE_SYMBOL:
            return strcmp(STRING(a), STRING(b)) >= 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_BUILTIN_FN:
        case VALUE_FN:
        case VALUE_MACRO_FN:
            LOG_CRITICAL("Cannot compare functions");
            return NULL;
        case VALUE_LIST:
            LOG_CRITICAL("Cannot order lists");
            return NULL;
        }
    }
    LOG_CRITICAL("Comparing incompatible types");
    return NULL;
}

static Value *compare(const Value *args, Value * (*comparison_fn)(const Value *, const Value *))
{
    // (= a b c)
    if (!args) {
        LOG_CRITICAL("Require an argument");
        return NULL;
    }
    if (!(args->type == VALUE_LIST && list_size(args->value.list) >= 2)) {
        LOG_CRITICAL("Require a list argument of size greater or equal to 2");
        return NULL;
    }
    const List *list = args->value.list;
    Value *head;
    Value *prev = NULL;
    while ((head = list_head(list)) != NULL) {
        if (prev) {
            Value *cmp_result = comparison_fn(prev, head);
            if (!(cmp_result == VALUE_CONST_TRUE)) {
                return cmp_result;  /* NULL or VALUE_CONST_FALSE */
            }
        }
        prev = head;
        list = list_tail(list);
    }
    return VALUE_CONST_TRUE;
}

Value *core_eq(const Value *args)
{
    return compare(args, cmp_eq);
}

Value *core_lt(const Value *args)
{
    return compare(args, cmp_lt);
}

Value *core_leq(const Value *args)
{
    return compare(args, cmp_leq);
}

Value *core_gt(const Value *args)
{
    return compare(args, cmp_gt);
}

Value *core_geq(const Value *args)
{
    return compare(args, cmp_geq);
}


static char *str_append(char *str, size_t n_str, char *partial, size_t n_partial)
{
    str = realloc(str, n_str + n_partial + 1);
    strncat(str, partial, n_partial);
    return str;
}

static char *core_str_inner(char *str, const Value *v)
{
    char *partial;
    switch(v->type) {
    case VALUE_NIL:
        str = str_append(str, strlen(str), "nil", 3);
        break;
    case VALUE_BOOL:
        partial = BOOL(v) ? "true" : "false";
        str = str_append(str, strlen(str), partial, strlen(partial));
        break;
    case VALUE_INT:
        asprintf(&partial, "%d", INT(v));
        str = str_append(str, strlen(str), partial, strlen(partial));
        free(partial);
        break;
    case VALUE_FLOAT:
        asprintf(&partial, "%f", FLOAT(v));
        str = str_append(str, strlen(str), partial, strlen(partial));
        free(partial);
        break;
    case VALUE_STRING:
    case VALUE_SYMBOL:
        asprintf(&partial, "%s", STRING(v));
        str = str_append(str, strlen(str), partial, strlen(partial));
        free(partial);
        break;
    case VALUE_LIST:
        str = str_append(str, strlen(str), "(", 1);
        Value *head2;
        const List *tail2 = v->value.list;
        while((head2 = list_head(tail2)) != NULL) {
            str = core_str_inner(str, head2);
            tail2 = list_tail(tail2);
            if (list_head(tail2)) {
                str = str_append(str, strlen(str), " ", 1);
            }
        }
        str = str_append(str, strlen(str), ")", 1);
        break;
    case VALUE_FN:
    case VALUE_MACRO_FN:
        str = str_append(str, strlen(str), "(lambda ", 8);
        str = core_str_inner(str, FN(v)->args);
        str = str_append(str, strlen(str), " ", 1);
        str = core_str_inner(str, FN(v)->body);
        str = str_append(str, strlen(str), ")", 1);
        break;
    case VALUE_BUILTIN_FN:
        asprintf(&partial, "#<builtin_fn@%p>", (void *) v->value.builtin_fn);
        str = str_append(str, strlen(str), partial, strlen(partial));
        free(partial);
        break;
    }
    return str;
}

Value *core_str_outer(const Value *args, bool printable)
{
    if (!args)
        return value_new_string("");

    char *str = calloc(1, sizeof(char));
    if (args->type == VALUE_LIST) {
        const List *list = LIST(args);
        Value *head;
        while ((head = list_head(list)) != NULL) {
            str = core_str_inner(str, head);
            list = list_tail(list);
            if (printable) {
                str = str_append(str, strlen(str), " ", 1);
            }
        }
    } else {
        str = core_str_inner(str, args);
    }
    Value *ret = value_new_string(str);
    free(str);
    return ret;
}

Value *core_str(const Value *args)
{
    return core_str_outer(args, false);
}

Value *core_pr(const Value *args)
{
    Value *str = core_str_outer(args, true);
    fprintf(stdout, "%s", str->value.str);
    return VALUE_CONST_NIL;
}


Value *core_pr_str(const Value *args)
{
    return core_str_outer(args, true);
}


Value *core_prn(const Value *args)
{
    Value *str = core_str_outer(args, true);
    fprintf(stdout, "%s", str->value.str);
    fprintf(stdout, "\n");
    fflush(stdout);
    return VALUE_CONST_NIL;
}


Value *core_count(const Value *args)
{
    Value *list = list_head(LIST(args));
    if (list->type != VALUE_LIST) {
        LOG_CRITICAL("count requires a list argument");
        return NULL;
    }
    return value_new_int(list_size(LIST(list)));
}

Value *core_slurp(const Value *args)
{
    // This is not for binary streams since we're using ftell.
    // (It's portable, though)
    if (args->type == VALUE_LIST && list_size(LIST(args)) == 1) {
        Value *v = list_head(LIST(args));
        Value *retval = NULL;
        FILE *f = NULL;
        if (!(f = fopen(v->value.str, "r"))) {
            LOG_CRITICAL("Failed to open file %s: %s", v->value.str, strerror(errno));
            goto out;
        }
        int ret;
        if ((ret = fseek(f, 0L, SEEK_END)) != 0) {
            LOG_CRITICAL("Failed to determine file size for %s: %s",
                         v->value.str, strerror(errno));
            goto out_file;
        }
        long fsize;
        if ((fsize = ftell(f)) < 0) {
            LOG_CRITICAL("Failed to determine file size for %s: %s",
                         v->value.str, strerror(errno));
            goto out_file;
        }
        char *buf = malloc(fsize + 1);
        if ((ret = fseek(f, 0L, SEEK_SET)) != 0) {
            LOG_CRITICAL("Failed to read file %s", v->value.str);
            goto out_buf;
        }
        size_t n_read;
        if ((n_read = fread(buf, 1, fsize, f)) < (size_t) fsize)   {
            LOG_CRITICAL("Failed to read file %s", v->value.str);
            goto out_buf;
        }
        buf[fsize] = '\0';
        retval = value_new_string(buf); // FIXME: fx value constructors to avoid copy
out_buf:
        free(buf);
out_file:
        fclose(f);
out:
        return retval;
    }
    LOG_CRITICAL("Wrong argument type or wrong number of arguments for slurp");
    return NULL;
}


#define REQUIRE_ARGS(args) do  { if (!args) { return NULL; } } while (0)
#define REQUIRE_TYPE(args, t) do  { if (args->type != t) { LOG_CRITICAL("Type mismatch"); return NULL; } } while (0)
#define REQUIRE_CARDINALITY(args, n) do { if (list_size(args->value.list) != n) { LOG_CRITICAL("Wrong number of arguments"); return NULL; } } while (0)
#define REQUIRE_CARDINALITY_GE(args, n) do { if (list_size(args->value.list) < n) { LOG_CRITICAL("Wrong number of arguments"); return NULL; } } while (0)

Value *core_cons(const Value *args)
{
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    REQUIRE_CARDINALITY(args, 2);
    Value *first = list_head(LIST(args));
    Value *second = list_head(list_tail(LIST(args)));
    REQUIRE_TYPE(second, VALUE_LIST);
    return value_new_list(list_cons(LIST(second), first));
}

Value *core_concat(const Value *args)
{
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    const List *concat = list_new();
    for (const ListItem *i = LIST(args)->begin; i != NULL; i = i->next) {
        Value *v = (Value *) i->p;
        REQUIRE_TYPE(v, VALUE_LIST);
        for (const ListItem *j = LIST(v)->begin; j != NULL; j = j->next) {
            concat = list_conj(concat, j->p);
        }
    }
    return value_new_list(concat);
}

Value *core_map(const Value *args)
{
    /* (map f '(a b c ...)) */
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    REQUIRE_CARDINALITY(args, 2);
    Value *fn = list_nth(LIST(args), 0);
    Value *fn_args = list_nth(LIST(args), 1);

    REQUIRE_TYPE(fn_args, VALUE_LIST);
    const List *mapped = list_new();
    Value *tco_expr;
    Environment *tco_env;
    for (size_t i = 0; i < list_size(LIST(fn_args)); ++i) {
        Value *result = apply(fn, value_make_list(list_nth(LIST(fn_args), i)),
                              &tco_expr, &tco_env);
        /* need to call eval since apply defers to eval for TCO support */
        mapped = list_conj(mapped, tco_expr ? eval(tco_expr, tco_env) : result);
    }
    return value_new_list(mapped);
}

Value *core_apply(const Value *args)
{
    /* (apply f a b c d ...) == (f a b c d ...) */
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    REQUIRE_CARDINALITY_GE(args, 2);
    Value *fn = list_head(LIST(args));
    Value *fn_args = value_new_list(list_tail(LIST(args)));
    size_t n_args = list_size(LIST(fn_args));

    /* Merge the arguments w/ a potential list of arguments at the end of
     * the argument list */
    if (is_list(list_nth(LIST(fn_args), n_args - 1))) {
        const List* concat = list_new();
        for (const ListItem *j = LIST(fn_args)->begin; j != LIST(fn_args)->end; j = j->next) {
            concat = list_conj(concat, j->p);
        }
        for (const ListItem *j = LIST(((Value *)(LIST(fn_args)->end->p)))->begin; j != NULL; j = j->next) {
            concat = list_conj(concat, j->p);
        }
        fn_args = value_new_list(concat);
    }
    Value *tco_expr;
    Environment *tco_env;
    Value *result = apply(fn, fn_args, &tco_expr, &tco_env);
    /* need to call eval since apply defers to eval for TCO support */
    return  tco_expr ? eval(tco_expr, tco_env) : result;
}

Value *core_is_nil(const Value *args)
{
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    REQUIRE_CARDINALITY(args, 1);
    Value *expr = list_head(LIST(args));
    return value_new_bool(is_nil(expr));
}

Value *core_is_true(const Value *args)
{
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    REQUIRE_CARDINALITY(args, 1);
    Value *expr = list_head(LIST(args));
    return value_new_bool(is_true(expr));
}

Value *core_is_false(const Value *args)
{
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    REQUIRE_CARDINALITY(args, 1);
    Value *expr = list_head(LIST(args));
    return value_new_bool(is_false(expr));
}

Value *core_is_symbol(const Value *args)
{
    REQUIRE_ARGS(args);
    REQUIRE_TYPE(args, VALUE_LIST);
    REQUIRE_CARDINALITY(args, 1);
    Value *expr = list_head(LIST(args));
    return value_new_bool(is_symbol(expr));
}
