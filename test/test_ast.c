#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "../src/ast.c"

static char *test_ast()
{
    // (add 5 7)
    char *add = malloc(4 * sizeof(char));
    strcpy(add, "add");
    AstSexpr *ast =
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

    mu_assert(strcmp(ast->as.list
                     ->as.compound.sexpr
                     ->as.atom
                     ->as.symbol, "add") == 0, "Wrong symbol name");
    mu_assert(ast->as.list
              ->as.compound.list
              ->as.compound.sexpr
              ->as.atom
              ->as.integer == 5, "Wrong LHS int");
    mu_assert(ast->as.list
              ->as.compound.list
              ->as.compound.list
              ->as.compound.sexpr
              ->as.atom
              ->as.decimal == 7.0, "Wrong RHS float");
    // ast_print(ast);
    ast_delete_sexpr(ast);
    return 0;
}

int tests_run = 0;

static char *test_suite()
{
    mu_run_test(test_ast);
    return 0;
}

int main()
{
    printf("---=[ AST tests\n");
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
