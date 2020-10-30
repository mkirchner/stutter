#include "lexer.h"

#include <stdlib.h>
#include <string.h>

const char *token_type_names[] = {
    "LEXER_TOK_ERROR",
    "LEXER_TOK_INT",
    "LEXER_TOK_FLOAT",
    "LEXER_TOK_STRING",
    "LEXER_TOK_SYMBOL",
    "LEXER_TOK_LPAREN",
    "LEXER_TOK_RPAREN",
    "LEXER_TOK_QUOTE",
    "LEXER_TOK_QUASIQUOTE",
    "LEXER_TOK_UNQUOTE",
    "LEXER_TOK_SPLICE_UNQUOTE",
    "LEXER_TOK_EOF"
};

static char *symbol_chars = "!&*+-0123456789<=>?@"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz";

Lexer *lexer_new(FILE *fp)
{
    Lexer *lexer = (Lexer *) malloc(sizeof(Lexer));
    *lexer = (Lexer) {
        .fp = fp,
        .state = LEXER_STATE_ZERO,
        .line_no = 1,
        .char_no = 0
    };
    return lexer;
}

void lexer_delete(Lexer *l)
{
    free(l);
}

void lexer_delete_token(LexerToken *t)
{
    switch(t->type) {
    case LEXER_TOK_INT:
    case LEXER_TOK_FLOAT:
    case LEXER_TOK_EOF:
        break;
    case LEXER_TOK_STRING:
    case LEXER_TOK_ERROR:
    case LEXER_TOK_SYMBOL:
    case LEXER_TOK_LPAREN:
    case LEXER_TOK_RPAREN:
    case LEXER_TOK_QUOTE:
    case LEXER_TOK_QUASIQUOTE:
    case LEXER_TOK_UNQUOTE:
    case LEXER_TOK_SPLICE_UNQUOTE:
        free(t->value.str);
        break;
    }
    free(t);
}

static LexerToken *lexer_make_token(const Lexer *l,
                                    const TokenType token_type,
                                    const char *buf)
{
    LexerToken *tok = (LexerToken *) malloc(sizeof(LexerToken));
    if (tok) {
        tok->type = token_type;
        tok->line = l->line_no;
        tok->column = l->char_no;
        switch(token_type) {
        case LEXER_TOK_INT:
            tok->value.int_ = atoi(buf);
            break;
        case LEXER_TOK_FLOAT:
            tok->value.double_ = atof(buf);
            break;
        case LEXER_TOK_STRING:
        case LEXER_TOK_ERROR:
        case LEXER_TOK_SYMBOL:
        case LEXER_TOK_LPAREN:
        case LEXER_TOK_RPAREN:
        case LEXER_TOK_QUOTE:
        case LEXER_TOK_QUASIQUOTE:
        case LEXER_TOK_UNQUOTE:
        case LEXER_TOK_SPLICE_UNQUOTE:
            tok->value.str = strdup(buf);
            break;
        case LEXER_TOK_EOF:
            tok->value.str = NULL;
            break;
        }
    }
    return tok;
}

static void lexer_advance_next_char(Lexer *l)
{
    l->char_no++;
}

static void lexer_advance_next_line(Lexer *l)
{
    l->line_no++;
    l->char_no = 0;
}

LexerToken *lexer_get_token(Lexer *l)
{
    char buf[1024] = {0};
    size_t bufpos = 0;
    int c;
    char *pos;
    while ((c = fgetc(l->fp)) != EOF) {
        lexer_advance_next_char(l);
        switch (l->state) {
        case LEXER_STATE_ZERO:
            switch(c) {
            case ';':
                l->state = LEXER_STATE_COMMENT;
                break;
            case '(':
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_LPAREN, buf);
                break;
            case ')':
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_RPAREN, buf);
                break;
            case '\'':
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_QUOTE, buf);
                break;
            case '`':
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_QUASIQUOTE, buf);
                break;
            /* start an unquote */
            case '~':
                buf[bufpos++] = c;
                l->state = LEXER_STATE_UNQUOTE;
                break;
            /* start a string */
            case '\"':
                /* don't put c in the buffer */
                l->state = LEXER_STATE_STRING;
                break;
            /* start  number */
            case '0' ... '9':
                buf[bufpos++] = c;
                l->state = LEXER_STATE_NUMBER;
                break;
            /* start a symbol */
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '+':
            case '/':
            case '*':
            case '<':
            case '=':
            case '>':
            case '&':
                buf[bufpos++] = c;
                l->state = LEXER_STATE_SYMBOL;
                break;
            case '-':
                buf[bufpos++] = c;
                l->state = LEXER_STATE_MINUS;
                break;
            /* eat whitespace */
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                lexer_advance_next_line(l);
                break;
            /* error */
            default:
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_ERROR, buf);
            }
            break;

        case LEXER_STATE_COMMENT:
            switch(c) {
            case '\n':
                lexer_advance_next_line(l);
                l->state = LEXER_STATE_ZERO;
                break;
            default:
                /* gobble up everything until EOL */
                break;
            }
            break;

        case LEXER_STATE_MINUS:
            /* This one is a little finicky since we want to allow for
             * symbols that start with a dash ("-main"), negative numbers
             * (-1, -2.4, -.7), and the subtraction operator (- 3 1). */
            switch(c) {
            case '0' ... '9':
                buf[bufpos++] = c;
                l->state = LEXER_STATE_NUMBER;
                break;
            case '.':
                buf[bufpos++] = c;
                l->state = LEXER_STATE_FLOAT;
                break;
            /* start a symbol */
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '+':
            case '/':
            case '*':
            case '<':
            case '=':
            case '>':
                ungetc(c, l->fp);
                l->char_no--;
                l->state = LEXER_STATE_SYMBOL;
                break;
            /* minus symbol */
            case ' ':
            case '\r':
            case '\t':
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_SYMBOL, buf);
                break;
            case '\n':
                lexer_advance_next_line(l);
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_SYMBOL, buf);
                break;
            /* error */
            default:
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_ERROR, buf);

            }
            break;
        case LEXER_STATE_UNQUOTE:
            l->state = LEXER_STATE_ZERO;
            if (c == '@') {
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_SPLICE_UNQUOTE, buf);
            } else {
                ungetc(c, l->fp);
                l->char_no--;
                return lexer_make_token(l, LEXER_TOK_UNQUOTE, buf);
            }
            break;

        case LEXER_STATE_STRING:
            if (c != '\"') {
                buf[bufpos++] = c;
                if (c == '\n') lexer_advance_next_line(l);
            } else {
                /* don't put c in the buffer */
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_STRING, buf);
            }
            break;

        case LEXER_STATE_NUMBER:
            switch(c) {
            case '(':
            case ')':
                ungetc(c, l->fp);
                l->char_no--;
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_INT, buf);
            case '\n':
                lexer_advance_next_line(l);
            case '\t':
            case '\r':
            case ' ':
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_INT, buf);
            case '.':
                buf[bufpos++] = c;
                l->state = LEXER_STATE_FLOAT;
                break;
            case '0' ... '9':
                buf[bufpos++] = c;
                break;
            default:
                /* error */
                buf[bufpos++] = c;
                return lexer_make_token(l, LEXER_TOK_ERROR, buf);
            }
            break;
        case LEXER_STATE_FLOAT:
            switch(c) {
            case '(':
            case ')':
                ungetc(c, l->fp);
                l->char_no--;
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_FLOAT, buf);
            case '\n':
                lexer_advance_next_line(l);
            case '\t':
            case '\r':
            case ' ':
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_FLOAT, buf);
            case '0' ... '9':
                buf[bufpos++] = c;
                break;
            default:
                /* error */
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_ERROR, buf);
            }
            break;
        case LEXER_STATE_SYMBOL:
            pos = strchr(symbol_chars, c);
            if (pos != NULL) {
                buf[bufpos++] = c;
            } else {
                ungetc(c, l->fp);
                l->char_no--;
                l->state = LEXER_STATE_ZERO;
                return lexer_make_token(l, LEXER_TOK_SYMBOL, buf);
            }
            break;
        default:
            buf[bufpos++] = c;
            return lexer_make_token(l, LEXER_TOK_ERROR, buf);
        }
    }
    /* acceptance states */
    switch(l->state) {
    case LEXER_STATE_ZERO:
    case LEXER_STATE_COMMENT:
        return lexer_make_token(l, LEXER_TOK_EOF, NULL);
    case LEXER_STATE_NUMBER:
        l->state = LEXER_STATE_ZERO;
        return lexer_make_token(l, LEXER_TOK_INT, buf);
    case LEXER_STATE_FLOAT:
        l->state = LEXER_STATE_ZERO;
        return lexer_make_token(l, LEXER_TOK_FLOAT, buf);
    case LEXER_STATE_SYMBOL:
        l->state = LEXER_STATE_ZERO;
        return lexer_make_token(l, LEXER_TOK_SYMBOL, buf);
    default:
        return lexer_make_token(l, LEXER_TOK_ERROR, buf);
    }
}

