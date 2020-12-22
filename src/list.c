#include "list.h"
#include "gc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/**
 * Create a new list item for a value.
 *
 * @param value The `struct Value` instance the items should point to
 * @return A pointer to a new `ListItem` instance
 *
 */
static ListItem *list_item_new(const struct Value *value)
{
    ListItem *item = (ListItem *) gc_calloc(&gc, 1, sizeof(ListItem));
    item->val = value;
    return item;
}

/**
 * Create a mutable copy of a list.
 *
* Creates a new, mutable linked list where each list item points to the same
* `struct Value` instance as the corresponding list item in `l`.
 *
 * @param l A pointer to the `List` instance to copy
 * @return A pointer to a mutable copy
 *
 */
static List *list_mutable_copy(const List *l)
{
    List *copy = gc_calloc(&gc, 1, sizeof(List));
    ListItem **q = &copy->head;
    ListItem *const *p = &l->head;
    while (*p) {
        *q = list_item_new((*p)->val);
        q = &(*q)->next;
        p = &(*p)->next;
    }
    copy->size = l->size;
    return copy;
}

const List *list_new()
{
    List *list = (List *) gc_calloc(&gc, 1, sizeof(List));
    return list;
}

const List *list_dup(const List *l)
{
    return list_mutable_copy(l);
}

const List *list_append(const List *l, const struct Value *value)
{
    // O(n) append at end of list
    List *copy = list_mutable_copy(l);
    ListItem **p = &copy->head;
    while (*p) {
        p = &(*p)->next;
    }
    *p = list_item_new(value);
    copy->size++;
    return copy;
}

const List *list_prepend(const List *l, const struct Value *value)
{
    // O(1) prepend at start of list
    List *copy = list_mutable_copy(l);
    ListItem *item = list_item_new(value);
    item->next = copy->head;
    copy->head = item;
    copy->size++;
    return copy;
}

const struct Value *list_head(const List *l)
{
    if (l && l->head) return l->head->val;
    return NULL;
}

const List *list_tail(const List *l)
{
    if (l) {
        // flat copy
        List *tail = (List *) gc_calloc(&gc, 1, sizeof(List));
        if (l->size > 1) {
            tail->head = l->head->next;
            tail->size = l->size - 1;
        }
        return tail;
    }
    return NULL;
}

const struct Value *list_nth(const List *l, const size_t n)
{
    ListItem *const *p = &l->head;
    size_t i = n;
    while (*p && i--) {
        p = &(*p)->next;
    }
    return *p ? (*p)->val : NULL;
}

size_t list_size(const List *l)
{
    return l->size;
}

bool list_is_empty(const List *l)
{
    return l->size == 0;
}
