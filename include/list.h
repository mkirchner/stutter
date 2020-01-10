/*
 * list.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __LIST_H__
#define __LIST_H__

#include <stdbool.h>
#include <stddef.h>

struct ListItem;

typedef struct List {
    struct ListItem* begin;
    struct ListItem* end;
    size_t size;
} List;

List* list_new();
List* list_copy(List* l);
void* list_head(List* l);
List* list_tail(List* l);
List* list_append(List* l, void* value);
List* list_prepend(List* l, void* value);
size_t list_size(List* l);
bool list_is_empty(List* l);

#endif /* !__LIST_H__ */
