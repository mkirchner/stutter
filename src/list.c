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
// start snippet list-item-new
static ListItem *list_item_new(const struct Value *value)
{
    ListItem *item = (ListItem *) gc_calloc(&gc, 1, sizeof(ListItem));
    item->val = value;
    return item;
}
// end snippet list-item-new

/**
 * Create a mutable copy of a list.
 *
 * Creates a new, mutable linked list where each list item points to the same
 * `struct Value` instance as the corresponding list item in `l`.
 * Returns an empty list if `l` is NULL.
 *
 * @param l A pointer to the `List` instance to copy, or NULL
 * @return A pointer to a mutable copy or the empty list
 *
 */
// start snippet list-mutable-copy
static List *list_mutable_copy(const List *l)
{
    List *copy = gc_calloc(&gc, 1, sizeof(List));
    if (l) {
        ListItem **q = &copy->head;
        ListItem *const *p = &l->head;
        while (*p) {
            *q = list_item_new((*p)->val);
            q = &(*q)->next;
            p = &(*p)->next;
        }
        copy->size = l->size;
    }
    return copy;
}
// end snippet list-mutable-copy

// start snippet list-new
const List *list_new()
{
    return list_mutable_copy(NULL);
}
// end snippet list-new

// start snippet list-dup
const List *list_dup(const List *l)
{
    return list_mutable_copy(l);
}
// end snippet list-dup

// start snippet list-append
const List *list_append(const List *l, const struct Value *value)
{
    // O(n) append at end of list, no structural sharing
    List *copy = gc_calloc(&gc, 1, sizeof(List));
    if (l) {
        ListItem **q = &copy->head;
        ListItem *const *p = &l->head;
        while (*p) {
            *q = list_item_new((*p)->val);
            q = &(*q)->next;
            p = &(*p)->next;
        }
        *q = list_item_new(value);
        copy->size = l->size + 1;
    }
    return copy;
}
// end snippet list-append

// start snippet list-prepend
const List *list_prepend(const List *l, const struct Value *value)
{
    // O(1) prepend at start of list, with structural sharing
    List *new = list_mutable_copy(NULL);
    ListItem *item = list_item_new(value);
    new->head = item;
    item->next = l->head;
    new->size = l->size + 1;
    return new;
}
// end snippet list-prepend

// start snippet list-head
const struct Value *list_head(const List *l)
{
    if (l && l->head) return l->head->val;
    return NULL;
}
// end snippet list-head

// start snippet list-tail
const List *list_tail(const List *l)
{
    if (l) {
        // structural sharing
        List *tail = list_mutable_copy(NULL);
        if (l->size > 1) {
            tail->head = l->head->next;
            tail->size = l->size - 1;
        }
        return tail;
    }
    return NULL;
}
// end snippet list-tail

// start snippet list-nth
const struct Value *list_nth(const List *l, const size_t n)
{
    ListItem *const *p = &l->head;
    size_t i = n;
    while (*p && i--) {
        p = &(*p)->next;
    }
    return *p ? (*p)->val : NULL;
}
// end snippet list-nth

// start snippet list-size-empty
size_t list_size(const List *l)
{
    return l->size;
}

bool list_is_empty(const List *l)
{
    return l->size == 0;
}
// end snippet list-size-empty
