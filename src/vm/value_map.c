#include "vm/value_map.h"
#include "vm/mem.h"
#include "primes.h"
#include "vm/fastrange.h"

#define VALUE_MAP_MAX_LOAD 0.75

static VmString TOMBSTONE_KEY;
static KeyValuePair TOMBSTONE = {.key = &TOMBSTONE_KEY, .value = NULL};
#define IS_TOMBSTONE(item) ((item).key == &TOMBSTONE_KEY)
#define IS_EMPTY(item) ((item).key == NULL)

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
            if (item->key && !IS_TOMBSTONE(*item)) {
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
        // Ignore empty items and drop tombstones
        if (old_items[i].key != NULL && !IS_TOMBSTONE(old_items[i])) {
            value_map_put(map, old_items[i].key, old_items[i].value);
        }
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
 *   If the returned pointer has ptr->key == NULL or IS_TOMBSTONE(*ptr) is
 *   true, then ptr points to an
 *   empty field that *would* hold an entry with that `key`, else the
 *   key/value pair for `key` is actually stored at `ptr`.
 */
static KeyValuePair *value_map_find(const ValueMap *map, const VmString *key)
{
    if (map->capacity == 0) return NULL;  // short-circut for non-allocated map
    KeyValuePair *tombstone = NULL;
    KeyValuePair *slot = NULL;
    /* Use D. Lemire's fastrange; note that SIZE_MAX is broken on OSX, hence
     * hardcode the 32bit version. This is an attempt to get the best of both
     * worlds: prime-sized hash maps and fast [0, capacity) range wrapping. */
    size_t index = fastrange32(key->hash, map->capacity);
    while (1) {
        slot = &map->items[index];
        // If we hit an empty slot, return it unless we passed a
        // tombstone on the way
        if (IS_EMPTY(*slot)) {
            return tombstone ? tombstone : slot;
        }
        // Keep track of tombstones
        if (!tombstone && IS_TOMBSTONE(*slot)) {
            tombstone = slot;
        }
        // If we find a matching key, return the slot
        if (!IS_TOMBSTONE(*slot) && slot->key->hash == key->hash) {
            if (slot->key->len == key->len) {
                // FIXME: replace w/ interned strings and a pointer cmp at some point
                if (memcmp(slot->key->str, key->str, key->len) == 0) {
                    return slot;
                }
            }
        }
        // Move forward; as long as the load factor is < 1.0
        // we will eventually find an empty slot. See above comment on fastrange.
        index = fastrange32(index + 1, map->capacity);
    }
}

void value_map_put(ValueMap *map, const VmString *key, const VmValue *value)
{
    // Check if the map requires an up-size
    if (map->capacity * VALUE_MAP_MAX_LOAD < map->size + 1) {
        value_map_upsize(map);
    }
    KeyValuePair *slot = value_map_find(map, key);
    // Increase size of map only if we don't re-use a tombstone
    if (!IS_TOMBSTONE(*slot)) map->size++;
    // Allow re-assignemnt of existing keys, no questions asked.
    slot->key = obj_string_copy(key);
    slot->value = vm_value_copy(value);
}

const VmValue *value_map_get(const ValueMap *map, const VmString *key)
{
    KeyValuePair *p = value_map_find(map, key);
    if (p && p->key && !IS_TOMBSTONE(*p)) {
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

void value_map_remove(ValueMap *map, const VmString *key)
{
    KeyValuePair *p = value_map_find(map, key);
    if (p && p->key && !IS_TOMBSTONE(*p)) {
        obj_string_delete(p->key);
        vm_value_delete(p->value);
        *p = TOMBSTONE;
    }
}
