#include "vm/mem.h"

/*
 * All-purpose reallocation abstratcion.
 *
 * Allocates memory if ptr == NULL, re-allocates for ptr != 0 and
 * frees, setting ptr = NULL, if size == 0.
 */
void* mem_reallocate(void *ptr, size_t size)
{
  if (!size) {
    free(ptr);
    ptr = NULL;
  } else {
      void* new_ptr = realloc(ptr, size);
      if (!new_ptr) {
          LOG_CRITICAL("Could not allocate %lu bytes of memory. Aborting.", size);
          exit(1);
      }
      ptr = new_ptr;
  }
  return ptr;
}

void* mem_copy(void* dst, const void* src, size_t n)
{
    return memcpy(dst, src, n);
}
