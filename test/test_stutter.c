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
#include "env.h"
#include "ht.h"
#include "log.h"
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

static char* test_hashtable()
{
    ht_t* ht = ht_new(3);
    LOG_DEBUG("Capacity: %lu", ht->capacity);
    mu_assert(ht->capacity == 3, "Capacity sizing failure");
    ht_put(ht, "key", "value", strlen("value")+1);
    // set/get item
    char* value = (char*) ht_get(ht, "key");
    mu_assert(value != NULL, "Query must find inserted key");
    mu_assert(strcmp(value, "value") == 0, "Query must return inserted value");

    // update item
    ht_put(ht, "key", "other", strlen("other")+1);
    value = (char*) ht_get(ht, "key");
    mu_assert(value != NULL, "Query must find key");
    mu_assert(strcmp(value, "other") == 0, "Query must return updated value");

    // delete item
    ht_remove(ht, "key");
    value = (char*) ht_get(ht, "key");
    mu_assert(value == NULL, "Query must NOT find deleted key");

    ht_delete(ht);
    return 0;
}

static char* test_primes()
{
    /*
     * Test a few known cases.
     */
    mu_assert(!is_prime(0), "Prime test failure for 0");
    mu_assert(!is_prime(1), "Prime test failure for 1");
    mu_assert(is_prime(2), "Prime test failure for 2");
    mu_assert(is_prime(3), "Prime test failure for 3");
    mu_assert(!is_prime(12742382), "Prime test failure for 12742382");
    mu_assert(is_prime(611953), "Prime test failure for 611953");
    mu_assert(is_prime(479001599), "Prime test failure for 479001599");
    return 0;
}

static char* test_env()
{
    /*
     * creation
     */
    env_t* env0 = env_new(NULL);
    mu_assert(env_get(env0, "some_key") == NULL, "New env should be empty");
    /*
     * get/set
     */
    int value = 42;
    env_set(env0, "key1", &value);
    value_t* p = env_get(env0, "key1");
    mu_assert(42 == *(int*)p, "Value must not change");
    /*
     * nesting
     */
    env_t* env1 = env_new(env0);
    mu_assert(env1->parent == env0, "Failed to set parent");
    env_t* env2 = env_new(env1);
    mu_assert(env2->parent == env1, "Failed to set parent");
    p = env_get(env2, "key1");
    mu_assert(p != NULL, "Should find key in nested env");
    mu_assert(42 == *(int*)p, "Should return nested value");

    env_delete(env2);
    env_delete(env1);
    env_delete(env0);

    return 0;
}

static char* test_suite()
{
    mu_run_test(test_ast);
    mu_run_test(test_lexer);
    mu_run_test(test_djb2);
    mu_run_test(test_hashtable);
    mu_run_test(test_primes);
    mu_run_test(test_env);
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

