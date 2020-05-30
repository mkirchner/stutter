#ifndef __EVAL_H__
#define __EVAL_H__

#include <env.h>
#include <value.h>

Value *eval(Value *expr, Environment *env);

#endif /* !EVAL_H */
