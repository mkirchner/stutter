#ifndef __EVAL_UTILS_H__
#define __EVAL_UTILS_H__

#include <common.h>
#include "value.h"

bool has_cardinality(const Value *expr, const size_t cardinality);
bool is_self_evaluating(const Value *value);
bool is_variable(const Value *value);
bool is_list_that_starts_with(const Value *value, const char *what, size_t len);
bool is_quoted(const Value *value);
bool is_quasiquoted(const Value *value);
bool is_assignment(const Value *value);
bool is_definition(const Value *value);
bool is_macro_definition(const Value *value);
bool is_let(const Value *value);
bool is_lambda(const Value *value);
bool is_if(const Value *value);
bool is_do(const Value *value);
bool is_try(const Value *value);
bool is_macro_expansion(const Value *value);
bool is_application(const Value *value);

#endif /* !__EVAL_UTILS_H__ */
