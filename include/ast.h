#ifndef __AST_H__
#define __AST_H__

#include <stdlib.h>
#include "location.h"

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
    AST_SEXPR_LIST,
    AST_SEXPR_ATOM,
    AST_SEXPR_QUOTE,
    AST_SEXPR_QUASIQUOTE,
    AST_SEXPR_UNQUOTE,
    AST_SEXPR_SPLICE_UNQUOTE,
    AST_LIST_COMPOUND,
    AST_LIST_EMPTY,
    AST_ATOM_SYMBOL,
    AST_ATOM_INT,
    AST_ATOM_FLOAT,
    AST_ATOM_STRING
} AstNodeType;

typedef struct {
    AstNodeType type;
    Location loc;
} AstNode;

typedef struct AstSexpr {
    AstNode node;
    union {
        struct AstList *list;
        struct AstAtom *atom;
        struct AstSexpr *quoted;
    } as;
} AstSexpr;

typedef struct AstList {
    AstNode node;
    union {
        struct {
            struct AstSexpr *sexpr;
            struct AstList *list;
        } compound;
    } as;
} AstList;

typedef struct AstAtom {
    AstNode node;
    union {
        char *symbol;
        int integer;
        double decimal;
        char *string;
    } as;
} AstAtom;

AstNode *ast_new_node(size_t size, AstNodeType node_type);
#define AST_NEW_NODE(ptr_type, node_type) (ptr_type*)ast_new_node(sizeof(node_type))

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

void ast_print(AstNode *ast);
void ast_print_atom(AstAtom *ast, int indent);
void ast_print_list(AstList *ast, int indent);
void ast_print_sexpr(AstSexpr *ast, int indent);

#endif /* !__AST_H__ */
