/*
 * test_stutter.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "ast.h"
#include "djb2.h"
#include "lexer.h"

int tests_run = 0;

static char* type_names[] = {
    "ERROR", "INT", "FLOAT", "STRING", "SYMBOL",
    "LPAREN", "RPAREN", "QUOTE", "EOF"
};

static char* test_lexer()
{
    /* set up lexer to read from input file */
    FILE* input_fd = fopen("test/data/lexer_test.str", "r");
    mu_assert(input_fd!= NULL, "Failed to open lexer test file");
    lexer_t* lexer = lexer_new(input_fd);
    mu_assert(lexer != NULL, "Failed to create a lexer object");

    /* at the same time, we'll read the expected symbols
       from  the reference file */
    FILE* ref_fd = fopen("test/data/lexer_reference.txt", "r");
    mu_assert(ref_fd!= NULL, "Failed to open lexer test reference file");
    char *ref_line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    lexer_token_t* tok = lexer_get_token(lexer);
    linelen = getdelim(&ref_line, &linecap, ' ', ref_fd);
    while (tok != NULL && tok->type != LEXER_TOK_EOF && linelen > 0) {
        ref_line[linelen-1] = '\0';
        // printf("'%s' =?= '%s'\n", type_names[tok->type], ref_line);
        mu_assert(strcmp(type_names[tok->type], ref_line) == 0,
                  "Unexpected symbol");
        lexer_delete_token(tok);
        tok = lexer_get_token(lexer);
        linelen = getdelim(&ref_line, &linecap, ' ', ref_fd);
    }
    mu_assert(tok != NULL && tok->type == LEXER_TOK_EOF
              && linelen == -1, "Incorrect number of symbols");
    lexer_delete(lexer);
    fclose(ref_fd);
    fclose(input_fd);
    return 0;
}

static char* test_ast()
{
    // (add 5 7)
    char* add = malloc(4*sizeof(char));
    strcpy(add, "add");
    ast_sexpr_t* ast =
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

    mu_assert(strcmp(ast->ast.list
                     ->ast.compound.sexpr
                     ->ast.atom
                     ->value.symbol, "add") == 0, "Wrong symbol name");
    mu_assert(ast->ast.list
              ->ast.compound.list
              ->ast.compound.sexpr
              ->ast.atom
              ->value.int_ == 5, "Wrong LHS int");
    mu_assert(ast->ast.list
              ->ast.compound.list
              ->ast.compound.list
              ->ast.compound.sexpr
              ->ast.atom
              ->value.float_ == 7.0, "Wrong RHS float");
    // ast_print(ast);
    ast_delete_sexpr(ast);
    return 0;
}

static char* test_djb2()
{
    /* Basic testing for the djb2 hash: can we call it and
     * does it return a reasonable result?
     */
    unsigned long hash = djb2("");
    mu_assert(hash == 5381, "djb2 implementation error");
    hash = djb2("Hello World!");
    mu_assert(hash != 5381, "djb2 addition failure");
    return 0;
}

static char* test_suite()
{
    mu_run_test(test_ast);
    mu_run_test(test_lexer);
    mu_run_test(test_djb2);
    return 0;
}

int main()
{
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}

