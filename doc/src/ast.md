# Representing Code

## Introduction

* frontend - AST - backend

### Abstract Syntax Trees

* central data structure for all stages of an interpreter or compiler: every
  stage of the application performs computations on the AST, rewrites the AST,
or creates another data structure before passing it along in the pipeline

* Parsers do not normally create parse trees. Instead, e.g. recursive descent
  parsers trace out the parse tree procedurally

* tree representation of the input stream that holds key tokens and grammatical
  relationships discovered during the parse.
* tree representation of the abstract syntax of the source code
* "abstract" vs. concrete means that a subset of the concrete language syntax (separators,
  grouping operators) are implicit to the tree and that some syntactic
  constructs (e.g. branching in conditional statements and loops) are represented
  using branch structures (e.g. if statement as condition-consequence-alternative)
* As such one level of abstraction above the parse tree (which is implicit in
  our implementation)
* Captures the essential structure of the input, omitting unnecessary syntactic
  details
* Result of the syntax analysis


### S-expressions

The syntatic elements of the Lisp language are called *symbolic expressions*,
or *s-expressions*. In Lisp, S-expressions are used to represent data and code
(a useful property called homoiconicity, see below). S-expressions may be either an *atom*
or a *list* of S-expressions:


    <sexpr> ::= (<list>) | <atom>
    <list>  ::= <sexpr> <list> | ⌀ 
    <atom>  ::= ... 

Atoms are numbers and symbols, e.g.

```
1.4142
42
hello-world
*earmuffs*
```

A *list* is a sequence of s-expressions, enclosed in parentheses, separated by
whitespace, or the *empty list*. For example:

```
(0 1 2 3)
(everest k2 makalu kanchenjunga)
(1 (2 3) (4 (5 (6 7))))
()
```

### Homoiconicity

A remarkable feature of Lisp is that s-expressions are used to represent
data (as shown above) but also to represent code, e.g.

```
(+ 1 2)
(sum 1 2 (- 5 2))
(f x y)
```

Here the Lisp interpreter interprets the first element of the list as the name of
a function and the remaining elements as the function arguments. In other
words, `(f x y)` translates to `f(x, y)` in traditional mathematical function
notation.


### Function application



The syntactic elements of the 

*S-expressions* () are a notation format invented for Lisp [mccarthy60],
and used to represent Lisp code and data (a useful property known as homoiconicity).

### Implementation idioms

* Use implementation language-specific idioms (i.e. C idioms in our case)
* Design tradeoff: should all constructs of the source language be represented
  by as a different node type or value-based differentiation using the same
  type, e.g. one type of binary operation type and distinguishing between the
  different operations using attributes such as "+" or "-" or having AST nodes of
  type `BinaryAdd`, `BinarySubtract`, etc
  

* Turns out, strictly imperative language like C provide the least support for
  convenient AST implementation
* General implementation: implement variant record using struct w/ enum and union; 
* Additional twist in C, two implementation options: one huge `union` vs. struct inheritance.

* Visitor pattern (double dispatch) as standard pattern to walk trees of
  class hierarchies is not available in C (it, obviously, can be implemented
  but solutions w/o built-in dynamic typing are not pretty)

* decouple AST representation/model from grammar changes
* Types: homogeneous ASTs, inhomogeneous
* design guidelines: dense (no unnecessary nodes), convenient (easy to walk),
  abstract over grammatical detail


[jones2003]: https://hillside.net/plop/plop2003/Papers/Jones-ImplementingASTs.pdf

## Design

* We will be using a *homogeneous AST* implementation
* That means that all nodes of the tree will all be of type `Value`.
* Depending on context, the context of a `Value` type need to be interpreted as
  boolean, integer, float, string, a list of `Value`s and the like. That
requires each node to track its own type information.


## Implementation

### Tagged values

We will use *tagged values* to explicitly track the type of a `Value`
instance. Tagging is implemented using an enum and a struct where the `enum`
holds the type information and a C `union` holds the data:

```c
typedef enum {
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_STRING
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        bool bool_;
        int int_;
        char *str;
    } as;
} Value;
```

Note that the `Value` struct contains the member `type` of type `ValueType`. In
order to determine the correct interpretation of the data in `as`, we check the
value of `type` and invoke the corresponding accessor function:

```c
struct Value *v = value_new_int(1337);
...
switch (v->type) {
case VALUE_TYPE_BOOL:
    print_bool(v);
    break;
case VALUE_TYPE_INT:
    print_int(v);
    break;
case VALUE_TYPE_STRING:
    print_string(v);
    break;
}
...




 The value will be
able to hold a few plain types (boolean, float, int) or a pointer to
heap-allocated dynamically sized types (string, List, Array, Map). For now, we
focus our attention on the plain types, List, and string.




# Parse Tree (later)

## Concept

* refer to grammar in parser

Implementation
* API
* Basic design
  * One type for every non-terminal and terminal

## Design: Modeling type hierarchies in C

* Why?
  * Polymorphism for processing AST nodes
  * We'll use the same pattern for all heap-based values later (?)

* Idea: base "class" with a type identifier allows type punning (i.e. casting
  to the right type).

C99 provides 3 guarantees: (1) members of a struct are laid out in memory in
the order in which they are declared; (2) pointers to a struct point to the
address of the first member of the struct; (3) The compiler may add unnamed
padding *but not at the beginning*.

### Struct inheritance

Had used it before but found enlightenment in [crafting_interpreters] who explains it ok.

Key observation: if we use a struct to define a *base type*, and we include
that base type as the first member of every derived type, we can access access

This allows us to define a "base class" like so:

```c
typedef struct Base {
    Type type;
    // potentially more base members
} Base;
```

Adding the base at the beginning of the definition of the "derived" class gives us a few desirable properties:

```c
typedef struct Derived {
    Base base;
    // more Derived fields here
} Derived;
```

Pointing to an instance of the `Derived` struct can be accomplished with a `Base*` pointer, downcasting would check the value of the `type` variable in the `Base` struct and then cast the `Base*` pointer to `Derived*`.

```
C99 6.7.2.1
1412 Within a structure object, the non-bit-field members and the units in
which bit-fields reside have addresses that increase in the order in which they
are declared.
1413 A pointer to a structure object, suitably converted, points to its initial
member (or if that member is a bit-field, then to the unit in which it
resides), and vice versa.
1414 There may be unnamed padding within a structure object, but not at its
beginning.
```

* The visitor pattern?


# Literature

[parr_10_language]: Terence Parr, "Language Implementation Patterns"
