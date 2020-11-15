#include "parser.h"

#include "lexer.h"
#include "log.h"

/*
 * Lexer extension to allow peeking
 */

typedef struct {
    Lexer *lexer;
    LexerToken *cur_tok;
} TokenStream;


static TokenStream *tokenstream_new(Lexer *l)
{
    TokenStream *ts = (TokenStream*) malloc(sizeof(TokenStream));
    *ts = (TokenStream) { .lexer = l, .cur_tok = NULL };
    return ts;
}

static void tokenstream_delete(TokenStream *ts)
{
    if (ts) {
        lexer_delete_token(ts->cur_tok);
        free(ts);
    }
}

static LexerToken *tokenstream_peek(TokenStream *ts)
{
    if (!ts->cur_tok) {
        ts->cur_tok = lexer_get_token(ts->lexer);
    }
    return ts->cur_tok;
}

static LexerToken *tokenstream_get(TokenStream *ts)
{
    LexerToken *tok = ts->cur_tok ? ts->cur_tok : lexer_get_token(ts->lexer);
    ts->cur_tok = NULL;
    return tok;
}

static void tokenstream_consume(TokenStream *ts)
{
    if (ts->cur_tok) {
        lexer_delete_token(ts->cur_tok);
        ts->cur_tok = NULL;
    } else {
        LexerToken *tok = tokenstream_get(ts);
        lexer_delete_token(tok);
    }
}

/*
 * Parser
 */

/* forward declarations */
static ParseResult parser_parse_sexpr(TokenStream *ts, Value **ast);
static ParseResult parser_parse_list(TokenStream *ts, Value **ast);
static ParseResult parser_parse_atom(TokenStream *ts, Value **ast);
static ParseResult parser_parse_program(TokenStream *ts, Value **ast);

const char *QUOTES[] = { "quote", "quasiquote", "unquote", "splice-unquote" };


ParseResult parser_parse(FILE *stream, Value **ast)
{
    Lexer *lexer = lexer_new(stream);
    TokenStream *ts = tokenstream_new(lexer);
    ParseResult success = parser_parse_program(ts, ast);
    tokenstream_delete(ts);
    lexer_delete(lexer);
    return success;
}

static ParseResult parser_parse_program(TokenStream *ts, Value **ast)
{
    LexerToken *tok = tokenstream_peek(ts);
    if (!tok) {
        LOG_CRITICAL("Line %lu, column %lu: Unexpected lexer failure",
                     ts->lexer->line_no, ts->lexer->char_no);
        *ast = NULL;
        return PARSER_FAIL;
    }
    switch (tok->type) {
        case LEXER_TOK_ERROR: {
            LOG_CRITICAL("Line %lu, column %lu: L -> ? has parse error at \"%s\"",
                         ts->lexer->line_no, ts->lexer->char_no,
                         tok->as.str);
            *ast = NULL;
            return PARSER_FAIL;
        }
        case LEXER_TOK_EOF: {
            LOG_CRITICAL("Line %lu, column %lu: Unexpected EOF",
                         ts->lexer->line_no, ts->lexer->char_no);
            *ast = NULL;
            return PARSER_FAIL;
        }
        case LEXER_TOK_INT:
        case LEXER_TOK_FLOAT:
        case LEXER_TOK_STRING:
        case LEXER_TOK_SYMBOL:
        case LEXER_TOK_LPAREN:
        case LEXER_TOK_QUOTE:
        case LEXER_TOK_QUASIQUOTE:
        case LEXER_TOK_UNQUOTE:
        case LEXER_TOK_SPLICE_UNQUOTE: {
            LOG_DEBUG("Line %lu, column %lu: P -> L $", ts->lexer->line_no, ts->lexer->char_no);
            ParseResult success;
            Value *list = NULL;
            if ((success = parser_parse_list(ts, &list)) != PARSER_SUCCESS) {
                *ast = NULL;
                return PARSER_FAIL;
            }

            // consume eof
            tok = tokenstream_get(ts);
            if (!tok) {
                LOG_CRITICAL("Line %lu, column %lu: Unexpected lexer failure",
                             ts->lexer->line_no, ts->lexer->char_no);
                *ast = NULL;
                return PARSER_FAIL;
            }
            if (tok->type != LEXER_TOK_EOF) {
                LOG_CRITICAL("Line %lu, column %lu: Expected EOF, got: %s",
                             ts->lexer->line_no, ts->lexer->char_no,
                             token_type_names[tok->type]);
                lexer_delete_token(tok);
                *ast = NULL;
                return PARSER_FAIL;
            }
            lexer_delete_token(tok);
            *ast = list;
            return PARSER_SUCCESS;
        }
        default: {
            LOG_CRITICAL("Line %lu, column %lu: Unexpected token %s",
                         ts->lexer->line_no, ts->lexer->char_no,
                         token_type_names[tok->type]);
            return PARSER_FAIL;
         }
    }
    /* unreachable */
    LOG_CRITICAL("Reached unreachable code. X-(");
    return PARSER_FAIL;
}

static ParseResult parser_parse_list(TokenStream *ts, Value **ast)
{
    LexerToken *tok = tokenstream_peek(ts);
    if (!tok) {
        LOG_CRITICAL("Line %lu, column %lu: Unexpected lexer failure",
                     ts->lexer->line_no, ts->lexer->char_no);
        *ast = NULL;
        return PARSER_FAIL;
    }
    switch (tok->type) {
        case LEXER_TOK_ERROR: {
            LOG_CRITICAL("Line %lu, column %lu: L -> ? has parse error at \"%s\"",
                         ts->lexer->line_no, ts->lexer->char_no,
                         tok->as.str);
            *ast = NULL;
            return PARSER_FAIL;
        }
        case LEXER_TOK_EOF:
        case LEXER_TOK_RPAREN: {
            LOG_DEBUG("Line %lu, column %lu: L -> eps", ts->lexer->line_no, ts->lexer->char_no);
            *ast = NULL;
            return PARSER_SUCCESS;
        }
        case LEXER_TOK_INT:
        case LEXER_TOK_FLOAT:
        case LEXER_TOK_STRING:
        case LEXER_TOK_SYMBOL:
        case LEXER_TOK_LPAREN:
        case LEXER_TOK_QUOTE:
        case LEXER_TOK_QUASIQUOTE:
        case LEXER_TOK_UNQUOTE:
        case LEXER_TOK_SPLICE_UNQUOTE: {
            LOG_DEBUG("Line %lu, column %lu: L -> S L", ts->lexer->line_no, ts->lexer->char_no);
            Value *sexpr = NULL;
            ParseResult success;
            if ((success = parser_parse_sexpr(ts, &sexpr)) != PARSER_SUCCESS) {
                *ast = NULL;
                return PARSER_FAIL;
            }
            Value *list2 = NULL;
            if ((success = parser_parse_list(ts, &list2)) != PARSER_SUCCESS) {
                *ast = NULL;
                return PARSER_FAIL;
            }
            Value *list = value_make_list(sexpr);
            LIST(list) = list_conj(LIST(list), list2);
            *ast = list;
            return PARSER_SUCCESS;
        }
        default: {
            LOG_CRITICAL("Line %lu, column %lu: Unexpected token type for atom: %s",
                         ts->lexer->line_no, ts->lexer->char_no,
                         token_type_names[tok->type]);
            return PARSER_FAIL;
         }
    }
    /* unreachable */
    LOG_CRITICAL("Reached unreachable code. X-(");
    return PARSER_FAIL;
}

static ParseResult parser_parse_sexpr(TokenStream *ts, Value **ast)
{
    LexerToken *tok = tokenstream_peek(ts);
    if (!tok) {
        LOG_CRITICAL("Line %lu, column %lu: Unexpected lexer failure",
                     ts->lexer->line_no, ts->lexer->char_no);
        return PARSER_FAIL;
    }
    size_t q = 0;
    switch (tok->type) {
        /*
         * S -> ( L )
         */
        case LEXER_TOK_LPAREN: {
            tokenstream_consume(ts);
            Value *list = NULL;
            ParseResult success = parser_parse_list(ts, &list);
            if (success == PARSER_SUCCESS) {
                tokenstream_consume(ts);
                *ast = list;
                return PARSER_SUCCESS;
            }
            return PARSER_FAIL;
        }
        /*
         * S -> quote S
         *
         * Note that the order of labels matters here.
         */
        case LEXER_TOK_SPLICE_UNQUOTE:
            q++;
        case LEXER_TOK_UNQUOTE:
            q++;
        case LEXER_TOK_QUASIQUOTE:
            q++;
        case LEXER_TOK_QUOTE: {
            LOG_DEBUG("Line %lu, column %lu: S -> (quote S)", ts->lexer->line_no, ts->lexer->char_no);
            Value *quote = value_make_list(value_new_symbol(QUOTES[q]));
            Value *sexpr;
            tokenstream_consume(ts);
            if (parser_parse_sexpr(ts, &sexpr) == PARSER_SUCCESS) {
                LIST(quote) = list_conj(LIST(quote), sexpr);
                *ast = quote;
                return PARSER_SUCCESS;
            }
        }
        /*
         * S -> A
         */
        case LEXER_TOK_INT:
        case LEXER_TOK_FLOAT:
        case LEXER_TOK_STRING:
        case LEXER_TOK_SYMBOL:
            return parser_parse_atom(ts, ast);
        /*
         * failures and wrong tokens
         */
        case LEXER_TOK_EOF:
            LOG_CRITICAL("Line %lu, column %lu: Unexpected EOF",
                         ts->lexer->line_no, ts->lexer->char_no);
            return PARSER_FAIL;
        case LEXER_TOK_ERROR:
            LOG_CRITICAL("Line %lu, column %lu: Lexer error",
                         ts->lexer->line_no, ts->lexer->char_no);
            return PARSER_FAIL;
        default:
            LOG_CRITICAL("Line %lu, column %lu: Unexpected token type for atom: %s",
                         ts->lexer->line_no, ts->lexer->char_no,
                         token_type_names[tok->type]);
            return PARSER_FAIL;
    }
    return PARSER_SUCCESS;
}

static ParseResult parser_parse_atom(TokenStream *ts, Value **ast)
{
    LexerToken *tok = tokenstream_get(ts);
    if (!tok) {
        LOG_CRITICAL("Line %lu, column %lu: Unexpected lexer failure",
                     ts->lexer->line_no, ts->lexer->char_no);
        return PARSER_FAIL;
    }
    switch (tok->type) {
        case LEXER_TOK_INT:
            *ast = value_new_int(tok->as.int_);
            break;
        case LEXER_TOK_FLOAT:
            *ast = value_new_float(tok->as.double_);
            break;
        case LEXER_TOK_STRING:
            *ast = value_new_string(tok->as.str);
            break;
        case LEXER_TOK_SYMBOL:
            *ast = value_new_symbol(tok->as.str);
            break;
        case LEXER_TOK_EOF:
            LOG_CRITICAL("Line %lu, column %lu: Unexpected EOF",
                         ts->lexer->line_no, ts->lexer->char_no);
            lexer_delete_token(tok);
            return PARSER_FAIL;
        case LEXER_TOK_ERROR:
            LOG_CRITICAL("Line %lu, column %lu: Lexer error",
                         ts->lexer->line_no, ts->lexer->char_no);
            lexer_delete_token(tok);
            return PARSER_FAIL;
        default:
            LOG_CRITICAL("Line %lu, column %lu: Unexpected token type for atom: %s",
                         ts->lexer->line_no, ts->lexer->char_no,
                         token_type_names[tok->type]);
            lexer_delete_token(tok);
            return PARSER_FAIL;
    }
    lexer_delete_token(tok);
    return PARSER_SUCCESS;
}



