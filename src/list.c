/*
 * list.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "list.h"
#include "gc.h"

#include <stdlib.h>
#include <string.h>


List* list_new()
{
    // doubly-linked list, managed memory
    List* list = (List*) gc_malloc(&gc, sizeof(List));
    list->begin = list->end = NULL;
    list->size = 0;
    return list;
}

static ListItem* list_item_new(void* value)
{
    ListItem* item = (ListItem*) gc_calloc(&gc, 1, sizeof(ListItem));
    item->p = value;
    return item;
}

static ListItem* list_item_copy(const ListItem* li)
{
    ListItem* copy = (ListItem*) gc_calloc(&gc, 1, sizeof(ListItem));
    memcpy(copy, li, sizeof(ListItem));
    return copy;
}

List* list_copy(List* l)
{
    List* new_l = gc_calloc(&gc, 1, sizeof(List));
    void* head = NULL;
    ListItem* prev = NULL;
    while ((head = list_head(l)) != NULL) {
        ListItem* new_li = list_item_new(head);
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

List* list_append(List* l, void* value)
{
    List* nl = list_copy(l);
    ListItem* item = list_item_new(value);
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

List* list_prepend(List* l, void* value)
{
    List* nl = list_copy(l);
    ListItem* item = list_item_new(value);
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

void* list_head(List* l)
{
    return l->begin ? (void*) l->begin->p : NULL;
}

List* list_tail(List* l)
{
    // flat copy
    List* tail = (List*) gc_malloc(&gc, sizeof(List));
    if (l->size > 1) {
        tail->begin = l->begin->next;
        tail->end = l->end;
        tail->size = l->size - 1;
    } else {
        tail->begin = NULL;
        tail->end = NULL;
        tail->size = 0;
    }
    return tail;
}

size_t list_size(List* l)
{
    return l->size;
}

bool list_is_empty(List* l)
{
    return list_size(l) == 0;
}
