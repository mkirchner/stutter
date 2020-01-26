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

struct AstAtom;
struct AstList;

typedef enum {
    SEXPR_LIST,
    SEXPR_ATOM,
    SEXPR_QUOTE,
    SEXPR_QUASIQUOTE,
    SEXPR_UNQUOTE,
    SEXPR_SPLICE_UNQUOTE
} AstSexprType;

typedef struct AstSexpr {
    AstSexprType type;
    union {
        struct AstList *list;
        struct AstAtom *atom;
        struct AstSexpr *quoted;
        struct AstSexpr *unquoted;
    } ast;
} AstSexpr;

typedef struct AstList {
    enum {LIST_COMPOUND, LIST_EMPTY} type;
    union {
        struct {
            struct AstSexpr *sexpr;
            struct AstList *list;
        } compound;
    } ast;
} AstList;

typedef struct AstAtom {
    enum { ATOM_SYMBOL, ATOM_INT, ATOM_FLOAT, ATOM_STRING } type;
    union {
        char *symbol;
        int int_;
        double float_;
        char *string;
    } value;
} AstAtom;

AstSexpr *ast_new_sexpr();
AstSexpr *ast_sexpr_from_list(AstList *list);
AstSexpr *ast_sexpr_from_atom(AstAtom *atom);
AstSexpr *ast_sexpr_from_quote(AstSexpr *quoted);
AstSexpr *ast_sexpr_from_quasiquote(AstSexpr *quoted);
AstList *ast_new_list();
AstList *ast_list_from_compound_list(AstSexpr *s, AstList *l);
AstList *ast_list_empty();
AstAtom *ast_new_atom();
AstAtom *ast_atom_from_symbol(char *symbol);
AstAtom *ast_atom_from_string(char *string);
AstAtom *ast_atom_from_int(int number);
AstAtom *ast_atom_from_float(double number);

void ast_delete_sexpr(AstSexpr *s);
void ast_delete_list(AstList *l);
void ast_delete_atom(AstAtom *a);

void ast_print(AstSexpr *ast);
void ast_print_atom(AstAtom *ast, int indent);
void ast_print_list(AstList *ast, int indent);
void ast_print_sexpr(AstSexpr *ast, int indent);

#endif /* !__AST_H__ */
