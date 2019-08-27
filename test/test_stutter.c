/*
 * test_stutter.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "minunit.h"

#include "array.h"
#include "ast.h"
#include "djb2.h"
#include "env.h"
#include "gc.h"
#include "ir.h"
#include "map.h"
#include "list.h"
#include "log.h"
#include "lexer.h"
#include "primes.h"
#include "value.h"

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
    Lexer* lexer = lexer_new(input_fd);
    mu_assert(lexer != NULL, "Failed to create a lexer object");

    /* at the same time, we'll read the expected symbols
       from  the reference file */
    FILE* ref_fd = fopen("test/data/lexer_reference.txt", "r");
    mu_assert(ref_fd!= NULL, "Failed to open lexer test reference file");
    char *ref_line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    LexerToken* tok = lexer_get_token(lexer);
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
    Map* ht = map_new(3);
    LOG_DEBUG("Capacity: %lu", ht->capacity);
    mu_assert(ht->capacity == 3, "Capacity sizing failure");
    map_put(ht, "key", "value", strlen("value")+1);
    // set/get item
    char* value = (char*) map_get(ht, "key");
    mu_assert(value != NULL, "Query must find inserted key");
    mu_assert(strcmp(value, "value") == 0, "Query must return inserted value");

    // update item
    map_put(ht, "key", "other", strlen("other")+1);
    value = (char*) map_get(ht, "key");
    mu_assert(value != NULL, "Query must find key");
    mu_assert(strcmp(value, "other") == 0, "Query must return updated value");

    // delete item
    map_remove(ht, "key");
    value = (char*) map_get(ht, "key");
    mu_assert(value == NULL, "Query must NOT find deleted key");

    map_delete(ht);
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
    Environment* env0 = env_new(NULL);
    mu_assert(env_get(env0, "some_key") == NULL, "New env should be empty");
    /*
     * get/set
     */
    int value = 42;
    env_set(env0, "key1", &value);
    int* p = (int*) env_get(env0, "key1");
    mu_assert(42 == *p, "Value must not change");
    /*
     * nesting
     */
    Environment* env1 = env_new(env0);
    mu_assert(env1->parent == env0, "Failed to set parent");
    Environment* env2 = env_new(env1);
    mu_assert(env2->parent == env1, "Failed to set parent");
    p = (int*) env_get(env2, "key1");
    mu_assert(p != NULL, "Should find key in nested env");
    mu_assert(42 == *p, "Should return nested value");

    env_delete(env2);
    env_delete(env1);
    env_delete(env0);

    return 0;
}

static char* test_array()
{
    // creation
    Array* a = array_new(sizeof(int));
    mu_assert(a != NULL, "New array should not be NULL");

    // get/set
    int i = 42;
    array_push_back(a, &i, 1);
    mu_assert(array_size(a) == 1, "Size should be 1");
    mu_assert(array_capacity(a) == 1, "Capacity should be 1");
    i++;
    array_push_back(a, &i, 1);
    i++;
    array_push_back(a, &i, 1);
    i++;
    array_push_back(a, &i, 1);
    mu_assert(array_size(a) == 4, "Size should be 4");
    mu_assert(array_capacity(a) == 4, "Capacity should be 4");
    mu_assert(*array_typed_at(a, 0, int) == 42, "42 != 42");
    mu_assert(*array_typed_at(a, 1, int) == 43, "43 != 43 1");
    mu_assert(*array_typed_at(a, 2, int) == 44, "44 != 44");
    mu_assert(*array_typed_at(a, 3, int) == 45, "45 != 45");

    int i2[4] = {46, 47, 48, 49};
    array_push_back(a, i2, 4);
    mu_assert(array_size(a) == 8, "Size should be 8");
    mu_assert(array_capacity(a) == 8, "Capacity should be 8");
    mu_assert(*array_typed_at(a, 0, int) == 42, "42 != 42");
    mu_assert(*array_typed_at(a, 1, int) == 43, "43 != 43");
    mu_assert(*array_typed_at(a, 2, int) == 44, "44 != 44");
    mu_assert(*array_typed_at(a, 3, int) == 45, "45 != 45");
    mu_assert(*array_typed_at(a, 4, int) == 46, "46 != 46");
    mu_assert(*array_typed_at(a, 5, int) == 47, "47 != 47");
    mu_assert(*array_typed_at(a, 6, int) == 48, "48 != 48");
    mu_assert(*array_typed_at(a, 7, int) == 49, "49 != 49");

    int j = 41;
    array_push_front(a, &j, 1);
    mu_assert(*array_typed_at(a, 0, int) == 41, "push fail 41");
    j--;
    array_push_front(a, &j, 1);
    mu_assert(*array_typed_at(a, 0, int) == 40, "push fail 40");
    mu_assert(*array_typed_at(a, 1, int) == 41, "assert fail 41");
    j--;
    array_push_front(a, &j, 1);
    mu_assert(*array_typed_at(a, 0, int) == 39, "push fail 39");
    mu_assert(*array_typed_at(a, 1, int) == 40, "assert fail 40");
    mu_assert(*array_typed_at(a, 2, int) == 41, "assert fail 41");
    j--;
    array_push_front(a, &j, 1);
    mu_assert(*array_typed_at(a, 0, int) == 38, "push fail 38");
    mu_assert(*array_typed_at(a, 1, int) == 39, "assert fail 39");
    mu_assert(*array_typed_at(a, 2, int) == 40, "assert fail 40");
    mu_assert(*array_typed_at(a, 3, int) == 41, "assert fail 41");

    mu_assert(array_size(a) == 12, "Size should be 12");
    mu_assert(array_capacity(a) == 16, "Capacity should be 16");
    mu_assert(*array_typed_at(a, 0, int) == 38, "38 != 38");
    mu_assert(*array_typed_at(a, 1, int) == 39, "39 != 39");
    mu_assert(*array_typed_at(a, 2, int) == 40, "40 != 40");
    mu_assert(*array_typed_at(a, 3, int) == 41, "41 != 41");
    mu_assert(*array_typed_at(a, 4, int) == 42, "42 != 42");
    mu_assert(*array_typed_at(a, 5, int) == 43, "43 != 43");
    mu_assert(*array_typed_at(a, 6, int) == 44, "44 != 44");
    mu_assert(*array_typed_at(a, 7, int) == 45, "45 != 45");
    mu_assert(*array_typed_at(a, 8, int) == 46, "46 != 46");
    mu_assert(*array_typed_at(a, 9, int) == 47, "47 != 47");
    mu_assert(*array_typed_at(a, 10, int) == 48, "48 != 48");
    mu_assert(*array_typed_at(a, 11, int) == 49, "49 != 49");

    mu_assert(*array_typed_pop_back(a, int) == 49, "49 pop fail");
    mu_assert(array_size(a) == 11, "Wrong size after popping 49");
    for (int i = 10; i >= 0; --i) {
        array_typed_pop_back(a, int);
    }
    mu_assert(array_size(a) == 0, "Wrong size after emptying array");
    mu_assert(array_pop_back(a) == NULL, "Pop from empty array must return NULL");

    int i3[4] = {0, 1, 2, 3};
    array_push_back(a, i3, 4);
    mu_assert(array_size(a) == 4, "Wrong size after refill");
    mu_assert(*array_typed_pop_front(a, int) == 0, "Pop front 0 fail");
    mu_assert(array_size(a) == 3, "Pop front 0 size fail");
    mu_assert(*array_typed_pop_front(a, int) == 1, "Pop front 1 fail");
    mu_assert(*array_typed_pop_front(a, int) == 2, "Pop front 2 fail");
    mu_assert(*array_typed_pop_front(a, int) == 3, "Pop front 3 fail");
    mu_assert(array_typed_pop_front(a, int) == NULL, "Pop on empty array must return NULL");

    // clean up
    array_delete(a);
    return 0;
}

static char* test_list()
{
    int numbers[4] = {1, 2, 3, 4};

    List* l = list_new();
    mu_assert(list_size(l) == 0, "Empty list should have length 0");
    for (size_t i = 0; i < 4; ++i) {
        list_append(l, numbers + i, sizeof(int));
    }
    mu_assert(list_size(l) == 4, "Number  of appended elemets should be 4");
    mu_assert(*(int*)list_head(l) == 1, "First element should be 1");
    List* tail = list_tail(l);
    mu_assert(list_size(tail) == 3, "Tail should have size 3");
    mu_assert(*(int*)list_head(tail) == 2, "First element of tail should be 2");
    list_delete(l);

    l = list_new();
    for (size_t i = 0; i < 4; ++i) {
        list_prepend(l, numbers + i, sizeof(int));
    }
    mu_assert(list_size(l) == 4, "Number  of prepended elemets should be 4");
    mu_assert(*(int*)list_head(l) == 4, "First element should be 4");
    list_delete(l);

    l = list_new();
    mu_assert(list_head(l) == NULL, "Empty list should have a NULL head");
    mu_assert(list_size(list_tail(l))== 0, "Empty list should have an empty tail");
    list_append(l, numbers, sizeof(int));
    mu_assert(*(int*)list_head(l) == 1, "Head of one-element list should be 1");
    mu_assert(list_size(list_tail(l))== 0, "One-element list should have an empty tail");
    list_delete(l);

    return 0;
}

static char* test_ir()
{
    // (add 5 7.0)
    char* add = malloc(4*sizeof(char));
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

void dtor(void* ptr)
{
    ptr = NULL;
}

static char* test_gc()
{
   return NULL; 
}

static char* test_suite()
{
    mu_run_test(test_ast);
    mu_run_test(test_lexer);
    mu_run_test(test_djb2);
    mu_run_test(test_hashtable);
    mu_run_test(test_primes);
    mu_run_test(test_env);
    mu_run_test(test_array);
    mu_run_test(test_list);
    mu_run_test(test_ir);
    mu_run_test(test_gc);
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

