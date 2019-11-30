/*
 * test_ir.c
 * Copyright (C) 2019 Marc Kirchner <Marc Kirchner@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "ir.h"

static char* test_ir()
{
    // (add 5 7.0)
    char* add = malloc(4 * sizeof(char));
    strcpy(add, "add");
    AstSexpr* ast =
        ast_sexpr_from_list(
            ast_list_from_compound_list(
                ast_sexpr_from_atom(
                    ast_atom_from_symbol(add)),
                ast_list_from_compound_list(
                    ast_sexpr_from_atom(
                        ast_atom_from_int(5)),
                    ast_list_from_compound_list(
                        ast_sexpr_from_atom(
                            ast_atom_from_float(7.0)),
                        ast_list_empty()))));
    value_print(ir_from_ast_sexpr(ast));
    printf("\n");

    // (add (quote 5) 7.0)
    AstSexpr* ast2 =
        ast_sexpr_from_list(
            ast_list_from_compound_list(
                ast_sexpr_from_atom(
                    ast_atom_from_symbol(add)),
                ast_list_from_compound_list(
                    ast_sexpr_from_quote(
                        ast_sexpr_from_atom(
                            ast_atom_from_int(5))),
                    ast_list_from_compound_list(
                        ast_sexpr_from_atom(
                            ast_atom_from_float(7.0)),
                        ast_list_empty()))));
    value_print(ir_from_ast_sexpr(ast2));
    printf("\n");
    return 0;
}
