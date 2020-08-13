#ifndef __VALUE_MAP_H__
#define __VALUE_MAP_H__

#include "vm/common.h"
#include "vm/object.h"
#include "vm/value.h"

/*
 * Hash table w/ open addressing and linear probing for conflict resolution.
 */

typedef struct {
   VmString *key;
   VmValue *value;
} KeyValuePair;

typedef struct {
    size_t size;
    size_t capacity;
    KeyValuePair* items;
} ValueMap;

ValueMap* value_map_new();
void value_map_delete(ValueMap* ptr);

void value_map_insert(ValueMap* map, const VmString *key, const VmValue *value);
const VmValue *value_map_get(const ValueMap* map, const VmString *key);


#endif /* !__VALUE_MAP_H__ */
