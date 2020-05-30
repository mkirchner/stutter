#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct ListItem {
    void *p;
    struct ListItem *prev;
    struct ListItem *next;
} ListItem;


typedef struct List {
    struct ListItem *begin;
    struct ListItem *end;
    size_t size;
} List;

const List *list_new();
const List *list_dup(const List *l);
void *list_head(const List *l);
void *list_nth(const List *l, const size_t n);
const List *list_tail(const List *l);
const List *list_conj(const List *l, void *value);
const List *list_cons(const List *l, void *value);
size_t list_size(const List *l);
bool list_is_empty(const List *l);

#endif /* !__LIST_H__ */
