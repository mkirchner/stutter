/*
 * ast.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __AST_H__
#define __AST_H__

#include <stdlib.h>

/*
 * stutter grammar
 *
 * program ::= sexpr EOF
 * sexpr   ::= atom | LPAREN list RPAREN | QUOTE sexpr
 * list    ::= sexpr list | ∅
 * atom    ::= STRING | SYMBOL | INT | FLOAT
 *
 */

struct ast_atom_t;
struct ast_list_t;

typedef struct ast_sexpr_t {
    enum {SEXPR_LIST, SEXPR_ATOM, SEXPR_QUOTE} type;
    union {
        struct ast_list_t* list;
        struct ast_atom_t* atom;
        struct ast_sexpr_t* quoted;
    } ast;
} ast_sexpr_t;

typedef struct ast_list_t {
    enum {LIST_COMPOUND, LIST_EMPTY} type;
    union {
        struct {
            struct ast_sexpr_t* sexpr;
            struct ast_list_t* list;
        } compound;
    } ast;
} ast_list_t;

typedef struct ast_atom_t {
    enum { ATOM_SYMBOL, ATOM_INT, ATOM_FLOAT, ATOM_STRING } type;
    union {
        char* symbol;
        int int_;
        double float_;
        char* string;
    } value;
} ast_atom_t;

ast_sexpr_t* ast_new_sexpr();
ast_sexpr_t* ast_sexpr_from_list(ast_list_t* list);
ast_sexpr_t* ast_sexpr_from_atom(ast_atom_t* atom);
ast_sexpr_t* ast_sexpr_from_quote(ast_sexpr_t* quoted);
ast_list_t* ast_new_list();
ast_list_t* ast_list_from_compound_list(ast_sexpr_t* s, ast_list_t* l);
ast_list_t* ast_list_empty();
ast_atom_t* ast_new_atom();
ast_atom_t* ast_atom_from_symbol(char* symbol);
ast_atom_t* ast_atom_from_string(char* string);
ast_atom_t* ast_atom_from_int(int number);
ast_atom_t* ast_atom_from_float(double number);

void ast_delete_sexpr(ast_sexpr_t* s);
void ast_delete_list(ast_list_t* l);
void ast_delete_atom(ast_atom_t* a);

void ast_print(ast_sexpr_t* ast);
void ast_print_atom(ast_atom_t* ast, int indent);
void ast_print_list(ast_list_t* ast, int indent);
void ast_print_sexpr(ast_sexpr_t* ast, int indent);

#endif /* !__AST_H__ */
