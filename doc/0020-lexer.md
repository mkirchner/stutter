The Lexer
=========

* takes a stream of characters and converts them into a stream of labeled
  tokens
  * e.g. 3 * ( 4 + 5.2 ) -> INT(3) MUL LPAREN INT(4) OP(+) FLOAT(5.2) RPAREN
* Enables the parser to reason at the token level instead of the parser level
* Look at the grammar in order to determine the set of tokens we need to
  support


Terminals
---------

* The set of tokens created by the lexer must eventually form the terminals of
  the grammar we attempt to parse

| Token           | Characters                              |
|-----------------|-----------------------------------------|
| EOF             |                                         |
| LPAREN          | `(`                                     |
| RPAREN          | `)`                                     |
| QUASIQUOTE      | `\``                                    |
| UNQUOTE         | `~`                                     |
| SPLICE_UNQUOTE  | `~@`                                    |
| STRING          | `"[^"]"`                                |
| SYMBOL          | `[a-zA-Z+*-/<>=][0-9a-zA-Z!&*+-<=>?@]*` |
| INT             | `[0-9]+`                                |
| FLOAT           | `[0-9]+`                                |


Using an FSM for the lexer
--------------------------

![lexer state diagram](./lexer/fsm.png)


Design / Concept
----------------

* model as a finite state machine (a Mealey, to be precise)
* nested `case` statements in a loop
* loop keeps reading fp until EOF
* outer nesting: FSM states
* inner nesting: lookahead token (i.e. the input into the FSM)
* show example code


Interface/API
-------------

```c
/* object lifecycle */
Lexer *lexer_new(FILE *fp);
void lexer_delete(Lexer *l);

/* interface */
LexerToken *lexer_get_token(Lexer *l);
void lexer_delete_token(LexerToken *tok);
```


Implementation
--------------

### The lexer struct

A lexer instance needs to keep track of (1) the file (pointer) it is operating
on; (2) it position within that file (in terms of line & characer pos); and (3)
it's internal state to enable stateful (read: finite state machine) parsing.

The implementation is

```c
typedef struct {
    FILE *fp;
    LexerState state;
    size_t line_no;
    size_t char_no;
} Lexer;
```

and we define `LexerState` as an `enum` with

```c
typedef enum {
    LEXER_STATE_ZERO,
    LEXER_STATE_COMMENT,
    LEXER_STATE_NUMBER,
    LEXER_STATE_FLOAT,
    LEXER_STATE_SYMBOL,
    LEXER_STATE_STRING,
    LEXER_STATE_UNQUOTE,
    LEXER_STATE_MINUS
} LexerState;
```

Implementing the lexer lifecycle is simple: a `Lexer` instance requires a
constructor that takes a file pointer and a trivial destructor that deletes the
allocated memory:

```c
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
```


### Lexer tokens

We implement a lexer token as a *tagged union* (also called a *variant*).
Different token types must store different types of data (e.g. a string token will
collect character data, whereas the integer and float tokens will hold integer
and double precision numbers, respectively). A tagged union consists of a type
indicator and a union field. At any point in time, the union only holds one of
its values, either a string, an integer, or a float, as indicated by the value
of the type variable.

```c
typedef struct {
    TokenType type;
    union {
        char* str;
        int int_;
        double double_;
    } value;
} LexerToken;
```

We also add convenience macros for easy access

```c
#define LEXER_TOKEN_VAL_AS_STR(t) (t->value.str)
#define LEXER_TOKEN_VAL_AS_INT(t) (t->value.int_)
#define LEXER_TOKEN_VAL_AS_FLOAT(t) (t->value.double_)
```

and model the `TokenType` as a typdef'd enum:

```c
typedef enum {
    LEXER_TOK_ERROR,
    LEXER_TOK_INT,
    LEXER_TOK_FLOAT,
    LEXER_TOK_STRING,
    LEXER_TOK_SYMBOL,
    LEXER_TOK_LPAREN,
    LEXER_TOK_RPAREN,
    LEXER_TOK_QUOTE,
    LEXER_TOK_QUASIQUOTE,
    LEXER_TOK_UNQUOTE,
    LEXER_TOK_SPLICE_UNQUOTE,
    LEXER_TOK_EOF
} TokenType;
```

In order to simplify printing the token names we also declare an array of
strings that will hold the token names:

```c
extern const char *token_type_names[];
```

Creating a new instance of a lexer token is slightly more involved: we pass the
desired `TokenType` and a character buffer (that has beed read from the file
pointer of the `Lexer` object). For the token types that require a string, we
simply duplicate the buffer to the `char *` pointer. If we require an integer
or a float type, we fall back on the `atoi()` and `atof()` functions,
respectively:

```c
static LexerToken *lexer_make_token(TokenType token_type, char *buf)
{
    LexerToken *tok = (LexerToken *) malloc(sizeof(LexerToken));
    if (tok) {
        tok->type = token_type;
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
```

Deleting a token is equally simple: if the tagged union holds a token type for
which we allocated a string buffer, we clean the allocation before freeing the
token itself:

```c
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
```
