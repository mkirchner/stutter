#include "vm/value_map.h"
#include "vm/mem.h"
#include "primes.h"
#include "vm/fastrange.h"

#define VALUE_MAP_MAX_LOAD 0.75

#define VALUE_MAP_IS_VALID_ITEM(x) (x.key != NULL)

ValueMap *value_map_new()
{
    ValueMap *vals = (ValueMap *) mem_reallocate(NULL, sizeof(ValueMap));
    *vals = (ValueMap) {
        .size = 0, .capacity = 0, .items = NULL
    };
    return vals;
}

void value_map_delete(ValueMap *ptr)
{
    if (ptr->items) {
        for (KeyValuePair *item = ptr->items;
                item < ptr->items + ptr->capacity; ++item) {
            if (item->key) {
                obj_string_delete(item->key);
                vm_value_delete(item->value);
            }
        }
    }
    mem_reallocate(ptr->items, 0);
    mem_reallocate(ptr, 0);
}

static void value_map_upsize(ValueMap *map)
{
    // up-size to next prime
    size_t old_capacity = map->capacity;
    KeyValuePair *old_items = map->items;
    map->capacity = map->capacity < 17 ? 17 : next_prime(map->capacity * 2);
    map->items = mem_reallocate(NULL, map->capacity * sizeof(VmValue));
    memset(map->items, 0, map->capacity * sizeof(VmValue)); // set to zero
    // re-insert existing items
    for (size_t i = 0; i < old_capacity; ++i) {
        if (old_items[i].key != NULL) value_map_insert(map, old_items[i].key, old_items[i].value);
    }
    // drop old items
    mem_reallocate(old_items, 0);
}

/*
 * Find the storage location for a specified key.
 *
 * @param map The `ValueMap`
 * @param key The key
 * @returns A pointer to a location inside `map->items`
 *
 * Note:
 *   If the returned pointer has ptr->key == NULL, then ptr points to an
 *   empty field that *would* hold an entry with that `key`, else the
 *   key/value pair for `key` is actually stored at `ptr`.
 */
static KeyValuePair *value_map_find(const ValueMap *map, const VmString *key)
{
    size_t index = fastrangesize(key->hash, map->capacity);
    while (1) {
        if (map->items[index].key == NULL || map->items[index].key->hash == key->hash) {
            return &map->items[index];
        }
        index = fastrangesize(index + 1, map->capacity);
    }
}

void value_map_put(ValueMap *map, const VmString *key, const VmValue *value)
{
    // Check if the map requires an up-size
    if (map->capacity * VALUE_MAP_MAX_LOAD < map->size + 1) {
        value_map_upsize(map);
    }
    KeyValuePair *slot = value_map_find(map, key);
    // Allow re-assignemnt of existing keys, no questions asked.
    slot->key = obj_string_copy(key);
    slot->value = vm_value_copy(value);
    map->size++;
}

const VmValue *value_map_get(const ValueMap *map, const VmString *key)
{
    KeyValuePair *p = value_map_find(map, key);
    if (p->key) {
        // Return the value if the key exists.
        return p->value;
    }
    // Otherwise signal non-existence with NULL
    return NULL;
}

size_t value_map_size(ValueMap *map)
{
    return map->size;
}

void value_map_remove(ValueMap* map, const VmString *key)
{
}
