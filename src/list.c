#include "list.h"
#include "gc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


const List *list_new()
{
    // doubly-linked list, managed memory
    List *list = (List *) gc_malloc(&gc, sizeof(List));
    list->begin = list->end = NULL;
    list->size = 0;
    return list;
}

static ListItem *list_item_new(void *value)
{
    ListItem *item = (ListItem *) gc_calloc(&gc, 1, sizeof(ListItem));
    item->p = value;
    return item;
}

static ListItem *list_item_copy(const ListItem *li)
{
    ListItem *copy = (ListItem *) gc_calloc(&gc, 1, sizeof(ListItem));
    memcpy(copy, li, sizeof(ListItem));
    return copy;
}

static List *list_mutable_copy(const List *l)
{
    List *new_l = gc_calloc(&gc, 1, sizeof(List));
    void *head = NULL;
    ListItem *prev = NULL;
    while ((head = list_head(l)) != NULL) {
        ListItem *new_li = list_item_new(head);
        new_li->prev = prev;
        if (!prev) {
            new_l->begin = new_li;
            new_l->begin->prev = NULL;
        } else {
            new_li->prev->next = new_li;
        }
        new_l->size++;
        l = list_tail(l);
        prev = new_li;
    }
    new_l->end = prev;
    if (new_l->end) new_l->end->next = NULL;
    return new_l;
}

const List *list_dup(const List *l)
{
    return list_mutable_copy(l);
}

const List *list_conj(const List *l, void *value)
{
    List *nl = list_mutable_copy(l);
    ListItem *item = list_item_new(value);
    if (nl->size > 0) {
        nl->end->next = item;
        item->prev = nl->end;
        nl->end = item;
        item->next = NULL;
        nl->size++;
    } else {
        nl->begin = nl->end = item;
        item->prev = NULL;
        item->next = NULL;
        nl->size = 1;
    }
    return nl;
}

const List *list_cons(const List *l, void *value)
{
    List *nl = list_mutable_copy(l);
    ListItem *item = list_item_new(value);
    if (nl->size > 0) {
        item->next = nl->begin;
        nl->begin->prev = item;
        nl->begin = item;
        item->prev = NULL;
        nl->size++;
    } else {
        nl->begin = nl->end = item;
        item->prev = NULL;
        item->next = NULL;
        nl->size = 1;
    }
    return nl;
}

/**
 * Return the first element in a list.
 *
 * Rules:
 *   - The head of (a b ... c) is a
 *   - The head of (a) is a
 *   - The head of the empty list is nil (we're returning NULL)
 */
void *list_head(const List *l)
{
    assert(l && "Invalid argument: must pass a list instance");
    return (l && l->begin && l->begin->p) ? (void *) l->begin->p : NULL;
}

void *list_nth(const List *l, const size_t n)
{
    if (!(l->begin) || list_size(l) <= n) {
        return NULL;
    }
    const ListItem *cur = l->begin;
    for (size_t i = 0; i < n; ++i) {
        cur = cur->next;
        if (!cur) {
            return NULL;
        }
    }
    return (void *) cur->p;
}

/**
 * Returns the tail of a list.
 *
 * Rules:
 * - The tail of (a b ... c) is (b ... c)
 * - The tail of (a) is the empty list
 * - The tail of the empty list is the empty  list
 *
 * @param l A list instance.
 */
const List *list_tail(const List *l)
{
    assert(l && "Invalid argument: l must not be NULL");
    if (l) {
        // flat copy
        List *tail = (List *) gc_malloc(&gc, sizeof(List));
        if (l->size > 1) {
            tail->begin = l->begin->next;
            tail->end = l->end;
            tail->size = l->size - 1;
            return tail;
        } else {
            tail->begin = NULL;
            tail->end = NULL;
            tail->size = 0;
            return tail;
        }
    }
    return NULL;
}

size_t list_size(const List *l)
{
    return l->size;
}

bool list_is_empty(const List *l)
{
    return list_size(l) == 0;
}
