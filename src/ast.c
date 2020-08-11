#include <stdio.h>
#include "ast.h"

AstNode* ast_new_node(size_t size, AstNodeType node_type)
{
    AstNode* node = malloc(size);
    node->type = node_type;
    return node;
}

AstSexpr *ast_new_sexpr()
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    return sexpr;
}

AstSexpr *ast_sexpr_from_list(AstList *list)
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    sexpr->node.type = AST_SEXPR_LIST;
    sexpr->as.list = list;
    return sexpr;
}

AstSexpr *ast_sexpr_from_atom(AstAtom *atom)
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    sexpr->node.type = AST_SEXPR_ATOM;
    sexpr->as.atom = atom;
    return sexpr;
}

static AstSexpr *ast_sexpr_from_anyquote(AstSexpr *quoted, AstNodeType t)
{
    AstSexpr *sexpr = malloc(sizeof(AstSexpr));
    sexpr->node.type = t;
    sexpr->as.quoted = quoted;
    return sexpr;
}

AstSexpr *ast_sexpr_from_quote(AstSexpr *quoted)
{
    return ast_sexpr_from_anyquote(quoted, AST_SEXPR_QUOTE);
}

AstSexpr *ast_sexpr_from_quasiquote(AstSexpr *quoted)
{
    return ast_sexpr_from_anyquote(quoted, AST_SEXPR_QUASIQUOTE);
}

AstList *ast_new_list()
{
    AstList *list = malloc(sizeof(AstList));
    list->node.type = AST_LIST_EMPTY; // default to empty list
    return list;
}

AstList *ast_list_from_compound_list(AstSexpr *s, AstList *l)
{
    AstList *list = malloc(sizeof(AstList));
    list->node.type = AST_LIST_COMPOUND;
    list->as.compound.sexpr = s;
    list->as.compound.list = l;
    return list;
}

AstList *ast_list_empty()
{
    AstList *list = malloc(sizeof(AstList));
    list->node.type = AST_LIST_EMPTY;
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
    atom->node.type = AST_ATOM_SYMBOL;
    atom->as.symbol = symbol;
    return atom;
}

AstAtom *ast_atom_from_string(char *string)
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    atom->node.type = AST_ATOM_STRING;
    atom->as.string = string;
    return atom;
}

AstAtom *ast_atom_from_int(int integer)
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    atom->node.type = AST_ATOM_INT;
    atom->as.integer = integer;
    return atom;
}

AstAtom *ast_atom_from_float(double number)
{
    AstAtom *atom = malloc(sizeof(AstAtom));
    atom->node.type = AST_ATOM_FLOAT;
    atom->as.decimal= number;
    return atom;
}

void ast_delete_node(AstNode* n)
{
    switch(n->type) {
        case AST_SEXPR_ATOM:
        case AST_SEXPR_LIST:
        case AST_SEXPR_QUOTE:
        case AST_SEXPR_QUASIQUOTE:
        case AST_SEXPR_SPLICE_UNQUOTE:
        case AST_SEXPR_UNQUOTE:
            ast_delete_sexpr((AstSexpr*) n);
            break;
        case AST_LIST_COMPOUND:
        case AST_LIST_EMPTY:
            ast_delete_list((AstList*) n);
            break;
        case AST_ATOM_SYMBOL:
        case AST_ATOM_INT:
        case AST_ATOM_FLOAT:
        case AST_ATOM_STRING:
            ast_delete_atom((AstAtom*) n);
            break;
    }
}

void ast_delete_sexpr(AstSexpr *s)
{
    if (s) {
        switch(s->node.type) {
        case AST_SEXPR_ATOM:
            ast_delete_atom(s->as.atom);
            break;
        case AST_SEXPR_LIST:
            ast_delete_list(s->as.list);
            break;
        case AST_SEXPR_QUOTE:
        case AST_SEXPR_QUASIQUOTE:
        case AST_SEXPR_SPLICE_UNQUOTE:
        case AST_SEXPR_UNQUOTE:
            ast_delete_sexpr(s->as.quoted);
            break;
        }
        free(s);
    }
}

void ast_delete_list(AstList *l)
{
    if (l) {
        switch(l->node.type) {
        case AST_LIST_COMPOUND:
            ast_delete_sexpr(l->as.compound.sexpr);
            ast_delete_list(l->as.compound.list);
            break;
        case AST_LIST_EMPTY:
            break;
        }
        free(l);
    }
}

void ast_delete_atom(AstAtom *a)
{
    // atoms do not reserve memory for member but use const refs
    if (a) {
        switch(a->node.type) {
        case AST_ATOM_SYMBOL:
            free(a->as.symbol);
            break;
        case AST_ATOM_STRING:
            free(a->as.string);
            break;
        default:
            break;
        }
        free(a);
    }
}


void ast_print(AstNode *ast)
{
    switch(ast->type) {
        case AST_SEXPR_ATOM:
        case AST_SEXPR_LIST:
        case AST_SEXPR_QUOTE:
        case AST_SEXPR_QUASIQUOTE:
        case AST_SEXPR_SPLICE_UNQUOTE:
        case AST_SEXPR_UNQUOTE:
            ast_print_sexpr((AstSexpr*) ast, 0);
            break;
        case AST_LIST_COMPOUND:
        case AST_LIST_EMPTY:
            ast_print_list((AstList*) ast, 0);
            break;
        case AST_ATOM_SYMBOL:
        case AST_ATOM_STRING:
        case AST_ATOM_INT:
        case AST_ATOM_FLOAT:
            ast_print_atom((AstAtom*) ast, 0);
            break;
    }
}

void ast_print_sexpr(AstSexpr *s, int indent)
{
    if (s) {
        printf("%*s<sexpr>\n", indent, "");
        switch(s->node.type) {
        case AST_SEXPR_ATOM:
            ast_print_atom(s->as.atom, indent + 2);
            break;
        case AST_SEXPR_LIST:
            ast_print_list(s->as.list, indent + 2);
            break;
        case AST_SEXPR_QUOTE:
        case AST_SEXPR_QUASIQUOTE:
        case AST_SEXPR_SPLICE_UNQUOTE:
        case AST_SEXPR_UNQUOTE:
            ast_print_sexpr(s->as.quoted, indent + 2);
            break;
        }
        printf("%*s</sexpr>\n", indent, "");
    }
}

void ast_print_list(AstList *l, int indent)
{
    if (l) {
        printf("%*s<list>\n", indent, "");
        switch(l->node.type) {
        case AST_LIST_COMPOUND:
            ast_print_sexpr(l->as.compound.sexpr, indent + 2);
            ast_print_list(l->as.compound.list, indent + 2);
            break;
        case AST_LIST_EMPTY:
            break;
        }
        printf("%*s</list>\n", indent, "");
    }
}

void ast_print_atom(AstAtom *a, int indent)
{
    if (a) {
        switch(a->node.type) {
        case AST_ATOM_INT:
            printf("%*s<int: %d>\n", indent, "", a->as.integer);
            break;
        case AST_ATOM_FLOAT:
            printf("%*s<float: %.3g>\n", indent, "", a->as.decimal);
            break;
        case AST_ATOM_STRING:
            printf("%*s<str: %s>\n", indent, "", a->as.string);
            break;
        case AST_ATOM_SYMBOL:
            printf("%*s<sym: %s>\n", indent, "", a->as.symbol);
            break;
        }
    }
}
