#include "reader.h"
#include "reader_stack.h"

#include <stdlib.h>
#include <string.h>
#include "log.h"

#undef LOGLEVEL
#define LOGLEVEL LOGLEVEL_INFO

Reader *reader_new(FILE *stream)
{
    Lexer *lexer = lexer_new(stream);
    Reader *reader = (Reader *) malloc(sizeof(Reader));
    *reader = (Reader) {
        .lexer = lexer
    };
    return reader;
}

void reader_delete(Reader *r)
{
    free(r->lexer);
    free(r);
}

AstSexpr *reader_read(Reader *reader)
{
    AstSexpr *ast = NULL;
    ReaderStack *stack = reader_stack_new(1024);
    ReaderStackToken eof = { .type = T_EOF, .ast = {NULL} };
    ReaderStackToken start = { .type = N_PROG, .ast = {NULL} };
    reader_stack_push(stack, eof);
    reader_stack_push(stack, start);
    LexerToken *tok;
    ReaderStackToken tos;

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
            } else if (tos.type == T_QUASIQUOTE && tok->type == LEXER_TOK_QUASIQUOTE) {
                reader_stack_pop(stack, &tos);
            } else if (tos.type == T_UNQUOTE && tok->type == LEXER_TOK_UNQUOTE) {
                reader_stack_pop(stack, &tos);
            } else if (tos.type == T_SPLICE_UNQUOTE && tok->type == LEXER_TOK_SPLICE_UNQUOTE) {
                reader_stack_pop(stack, &tos);
            } else {
                // report error looking for tok at top of stack
                LOG_CRITICAL("Parse error at %lu:%lu: expected=%s, found=%s)",
                             tok->loc.line, tok->loc.col,
                             reader_stack_token_type_names[tos.type],
                             token_type_names[tok->type]);
                ast_delete_sexpr(ast);
                return NULL;
            }
        } else {
            // Non-terminals, do a leftmost derivation.
            /*
             * program ::= sexpr EOF
             * sexpr   ::= atom | LPAREN list RPAREN | [QUASI|UN|SPLICE_UN]QUOTE sexpr
             * list    ::= sexpr list | ∅
             * atom    ::= STRING | SYMBOL | INT | FLOAT
             */

            // atoms map 1:1 so just grab the data without explicitly creating the terminal
            if (tos.type == N_ATOM && tok->type == LEXER_TOK_INT) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->node.type = AST_ATOM_INT;
                tos.ast.atom->node.loc = tok->loc;
                tos.ast.atom->as.integer = LEXER_TOKEN_VAL_AS_INT(tok);
                LOG_DEBUG("Rule: A->int (int=%d)", tos.ast.atom->as.integer);
            } else if (tos.type == N_ATOM && tok->type == LEXER_TOK_FLOAT) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->node.type = AST_ATOM_FLOAT;
                tos.ast.atom->node.loc = tok->loc;
                tos.ast.atom->as.decimal = LEXER_TOKEN_VAL_AS_FLOAT(tok);
                LOG_DEBUG("Rule: A->float (float=%.2f)", tos.ast.atom->as.decimal);
            } else if (tos.type == N_ATOM && tok->type == LEXER_TOK_STRING) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->node.type = AST_ATOM_STRING;
                tos.ast.atom->node.loc = tok->loc;
                tos.ast.atom->as.string = strdup(LEXER_TOKEN_VAL_AS_STR(tok));
                LOG_DEBUG("Rule: A->str (str=%s)", tos.ast.atom->as.string);
            } else if (tos.type == N_ATOM && tok->type == LEXER_TOK_SYMBOL) {
                reader_stack_pop(stack, &tos);
                tos.ast.atom->node.type = AST_ATOM_SYMBOL;
                tos.ast.atom->node.loc = tok->loc;
                tos.ast.atom->as.string = strdup(LEXER_TOKEN_VAL_AS_STR(tok));
                LOG_DEBUG("Rule: A->sym (sym=%s)", tos.ast.atom->as.symbol);
            } else if (tos.type == N_LIST) {
                if (tok->type == LEXER_TOK_LPAREN ||
                        tok->type == LEXER_TOK_QUOTE ||
                        tok->type == LEXER_TOK_QUASIQUOTE ||
                        tok->type == LEXER_TOK_UNQUOTE ||
                        tok->type == LEXER_TOK_SPLICE_UNQUOTE ||
                        tok->type == LEXER_TOK_INT ||
                        tok->type == LEXER_TOK_FLOAT ||
                        tok->type == LEXER_TOK_STRING ||
                        tok->type == LEXER_TOK_SYMBOL) {
                    LOG_DEBUG("Rule: %s", "L->SL");
                    // pop current token from stack and create nodes in the AST
                    reader_stack_pop(stack, &tos);
                    tos.ast.list->node.type = AST_LIST_COMPOUND;
                    tos.ast.list->node.loc = tok->loc;
                    tos.ast.list->as.compound.list = ast_new_list();
                    tos.ast.list->as.compound.sexpr = ast_new_sexpr();
                    // push rule RHS onto stack in reverse order
                    ReaderStackToken token;
                    token.type = N_LIST;
                    token.ast.list = tos.ast.list->as.compound.list;
                    reader_stack_push(stack, token);
                    token.type = N_SEXP;
                    token.ast.sexp = tos.ast.list->as.compound.sexpr;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else if (tok->type == LEXER_TOK_RPAREN) {
                    reader_stack_pop(stack, &tos);
                    tos.ast.list->node.type = AST_LIST_EMPTY;
                    tos.ast.list->node.loc = tok->loc;
                    continue; // do not advance token
                } else {
                    // parse error
                    LOG_CRITICAL("Parse error at %lu:%lu: L->SL|eps expected=%s, found=%s)",
                                 tok->loc.line, tok->loc.col,
                                 reader_stack_token_type_names[tos.type],
                                 token_type_names[tok->type]);
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
                    tos.ast.sexp->node.type = AST_SEXPR_ATOM;
                    tos.ast.sexp->node.loc = tok->loc;
                    tos.ast.sexp->as.atom = ast_new_atom();
                    ReaderStackToken token;
                    token.type = N_ATOM;
                    token.ast.atom = tos.ast.sexp->as.atom;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else if (tok->type == LEXER_TOK_LPAREN) {
                    // S -> ( L )
                    LOG_DEBUG("Rule: %s", "S->(L)");
                    // pop current token from stack and create nodes in the AST
                    reader_stack_pop(stack, &tos);
                    tos.ast.sexp->node.type = AST_SEXPR_LIST;
                    tos.ast.sexp->node.loc = tok->loc;
                    tos.ast.sexp->as.list = ast_new_list();
                    // push rule RHS onto stack in reverse order
                    ReaderStackToken token;
                    token.type = T_RPAREN;
                    reader_stack_push(stack, token);
                    token.type = N_LIST;
                    token.ast.list = tos.ast.sexp->as.list;
                    reader_stack_push(stack, token);
                    token.type = T_LPAREN;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else if (tok->type == LEXER_TOK_QUOTE ||
                           tok->type == LEXER_TOK_QUASIQUOTE ||
                           tok->type == LEXER_TOK_UNQUOTE ||
                           tok->type == LEXER_TOK_SPLICE_UNQUOTE) {
                    // S -> 'S
                    LOG_DEBUG("Rule: %s", "S->['`~@]S"); // FIXME: add splice-unquote
                    // pop current token from stack and create nodes in the AST
                    reader_stack_pop(stack, &tos);
                    if (tok->type == LEXER_TOK_QUOTE) {
                        tos.ast.sexp->node.type = AST_SEXPR_QUOTE;
                    } else if (tok->type == LEXER_TOK_QUASIQUOTE) {
                        tos.ast.sexp->node.type = AST_SEXPR_QUASIQUOTE;
                    } else if (tok->type == LEXER_TOK_UNQUOTE) {
                        tos.ast.sexp->node.type = AST_SEXPR_UNQUOTE;
                    } else if (tok->type == LEXER_TOK_SPLICE_UNQUOTE) {
                        tos.ast.sexp->node.type = AST_SEXPR_SPLICE_UNQUOTE;
                    }
                    tos.ast.sexp->node.loc = tok->loc;
                    tos.ast.sexp->as.quoted = ast_new_sexpr();
                    // push rule RHS onto stack in reverse order
                    ReaderStackToken token;
                    token.type = N_SEXP;
                    token.ast.sexp = tos.ast.list->as.compound.sexpr;
                    reader_stack_push(stack, token);
                    if (tok->type == LEXER_TOK_QUOTE) {
                        token.type = T_QUOTE;
                    } else if (tok->type == LEXER_TOK_QUASIQUOTE) {
                        token.type = T_QUASIQUOTE;
                    } else if (tok->type == LEXER_TOK_UNQUOTE) {
                        token.type = T_UNQUOTE;
                    } else if (tok->type == LEXER_TOK_SPLICE_UNQUOTE) {
                        token.type = T_SPLICE_UNQUOTE;
                    }
                    token.ast.sexp = tos.ast.sexp->as.quoted;
                    reader_stack_push(stack, token);
                    continue; // do not advance token
                } else {
                    // parse error
                    LOG_CRITICAL("Parse error at %lu:%lu: S->A|(L)|'S expected=%s, found=%s)",
                                 tok->loc.line, tok->loc.col,
                                 reader_stack_token_type_names[tos.type],
                                 token_type_names[tok->type]);
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
                    ReaderStackToken token;
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
                LOG_CRITICAL("Parse error at %lu:%lu: could not find rule for %s with input %s)",
                             tok->loc.line, tok->loc.col,
                             reader_stack_token_type_names[tos.type],
                             token_type_names[tok->type]);
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

