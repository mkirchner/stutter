/*
 * reader.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "reader.h"
#include "reader_stack.h"

#include <stdlib.h>
#include <string.h>
#include "log.h"


reader_t* reader_new(FILE* stream)
{
    lexer_t* lexer = lexer_new(stream);
    reader_t* reader = (reader_t*) malloc(sizeof(reader_t));
    *reader = (reader_t) {
        .lexer = lexer
    };
    return reader;
}

void reader_delete(reader_t* r)
{
    free(r->lexer);
    free(r);
}

ast_sexpr_t* reader_read(reader_t* reader)
{
    ast_sexpr_t* ast = NULL;
    reader_stack_t* stack = reader_stack_new(1024);
    reader_stack_token_t eof = { .type = T_EOF, .ast = {NULL} };
    reader_stack_token_t start = { .type = N_PROG, .ast = {NULL} };
    reader_stack_push(stack, eof);
    reader_stack_push(stack, start);
    lexer_token_t* tok;
    reader_stack_token_t tos;

    tok = lexer_get_token(reader->lexer);
    while (tok != NULL) {
        reader_stack_peek(stack, &tos);
        LOG_DEBUG("tos -> %s | tok -> %s",
                  reader_stack_token_type_names[tos.type],
                  token_type_names[tok->type]);
        if (tos.type == T_EOF && tok->type == LEXER_TOK_EOF) {
            LOG_DEBUG("%s", "Accepting EOF.");
            return ast;
        } else if (reader_is_terminal(tos) || tok->type == LEXER_TOK_EOF) {
            if (tos.type == T_LPAREN && tok->type == LEXER_TOK_LPAREN) {
                reader_stack_pop(stack, &tos);
            } else if (tos.type == T_RPAREN && tok->type == LEXER_TOK_RPAREN) {
                reader_stack_pop(stack, &tos);
            } else if (tos.type == T_QUOTE && tok->type == LEXER_TOK_QUOTE) {
                reader_stack_pop(stack, &tos);
            } else {
                // report error looking for tok at top of stack
                LOG_CRITICAL("Parse error stack/terminal mismatch (tok=%s, tos=%s)",
                             token_type_names[tok->type],
                             reader_stack_token_type_names[tos.type]);
                ast_delete_sexpr(ast);
                return NULL;
            }
        } else {
            // Non-terminals, do a leftmost derivation.
            /*
             * program ::= sexpr EOF
             * sexpr   ::= atom | LPAREN list RPAREN | QUOTE sexpr
             * list    ::= sexpr list | ∅
             * atom    ::= STRING | SYMBOL | INT | FLOAT
             */

            // atoms map 1:1 so just grab the data without explicitly creating the terminal
            if (tos.type == N_ATOM && tok->type == LEXER_TOK_INT) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->type = ATOM_INT;
                tos.ast.atom->value.int_ = *LEXER_TOKEN_VAL_AS_INT(tok);
                LOG_DEBUG("Rule: A->int (int=%d)", tos.ast.atom->value.int_);
            } else if (tos.type == N_ATOM && tok->type == LEXER_TOK_FLOAT) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->type = ATOM_FLOAT;
                tos.ast.atom->value.float_ = *LEXER_TOKEN_VAL_AS_FLOAT(tok);
                LOG_DEBUG("Rule: A->float (float=%.2f)", tos.ast.atom->value.float_);
            } else if (tos.type == N_ATOM && tok->type == LEXER_TOK_STRING) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->type = ATOM_STRING;
                tos.ast.atom->value.string = strdup(LEXER_TOKEN_VAL_AS_STR(tok));
                LOG_DEBUG("Rule: A->str (str=%s)", tos.ast.atom->value.string);
            } else if (tos.type == N_ATOM && tok->type == LEXER_TOK_SYMBOL) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->type = ATOM_SYMBOL;
                tos.ast.atom->value.string = strdup(LEXER_TOKEN_VAL_AS_STR(tok));
                LOG_DEBUG("Rule: A->sym (sym=%s)", tos.ast.atom->value.symbol);
            } else if (tos.type == N_LIST) {
                if (tok->type == LEXER_TOK_LPAREN ||
                        tok->type == LEXER_TOK_QUOTE ||
                        tok->type == LEXER_TOK_INT ||
                        tok->type == LEXER_TOK_FLOAT ||
                        tok->type == LEXER_TOK_STRING ||
                        tok->type == LEXER_TOK_SYMBOL) {
                    LOG_DEBUG("Rule: %s", "L->SL");
                    // pop current token from stack and create nodes in the AST
                    reader_stack_pop(stack, &tos);
                    tos.ast.list->type = LIST_COMPOUND;
                    tos.ast.list->ast.compound.list = ast_new_list();
                    tos.ast.list->ast.compound.sexpr = ast_new_sexpr();
                    // push rule RHS onto stack in reverse order
                    reader_stack_token_t token;
                    token.type = N_LIST;
                    token.ast.list = tos.ast.list->ast.compound.list;
                    reader_stack_push(stack, token);
                    token.type = N_SEXP;
                    token.ast.sexp = tos.ast.list->ast.compound.sexpr;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else if (tok->type == LEXER_TOK_RPAREN) {
                    reader_stack_pop(stack, &tos);
                    tos.ast.list->type = LIST_EMPTY;
                    continue; // do not advance token
                } else {
                    // parse error
                    LOG_CRITICAL("Parse error for rule L->SL|eps (tok=%s, tos=%s)",
                                 token_type_names[tok->type],
                                 reader_stack_token_type_names[tos.type]);
                    ast_delete_sexpr(ast);
                    lexer_delete_token(tok);
                    reader_stack_delete(stack);
                    return NULL;
                }
            } else if (tos.type == N_SEXP) {
                if (tok->type == LEXER_TOK_INT || tok->type == LEXER_TOK_FLOAT ||
                        tok->type == LEXER_TOK_STRING || tok->type == LEXER_TOK_SYMBOL) {
                    // S -> A
                    LOG_DEBUG("Rule: %s", "S->A");
                    // pop current token from stack and create nodes in the AST
                    reader_stack_pop(stack, &tos);
                    tos.ast.sexp->type = SEXPR_ATOM;
                    tos.ast.sexp->ast.atom = ast_new_atom();
                    reader_stack_token_t token;
                    token.type = N_ATOM;
                    token.ast.atom = tos.ast.sexp->ast.atom;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else if (tok->type == LEXER_TOK_LPAREN) {
                    // S -> ( L )
                    LOG_DEBUG("Rule: %s", "S->(L)");
                    // pop current token from stack and create nodes in the AST
                    reader_stack_pop(stack, &tos);
                    tos.ast.sexp->type = SEXPR_LIST;
                    tos.ast.sexp->ast.list = ast_new_list();
                    // push rule RHS onto stack in reverse order
                    reader_stack_token_t token;
                    token.type = T_RPAREN;
                    reader_stack_push(stack, token);
                    token.type = N_LIST;
                    token.ast.list = tos.ast.sexp->ast.list;
                    reader_stack_push(stack, token);
                    token.type = T_LPAREN;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else if (tok->type == LEXER_TOK_QUOTE) {
                    // S -> 'S
                    LOG_DEBUG("Rule: %s", "S->'S");
                    // pop current token from stack and create nodes in the AST
                    reader_stack_pop(stack, &tos);
                    tos.ast.sexp->type = SEXPR_QUOTE;
                    tos.ast.sexp->ast.quoted = ast_new_sexpr();
                    // push rule RHS onto stack in reverse order
                    reader_stack_token_t token;
                    token.type = N_SEXP;
                    token.ast.sexp = tos.ast.list->ast.compound.sexpr;
                    reader_stack_push(stack, token);
                    token.type = T_QUOTE;
                    token.ast.sexp = tos.ast.sexp->ast.quoted;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else {
                    // parse error
                    LOG_CRITICAL("Parse error for rule S->A|(L)|'S (tok=%s, tos=%s)",
                                 token_type_names[tok->type],
                                 reader_stack_token_type_names[tos.type]);
                    ast_delete_sexpr(ast);
                    lexer_delete_token(tok);
                    reader_stack_delete(stack);
                    return NULL;
                }
            } else if (tos.type == N_PROG) {
                // FIXME: deal with empty file
                if (tok->type == LEXER_TOK_EOF) {
                    // P -> $
                    LOG_DEBUG("Rule: %s", "P->$");
                    reader_stack_pop(stack, &tos);
                    continue; // do not advance token
                } else {
                    // P -> S$
                    LOG_DEBUG("Rule: %s", "P->S$");
                    reader_stack_pop(stack, &tos);
                    // create root of AST
                    ast = ast_new_sexpr();
                    reader_stack_token_t token;
                    token.type = N_SEXP;
                    token.ast.sexp = ast;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                }
            } else {
                // report error looking for tok at top of stack
                // FIXME: better error reporting
                LOG_CRITICAL("Could not find rule for token %s with %s at "
                             "top of stack.", token_type_names[tok->type],
                             reader_stack_token_type_names[tos.type]);
                ast_delete_sexpr(ast);
                return NULL;
            }
        }
        lexer_delete_token(tok);
        tok = lexer_get_token(reader->lexer);
    }
    lexer_delete_token(tok);
    reader_stack_delete(stack);
    return ast;
}

