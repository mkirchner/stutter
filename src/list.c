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


typedef struct ListItem {
    char* p;
    struct ListItem* prev;
    struct ListItem* next;
} ListItem;

List* list_new()
{
    // doubly-linked list
    List* list = (List*) gc_malloc(&gc, sizeof(List));
    list->begin = list->end = NULL;
    list->size = 0;
    return list;
}

void list_delete(List* l)
{
    if (l) {
        ListItem* i = l->begin;
        ListItem* j;
        while(i != l->end) {
            gc_free(&gc, i->p); // free the payload
            j = i->next; // save the ptr to next
            gc_free(&gc, i); // free current
            i = j; // move on
        }
    }
    gc_free(&gc, l);
}

static ListItem* list_new_item(void* value, size_t size)
{
    ListItem* item = (ListItem*) gc_malloc(&gc, sizeof(ListItem));
    item->p = (char *) gc_malloc(&gc, size);
    memcpy(item->p, value, size);
    return item;
}

void list_append(List* l, void* value, size_t size)
{
    ListItem* item = list_new_item(value, size);
    if (l->size > 0) {
        l->end->next = item;
        item->prev = l->end;
        l->end = item;
        item->next = NULL;
        l->size++;
    } else {
        l->begin = l->end = item;
        item->prev = NULL;
        item->next = NULL;
        l->size = 1;
    }
}

void list_prepend(List* l, void* value, size_t size)
{
    ListItem* item = list_new_item(value, size);
    if (l->size > 0) {
        item->next = l->begin;
        l->begin->prev = item;
        l->begin = item;
        item->prev = NULL;
        l->size++;
    } else {
        l->begin = l->end = item;
        item->prev = NULL;
        item->next = NULL;
        l->size = 1;
    }
}

void* list_head(List* l)
{
    return l->begin ? (void*) l->begin->p : NULL;
}

static ListItem* list_copy_item(ListItem* item)
{
    ListItem* copy = (ListItem*) gc_malloc(&gc, sizeof(ListItem));
    memcpy(copy, item, sizeof(ListItem));
    return copy;
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
