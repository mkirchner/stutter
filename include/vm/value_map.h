#ifndef __VALUE_MAP_H__
#define __VALUE_MAP_H__

#include "vm/common.h"
#include "vm/object.h"
#include "vm/value.h"

/*
 * Implements a hash map that maps `VmString` to `VmValue`.
 *
 * Uses open addressing with linear probing for conflict resolutions.
 * Item deletion creates tombstones that get cleared when the map
 * resizes. Map resizes are currently limited to upsizing once the
 * load factor reaches 0.75.
 */

typedef struct {
    VmString *key;
    VmValue *value;
} KeyValuePair;

typedef struct {
    size_t size;
    size_t capacity;
    KeyValuePair *items;
} ValueMap;

ValueMap *value_map_new();
void value_map_delete(ValueMap *ptr);

void value_map_put(ValueMap *map, const VmString *key, const VmValue *value);
const VmValue *value_map_get(const ValueMap *map, const VmString *key);
void value_map_remove(ValueMap *map, const VmString *key);


#endif /* !__VALUE_MAP_H__ */
