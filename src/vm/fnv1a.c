#include "vm/fnv1a.h"

uint32_t fnv1a(const char *str, size_t len)
{
    /*
     * See https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function#FNV-1a_hash
     * for FNV-1a parameters
     */
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; ++i) {
        hash ^= str[i];
        hash *= 16777619;
    }
    return hash;
}
