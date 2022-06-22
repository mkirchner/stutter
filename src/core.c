#include "core.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "apply.h"
#include "eval.h"
#include "exc.h"
#include "log.h"


#define NARGS(args) list_size(LIST(args))
#define ARG(args, n) list_nth(LIST(args), n)

#define CHECK_ARGLIST(args) do  {\
    if (!(args && args->type == VALUE_LIST)) {\
        exc_set(value_make_exception("Invalid argument list in core function"));\
        return NULL;\
    }\
} while (0)

#define REQUIRE_VALUE_TYPE(value, t, msg) do  {\
    if (!(value->type & (t))) {\
        LOG_CRITICAL("%s: expected %s, got %s", msg, value_type_names[t], value_type_names[value->type]);\
        exc_set(value_make_exception("%s: expected %s, got %s", msg, value_type_names[t], value_type_names[value->type]));\
        return NULL;\
    }\
} while (0)


#define REQUIRE_LIST_CARDINALITY(val, n, msg) do {\
    if (list_size(val->value.list) != n) {\
        LOG_CRITICAL("%s: expected %lu, got %lu", msg, n, list_size(val->value.list));\
        exc_set(value_make_exception("%s: expected %lu, got %lu", msg, n, list_size(val->value.list)));\
        return NULL;\
    }\
} while (0)

#define REQUIRE_LIST_CARDINALITY_GE(val, n, msg) do {\
    if (list_size(val->value.list) < (size_t) n) {\
        LOG_CRITICAL("%s: expected at least %lu, got %lu", msg, n, list_size(val->value.list));\
        exc_set(value_make_exception("%s: expected at least %lu, got %lu", msg, n, list_size(val->value.list)));\
        return NULL;\
    }\
} while (0)


bool is_truthy(const Value *v)
{
    /* we follow Clojure's lead: the only values that are considered
     * logical false are `false` and `nil` */
    assert(v);
    switch(v->type) {
    case VALUE_NIL:
        return false;
    case VALUE_EXCEPTION:
        return false;
    case VALUE_BOOL:
        return v->value.bool_ == true;
    case VALUE_INT:
    case VALUE_FLOAT:
    case VALUE_STRING:
    case VALUE_SYMBOL:
    case VALUE_LIST:
    case VALUE_FN:
    case VALUE_MACRO_FN:
    case VALUE_BUILTIN_FN:
        return true;
    }
}

static bool is_true(const Value *v)
{
    assert(v);
    return v->type == VALUE_BOOL && v->value.bool_;
}

static bool is_false(const Value *v)
{
    assert(v);
    return v->type == VALUE_BOOL && !v->value.bool_;
}

static bool is_nil(const Value *v)
{
    assert(v);
    return v->type == VALUE_NIL;
}

Value *core_list(const Value *args)
{
    CHECK_ARGLIST(args);
    return value_new_list(LIST(args));
}

Value *core_is_list(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "list? requires exactly one parameter");
    Value *arg0 = ARG(args, 0);
    return arg0->type == VALUE_LIST ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
}

Value *core_vector(const Value *args)
{
    CHECK_ARGLIST(args);
    return value_new_vector(LIST(args));
}

Value *core_is_vector(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "vector? requires exactly one parameter");
    Value *arg0 = ARG(args, 0);
    return arg0->type == VALUE_VECTOR ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
}

Value *core_is_empty(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "empty? requires exactly one parameter");
    Value *arg0 = ARG(args, 0);
    REQUIRE_VALUE_TYPE(arg0, VALUE_LIST, "empty? requires a list type");
    return NARGS(arg0) == 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
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


static Value *core_acc(const Value *args, float (*acc_fn)(float, float))
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY_GE(args, 1ul, "Require at least one argument");
    assert(acc_fn);
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
        exc_set(value_make_exception("Non-numeric argument in accumulation"));
        return NULL;
    }
    list = list_tail(list);
    while ((head = list_head(list)) != NULL) {
        if (head->type == VALUE_FLOAT) {
            acc = acc_fn(acc, head->value.float_);
            all_int = false;
        } else if (head->type == VALUE_INT) {
            acc = acc_fn(acc, (float) head->value.int_);
        } else {
            exc_set(value_make_exception("Non-numeric argument in accumulation"));
            return NULL;
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
        case VALUE_EXCEPTION:
            /* Errors do not support comparison */
            exc_set(value_make_exception("Comparison of error values is not supported"));
            return NULL;
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
    } else if (a->type == VALUE_INT && b->type == VALUE_FLOAT) {
        return ((float) INT(a)) == FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    } else if (b->type == VALUE_INT && a->type == VALUE_FLOAT) {
        return ((float) INT(b)) == FLOAT(a) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    } else if (b->type == VALUE_NIL || a->type == VALUE_NIL) {
        /* nil can be compared to anything but will yield false unless compared
         * to itself */
        return VALUE_CONST_FALSE;
    }
    exc_set(value_make_exception("Cannot compare incompatible types"));
    return NULL;
}

static Value *cmp_lt(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            exc_set(value_make_exception("Cannot order NIL values"));
            return NULL;
        case VALUE_EXCEPTION:
            exc_set(value_make_exception("Cannot order EXCEPTION values"));
            return NULL;
        case VALUE_BOOL:
            exc_set(value_make_exception("Cannot order BOOLEAN values"));
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
            exc_set(value_make_exception("Cannot order functions"));
            return NULL;
        case VALUE_LIST:
            exc_set(value_make_exception("Cannot order lists"));
            return NULL;
        case VALUE_VECTOR:
            exc_set(value_make_exception("Cannot order lists"));
            return NULL;
        }
    } else if (a->type == VALUE_INT && b->type == VALUE_FLOAT) {
        return ((float) INT(a)) < FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    } else if (b->type == VALUE_INT && a->type == VALUE_FLOAT) {
        return FLOAT(a) < ((float) INT(b)) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    }
    exc_set(value_make_exception("Cannot compare incompatible types"));
    return NULL;
}

static Value *cmp_leq(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            exc_set(value_make_exception("Cannot order NIL values"));
            return NULL;
        case VALUE_EXCEPTION:
            exc_set(value_make_exception("Cannot order EXCEPTION values"));
            return NULL;
        case VALUE_BOOL:
            exc_set(value_make_exception("Cannot order BOOLEAN values"));
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
            exc_set(value_make_exception("Cannot order functions"));
            return NULL;
        case VALUE_LIST:
            exc_set(value_make_exception("Cannot order lists"));
            return NULL;
        }
    } else if (a->type == VALUE_INT && b->type == VALUE_FLOAT) {
        return ((float) INT(a)) <= FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    } else if (b->type == VALUE_INT && a->type == VALUE_FLOAT) {
        return FLOAT(a) <= ((float) INT(b)) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    }
    exc_set(value_make_exception("Cannot compare incompatible types"));
    return NULL;
}

static Value *cmp_gt(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            exc_set(value_make_exception("Cannot order NIL values"));
            return NULL;
        case VALUE_EXCEPTION:
            exc_set(value_make_exception("Cannot order EXCEPTION values"));
            return NULL;
        case VALUE_BOOL:
            exc_set(value_make_exception("Cannot order BOOLEAN values"));
            return NULL;
        case VALUE_INT:
            return INT(a) > INT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_FLOAT:
            return FLOAT(a) > FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_STRING:
        case VALUE_SYMBOL:
            return strcmp(STRING(a), STRING(b)) > 0 ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
        case VALUE_BUILTIN_FN:
        case VALUE_FN:
        case VALUE_MACRO_FN:
            exc_set(value_make_exception("Cannot order functions"));
            return NULL;
        case VALUE_LIST:
            exc_set(value_make_exception("Cannot order lists"));
            return NULL;
        }
    } else if (a->type == VALUE_INT && b->type == VALUE_FLOAT) {
        return ((float) INT(a)) > FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    } else if (b->type == VALUE_INT && a->type == VALUE_FLOAT) {
        return FLOAT(a) > ((float) INT(b)) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    }
    exc_set(value_make_exception("Cannot compare incompatible types"));
    return NULL;
}

static Value *cmp_geq(const Value *a, const Value *b)
{
    if (a->type == b->type) {
        switch(a->type) {
        case VALUE_NIL:
            exc_set(value_make_exception("Cannot order NIL values"));
            return NULL;
        case VALUE_EXCEPTION:
            exc_set(value_make_exception("Cannot order EXCEPTION values"));
            return NULL;
        case VALUE_BOOL:
            exc_set(value_make_exception("Cannot order BOOLEAN values"));
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
            exc_set(value_make_exception("Cannot order functions"));
            return NULL;
        case VALUE_LIST:
            exc_set(value_make_exception("Cannot order lists"));
            return NULL;
        }
    } else if (a->type == VALUE_INT && b->type == VALUE_FLOAT) {
        return ((float) INT(a)) >= FLOAT(b) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    } else if (b->type == VALUE_INT && a->type == VALUE_FLOAT) {
        return FLOAT(a) >= ((float) INT(b)) ? VALUE_CONST_TRUE : VALUE_CONST_FALSE;
    }
    exc_set(value_make_exception("Cannot compare incompatible types"));
    return NULL;
}

static Value *compare(const Value *args, Value * (*comparison_fn)(const Value *, const Value *))
{
    // (= a b c)
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY_GE(args, 2ul, "Require at least two values to compare");
    const List *list = LIST(args);
    Value *head;
    Value *prev = NULL;
    while ((head = list_head(list)) != NULL) {
        if (prev) {
            Value *cmp_result = comparison_fn(prev, head);
            if (!(cmp_result == VALUE_CONST_TRUE)) {
                return cmp_result;
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
    case VALUE_EXCEPTION:
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
    case VALUE_VECTOR:
        str = str_append(str, strlen(str), "[", 1);
        size_t vec_size = vector_size(VECTOR(v));
        for (size_t i = 0; i < vec_size; ++i) {
            str = core_str_inner(str, vector_typed_at(VECTOR(v), i, Value));
            if (i < vec_size - 1) {
                str = str_append(str, strlen(str), " ", 1);
            }
        }
        str = str_append(str, strlen(str), "]", 1);
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
    CHECK_ARGLIST(args);
    const Value *seq = ARG(args, 0);
    size_t cnt = 0;
    if (!is_nil(seq)) {
        REQUIRE_VALUE_TYPE(seq, VALUE_LIST | VALUE_VECTOR, "count requires a sequence argument");
        cnt = is_list(seq) ? list_size(LIST(seq)) : vector_size(VECTOR(seq));
    }
    return value_new_int(cnt);
}

Value *core_slurp(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "slurp takes exactly one argument");
    // This is not for binary streams since we're using ftell.
    // (It's portable, though)
    Value *v = ARG(args, 0);
    REQUIRE_VALUE_TYPE(v, VALUE_STRING, "slurp takes a string argument");
    Value *retval = NULL;
    FILE *f = NULL;
    if (!(f = fopen(STRING(v), "r"))) {
        exc_set(value_make_exception("Failed to open file %s: %s", STRING(v), strerror(errno)));
        goto out;
    }
    int ret;
    if ((ret = fseek(f, 0L, SEEK_END)) != 0) {
        exc_set(value_make_exception("Failed to determine file size for %s: %s",
                                     STRING(v), strerror(errno)));
        goto out_file;
    }
    long fsize;
    if ((fsize = ftell(f)) < 0) {
        exc_set(value_make_exception("Failed to determine file size for %s: %s",
                                     STRING(v), strerror(errno)));
        goto out_file;
    }
    char *buf = malloc(fsize + 1);
    if ((ret = fseek(f, 0L, SEEK_SET)) != 0) {
        exc_set(value_make_exception("Failed to read file %s", STRING(v)));
        goto out_buf;
    }
    size_t n_read;
    if ((n_read = fread(buf, 1, fsize, f)) < (size_t) fsize)   {
        exc_set(value_make_exception("Failed to read file %s", STRING(v)));
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


Value *core_cons(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 2ul, "CONS takes exactly two arguments");
    Value *first = ARG(args, 0);
    Value *second = ARG(args, 1);
    REQUIRE_VALUE_TYPE(second, VALUE_LIST, "the second parameter to CONS must be a list");
    return value_new_list(list_prepend(LIST(second), first));
}

Value *core_concat(const Value *args)
{
    CHECK_ARGLIST(args);
    const List *concat = list_new();
    for (const ListItem *i = LIST(args)->head; i != NULL; i = i->next) {
        Value *v = i->val;
        REQUIRE_VALUE_TYPE(v, VALUE_LIST, "all parameters to CONCAT must be lists");
        for (const ListItem *j = LIST(v)->head; j != NULL; j = j->next) {
            concat = list_append(concat, j->val);
        }
    }
    return value_new_list(concat);
}

Value *core_map(const Value *args)
{
    /* (map f '(a b c ...)) */
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 2ul, "MAP takes exactly two parameters");
    Value *fn = ARG(args, 0);
    Value *fn_args = ARG(args, 1);

    REQUIRE_VALUE_TYPE(fn_args, VALUE_LIST, "The second parameter to MAP must be a list");
    const List *mapped = list_new();
    Value *tco_expr = NULL;
    Environment *tco_env;
    for (size_t i = 0; i < list_size(LIST(fn_args)); ++i) {
        Value *result = apply(fn, value_make_list(ARG(fn_args, i)),
                              &tco_expr, &tco_env);
        /* apply() may defer to eval() because of TCO support, we
         * need to catch that and eval the expression */
        if (tco_expr && !exc_is_pending()) {
            result = eval(tco_expr, tco_env);
        }
        if (!result) {
            assert(exc_is_pending());
            return NULL;
        }
        mapped = list_append(mapped, result);
    }
    return value_new_list(mapped);
}

Value *core_apply(const Value *args)
{
    /* (apply f a b c d ...) == (f a b c d ...) */
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY_GE(args, 2ul, "APPLY requires at least two arguments");
    Value *fn = ARG(args, 0);
    Value *fn_args = value_new_list(list_tail(LIST(args)));
    size_t n_args = NARGS(fn_args);

    /* The last argument may be a list; if it is, we need to prepend
     * the other args to that list to yield the final list of arguments */
    if (n_args > 0 && is_list(ARG(fn_args, n_args - 1))) {
        const List *concat = list_dup(LIST(ARG(fn_args, n_args - 1)));
        for (size_t i = 0; i < (n_args - 1); ++i) {
            concat = list_prepend(concat, ARG(fn_args, i));
        }
        fn_args = value_new_list(concat);
    }
    Value *tco_expr;
    Environment *tco_env;
    Value *result = apply(fn, fn_args, &tco_expr, &tco_env);
    /* need to call eval since apply defers to eval for TCO support */
    if (tco_expr && !exc_is_pending()) {
        result = eval(tco_expr, tco_env);
    }
    if (!result) {
        assert(exc_is_pending());
        return NULL;
    }
    return result;
}

Value *core_is_nil(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "NIL? takes exactly one argument");
    Value *expr = ARG(args, 0);
    return value_new_bool(is_nil(expr));
}

Value *core_is_true(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "TRUE? takes exactly one argument");
    Value *expr = ARG(args, 0);
    return value_new_bool(is_true(expr));
}

Value *core_is_false(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "FALSE? takes exactly one argument");
    Value *expr = ARG(args, 0);
    return value_new_bool(is_false(expr));
}

Value *core_is_symbol(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "SYMBOL? takes exactly one argument");
    Value *expr = ARG(args, 0);
    return value_new_bool(is_symbol(expr));
}

Value *core_symbol(const Value *args)
{
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "SYMBOL takes exactly one argument");
    Value *expr = ARG(args, 0);
    return value_new_symbol(STRING(expr));
}

Value *core_assert(const Value *args)
{
    CHECK_ARGLIST(args);
    size_t nargs = NARGS(args);
    if (nargs < 1 || nargs > 2) {
        exc_set(value_make_exception("Invalid argument list in core function: "
                                     "core_assert takes 1 or 2 arguments."));
        return NULL;
    }
    const Value *arg0 = ARG(args, 0);
    const Value *arg1 = NULL;
    if (nargs == 2) {
        arg1 = ARG(args, 1);
        REQUIRE_VALUE_TYPE(arg1, VALUE_STRING,
                           "Second argument to assert must be a string");
    }
    if (is_truthy(arg0)) {
        return VALUE_CONST_NIL;
    }
    if (nargs == 1) {
        exc_set(value_make_exception("Assert failed: %s is not true.",
                                     core_pr_str(arg0)->value.str));
    } else {
        exc_set(value_make_exception("Assert failed: %s", STRING(arg1)));
    }
    return NULL;
}

Value *core_throw(const Value *args)
{
    REQUIRE_LIST_CARDINALITY(args, 1ul, "THROW takes exactly one argument");
    Value *value = ARG(args, 0);
    exc_set(value); // FIXME: we expect .string to be valid...
    return NULL;
}

Value *core_nth(const Value *args)
{
    // (nth collection index)
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 2ul, "NTH takes exactly two arguments");
    Value *coll = ARG(args, 0);
    REQUIRE_VALUE_TYPE(coll, VALUE_LIST | VALUE_VECTOR, "First argument to nth must be a list or a vector");
    Value *pos = ARG(args, 1);
    REQUIRE_VALUE_TYPE(pos, VALUE_INT, "Second argument to nth must be an integer");
    if (INT(pos) < 0 || (unsigned) INT(pos) >= NARGS(coll)) {
        exc_set(value_make_exception("Index error"));
        return NULL;
    }
    if (is_list(coll)) {
        return list_nth(LIST(coll), INT(pos));
    } else {
        return vector_typed_at(VECTOR(coll), INT(pos), Value);
    }
    return NULL;
}

Value *core_first(const Value *args)
{
    // (first coll)
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "FIRST takes exactly one argument");
    const Value *coll = ARG(args, 0);
    if (is_nil(coll)) {
        return VALUE_CONST_NIL;
    }
    REQUIRE_VALUE_TYPE(coll, VALUE_LIST | VALUE_VECTOR, "Argument to FIRST must be a collection or NIL");
    if (is_list(coll)) {
        if (list_size(LIST(coll)) == 0) return VALUE_CONST_NIL;
        return ARG(coll, 0);
    } else {
        if (vector_size(VECTOR(coll)) == 0) return VALUE_CONST_NIL;
        return vector_typed_at(VECTOR(coll), 0, Value);
    }
}

Value *core_rest(const Value *args)
{
    // (rest coll)
    CHECK_ARGLIST(args);
    REQUIRE_LIST_CARDINALITY(args, 1ul, "REST takes exactly one argument");
    Value *coll = ARG(args, 0);
    if (is_nil(coll) || NARGS(coll) <= 1) {
        return value_new_list(NULL);
    }
    REQUIRE_VALUE_TYPE(coll, VALUE_LIST, "Argument to REST must be a collection or NIL");
    return value_new_list(list_tail(LIST(coll)));
}
