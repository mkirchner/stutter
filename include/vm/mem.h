#ifndef __MEM_H__
#define __MEM_H__

#include <stdlib.h>
#include <string.h>

#include "log.h"

void *mem_reallocate(void *ptr, size_t size);
void *mem_copy(void *dst, const void *src, size_t n);

#endif /* !__MEM_H__ */
