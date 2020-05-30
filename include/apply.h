#ifndef __APPLY_H__
#define __APPLY_H__

#include <env.h>
#include <value.h>

Value *apply(Value *fn, Value *args, Value **tco_expr, Environment **tco_env);

#endif /* !APPLY_H */
