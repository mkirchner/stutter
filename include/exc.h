#ifndef __EXC_H__
#define __EXC_H__

#include "value.h"

void exc_set(const Value *error);
const Value *exc_get();
void exc_clear();
bool exc_is_pending();

#endif /* !__EXC_H__ */
