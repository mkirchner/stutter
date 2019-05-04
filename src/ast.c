/*
 * ast.c
 * Copyright (C) 2019 marc
 *
 * Distributed under terms of the MIT license.
 */

#include "ast.h"
#include <stdio.h>

ast_sexpr_t* ast_sexpr_from_list(ast_list_t* list)
{
    ast_sexpr_t* sexpr = malloc(sizeof(ast_sexpr_t));
    sexpr->type = SEXPR_LIST;
    sexpr->ast.list = list;
    return sexpr;
}

ast_sexpr_t* ast_sexpr_from_atom(ast_atom_t* atom)
{
    ast_sexpr_t* sexpr = malloc(sizeof(ast_sexpr_t));
    sexpr->type = SEXPR_ATOM;
    sexpr->ast.atom = atom;
    return sexpr;
}

ast_sexpr_t* ast_sexpr_from_quote(ast_sexpr_t* quoted)
{
    ast_sexpr_t* sexpr = malloc(sizeof(ast_sexpr_t));
    sexpr->type = SEXPR_QUOTE;
    sexpr->ast.quoted = quoted;
    return sexpr;
}

ast_list_t* ast_list_from_compound_list(ast_sexpr_t* s, ast_list_t* l)
{
    ast_list_t* list = malloc(sizeof(ast_list_t));
    list->type = LIST_COMPOUND;
    list->ast.compound.sexpr = s;
    list->ast.compound.list = l;
    return list;
}

ast_list_t* ast_list_empty()
{
    ast_list_t* list = malloc(sizeof(ast_list_t));
    list->type = LIST_EMPTY;
    return list;
}

ast_atom_t* ast_atom_from_symbol(char* symbol)
{
    ast_atom_t* atom = malloc(sizeof(ast_atom_t));
    atom->type = ATOM_SYMBOL;
    atom->value.symbol = symbol;
    return atom;
}

ast_atom_t* ast_atom_from_string(char* string)
{
    ast_atom_t* atom = malloc(sizeof(ast_atom_t));
    atom->type = ATOM_STRING;
    atom->value.string = string;
    return atom;
}

ast_atom_t* ast_atom_from_int(int number)
{
    ast_atom_t* atom = malloc(sizeof(ast_atom_t));
    atom->type = ATOM_INT;
    atom->value.int_ = number;
    return atom;
}

ast_atom_t* ast_atom_from_float(double number)
{
    ast_atom_t* atom = malloc(sizeof(ast_atom_t));
    atom->type = ATOM_FLOAT;
    atom->value.float_ = number;
    return atom;
}

void ast_delete_sexpr(ast_sexpr_t* s)
{
    switch(s->type) {
        case SEXPR_ATOM:
            ast_delete_atom(s->ast.atom);
            break;
        case SEXPR_LIST:
            ast_delete_list(s->ast.list);
            break;
        case SEXPR_QUOTE:
            ast_delete_sexpr(s->ast.quoted);
            break;
    }
    free(s);
}

void ast_delete_list(ast_list_t* l)
{
    switch(l->type) {
        case LIST_COMPOUND:
            ast_delete_sexpr(l->ast.compound.sexpr);
            ast_delete_list(l->ast.compound.list);
            break;
        case LIST_EMPTY:
            break;
    }
    free(l);
}

void ast_delete_atom(ast_atom_t* a)
{
    // atoms do not reserve memory for member but use const refs
    free(a);
}


void ast_print(ast_sexpr_t* ast)
{
    ast_print_sexpr(ast, 0);
}

void ast_print_sexpr(ast_sexpr_t* s, int indent)
{
    printf("%*s<sexpr>\n", indent, "");
    switch(s->type) {
        case SEXPR_ATOM:
            ast_print_atom(s->ast.atom, indent+2);
            break;
        case SEXPR_LIST:
            ast_print_list(s->ast.list, indent+2);
            break;
        case SEXPR_QUOTE:
            ast_print_sexpr(s->ast.quoted, indent+2);
            break;
    }
    printf("%*s</sexpr>\n", indent, "");
}

void ast_print_list(ast_list_t* l, int indent)
{
    printf("%*s<list>\n", indent, "");
    switch(l->type) {
        case LIST_COMPOUND:
            ast_print_sexpr(l->ast.compound.sexpr, indent+2);
            ast_print_list(l->ast.compound.list, indent+2);
            break;
        case LIST_EMPTY:
            break;
    }
    printf("%*s</list>\n", indent, "");
}

void ast_print_atom(ast_atom_t* a, int indent)
{
    switch(a->type) {
        case ATOM_INT:
            printf("%*s<int: %d>\n", indent, "", a->value.int_);
            break;
        case ATOM_FLOAT:
            printf("%*s<float: %.3g>\n", indent, "", a->value.float_);
            break;
        case ATOM_STRING:
            printf("%*s<str: %s>\n", indent, "", a->value.string);
            break;
        case ATOM_SYMBOL:
            printf("%*s<sym: %s>\n", indent, "", a->value.symbol);
            break;
    }
}
