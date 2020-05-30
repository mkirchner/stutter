#include <stdio.h>
#include "ast.h"

AstSexpr *ast_new_sexpr()
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    return sexpr;
}

AstSexpr *ast_sexpr_from_list(AstList *list)
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    sexpr->type = SEXPR_LIST;
    sexpr->ast.list = list;
    return sexpr;
}

AstSexpr *ast_sexpr_from_atom(AstAtom *atom)
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    sexpr->type = SEXPR_ATOM;
    sexpr->ast.atom = atom;
    return sexpr;
}

static AstSexpr *ast_sexpr_from_anyquote(AstSexpr *quoted, AstSexprType t)
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    sexpr->type = t;
    sexpr->ast.quoted = quoted;
    return sexpr;
}

AstSexpr *ast_sexpr_from_quote(AstSexpr *quoted)
{
    return ast_sexpr_from_anyquote(quoted, SEXPR_QUOTE);
}

AstSexpr *ast_sexpr_from_quasiquote(AstSexpr *quoted)
{
    return ast_sexpr_from_anyquote(quoted, SEXPR_QUASIQUOTE);
}

AstList *ast_new_list()
{
    AstList *list = malloc(sizeof(AstList));
    list->type = LIST_EMPTY; // default to empty list
    return list;
}

AstList *ast_list_from_compound_list(AstSexpr *s, AstList *l)
{
    AstList *list = malloc(sizeof(AstList));
    list->type = LIST_COMPOUND;
    list->ast.compound.sexpr = s;
    list->ast.compound.list = l;
    return list;
}

AstList *ast_list_empty()
{
    AstList *list = malloc(sizeof(AstList));
    list->type = LIST_EMPTY;
    return list;
}

AstAtom *ast_new_atom()
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    return atom;
}

AstAtom *ast_atom_from_symbol(char *symbol)
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    atom->type = ATOM_SYMBOL;
    atom->value.symbol = symbol;
    return atom;
}

AstAtom *ast_atom_from_string(char *string)
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    atom->type = ATOM_STRING;
    atom->value.string = string;
    return atom;
}

AstAtom *ast_atom_from_int(int number)
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    atom->type = ATOM_INT;
    atom->value.int_ = number;
    return atom;
}

AstAtom *ast_atom_from_float(double number)
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    atom->type = ATOM_FLOAT;
    atom->value.float_ = number;
    return atom;
}

void ast_delete_sexpr(AstSexpr *s)
{
    if (s) {
        switch(s->type) {
        case SEXPR_ATOM:
            ast_delete_atom(s->ast.atom);
            break;
        case SEXPR_LIST:
            ast_delete_list(s->ast.list);
            break;
        case SEXPR_QUOTE:
        case SEXPR_QUASIQUOTE:
            ast_delete_sexpr(s->ast.quoted);
            break;
        }
        free(s);
    }
}

void ast_delete_list(AstList *l)
{
    if (l) {
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
}

void ast_delete_atom(AstAtom *a)
{
    // atoms do not reserve memory for member but use const refs
    if (a) {
        switch(a->type) {
        case ATOM_SYMBOL:
            free(a->value.symbol);
            break;
        case ATOM_STRING:
            free(a->value.string);
            break;
        default:
            break;
        }
        free(a);
    }
}


void ast_print(AstSexpr *ast)
{
    ast_print_sexpr(ast, 0);
}

void ast_print_sexpr(AstSexpr *s, int indent)
{
    if (s) {
        printf("%*s<sexpr>\n", indent, "");
        switch(s->type) {
        case SEXPR_ATOM:
            ast_print_atom(s->ast.atom, indent + 2);
            break;
        case SEXPR_LIST:
            ast_print_list(s->ast.list, indent + 2);
            break;
        case SEXPR_QUOTE:
        case SEXPR_QUASIQUOTE:
            ast_print_sexpr(s->ast.quoted, indent + 2);
            break;
        }
        printf("%*s</sexpr>\n", indent, "");
    }
}

void ast_print_list(AstList *l, int indent)
{
    if (l) {
        printf("%*s<list>\n", indent, "");
        switch(l->type) {
        case LIST_COMPOUND:
            ast_print_sexpr(l->ast.compound.sexpr, indent + 2);
            ast_print_list(l->ast.compound.list, indent + 2);
            break;
        case LIST_EMPTY:
            break;
        }
        printf("%*s</list>\n", indent, "");
    }
}

void ast_print_atom(AstAtom *a, int indent)
{
    if (a) {
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
}
