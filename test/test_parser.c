#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "parser.h"
#include "../src/parser.c"

static char *test_parser()
{
    char *source[] = {
        "1",
        "\"a\"",
        "(fn 3 4 1)",
        "(lambda (a) (+ 1 a))",
        "(= 7 (let (b 12) (do (eval (read-string (def aa 7))) aa)))"
    };
    for (size_t k = 0; k < 4; ++k) {
        size_t n = strlen(source[k]);
        FILE *stream = fmemopen(source[k], n, "r");
        mu_assert(stream != NULL, "Failed to open lexer test file");

        Value *ast = NULL;
        ParseResult success = parser_parse(stream, &ast);
        mu_assert(success == PARSER_SUCCESS, "Failed to parse");
        // value_print(ast);
        // printf("\n");
    }
    return 0;
}

int tests_run = 0;

static char *test_suite()
{
    int bos;
    gc_start(&gc, &bos);
    mu_run_test(test_parser);
    gc_stop(&gc);
    return 0;
}

int main()
{
    printf("---=[ Parser tests\n");
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);
    return result != 0;
}
