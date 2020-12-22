#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>
#include <stddef.h>

struct Value;

/**
 * A list item.
 *
 */
typedef struct ListItem {
    const struct Value *val; /**< pointer to an immutable `Value` instance */
    struct ListItem *next;   /**< pointer to the next list item */
} ListItem;

/**
 * A singly linked list of immutable values.
 *
 */
typedef struct List {
    struct ListItem *head; /**< pointer to the first item in the list */
    size_t size;           /**< size of the list */
} List;

/**
 * Create a new list.
 *
 * @return A new list instance.
 *
 */
const List *list_new();

/**
 * Duplicate an existing list.
 *
 * This creates a flat copy of the list; the val pointers in the copy
 * point to the same Value instances as the source.
 *
 * @param l The list to duplicate
 * @return A pointer to a copy of the list
 *
 */
const List *list_dup(const List *l);

/**
 * Return the first value in a list.
 *
 * Rules:
 *   - The head of (a b ... c) is a
 *   - The head of (a) is a
 *   - The head of the empty list is nil (we're returning NULL)
 *
 * @param l A list
 * @return A pointer to the value of the first item in the list or
 *         NULL if `l` is empty.
 */
const struct Value *list_head(const List *l);

/**
 * Returns the tail of a list.
 *
 * Rules:
 * - The tail of (a b ... c) is (b ... c)
 * - The tail of (a) is the empty list
 * - The tail of the empty list is the empty list
 *
 * @param l A list instance.
 * @return A pointer to the value instance at the head of the list
 */
const List *list_tail(const List *l);

/**
 * Return the n-th item in a list.
 *
 * Returns the n-th item in a list or NULL if the list is shorter than n.
 * This is an O(n) operation since we need to walk the list.
 *
 * @param l A list instance
 * @param n The index into the list
 * @return A pointer to value instance at the list index, or NULL
 *
 */
const struct Value *list_nth(const List *l, const size_t n);

/**
 * Insert a value at the beginning of the list.
 *
 * This is an O(1) operation.
 *
 * @param l A list
 * @param value The value to prepend
 * @return A copy of `l` with `value` prepended.
 *
 */
const List *list_prepend(const List *l, const struct Value *value);

/**
 * Append a value at the end of the list.
 *
 * This is an O(n) operation.
 *
 * @param l A list
 * @param value The value to append
 * @return A copy of `l` with `value` appended.
 *
 */
const List *list_append(const List *l, const struct Value *value);

/**
 * Return the size of a list.
 *
 * This is O(1) since we're keeping tabs on the list size.
 *
 * @param l A list
 * @return The size of the list `l`.
 *
 */
size_t list_size(const List *l);

/**
 * Test if a list is empty.
 *
 * @param l A list
 * @return True if the list `l` is empty, otherwise false.
 *
 */
bool list_is_empty(const List *l);

#endif /* !__LIST_H__ */
