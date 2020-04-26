![Build Status](https://github.com/mkirchner/stutter/workflows/C/C++%20CI/badge.svg)

Stutter - Lisp, from scratch, in C
==================================

![screenshot](doc/screenshot.png)

`stutter` is an educational Lisp interpreter implementation in C, written
entirely from scratch, not using any libraries (with the notable exception of
`editline` to maintain my sanity).

It is quite reasonable to think of `stutter` as a practical exercise that
touches a broad set of CS topics:

* formal languages (lexing, parsing, abstract syntax trees)
* metalinguistic evaluation (eval/apply, macros)
* data structures (lists, trees, maps, arrays)
* hashing functions (for O(1) maps)
* automatic memory management (mark & sweep garbage collection)

All this is wrapped into a thick educational, down-to-earth package that is
implemented in one of the most bare-bones, unforgiving languages out there:
C99.

`stutter` is a work in progress (and will be, for the forseeable future). See [the tests](test/lang/) to get an idea of what the language is already capable of.


Rationale
---------

`stutter` is my shot at the ultimate rite of passage in computer science:
writing your own lisp. It is hard to describe the Zen that flows from creating
a language as abstract as Lisp out of thin air, using the bare-bones
programming model that is C99.

Obviously, in modern our modern times, writing a Lisp interpreter is not
as challenging as it used to be since there are a lot of libraries that can
help us to achieve that goal. So, we need a few rules.


The Rules
---------

1. Write everything from scratch.
2. Do not question the rules.


Getting started
---------------

Clone the repo and its submodules (submodules because the garbarge collector is in a [separate repo](https://github.com/mkirchner/gc)).

```bash
$ git clone --recursive git@github.com:mkirchner/stutter.git
$ cd stutter
$ make && make test
```

This should work on a Mac with a recent `clang`. No efforts to make it portable
(yet).


### Next steps

- [ ] Create `stutter` test suite
  - [ ] write tests
- [ ] Support `try/catch`
  - [ ] Concept that avoids global error state
  - [ ] Implement
- [ ] Document core language
- [ ] Better error reporting
  - [ ] Surface lexer token line/col info in the reader
- [ ] Refactor AST/IR to a single representation (this is an artifact)
- [ ] Core capabilities
  - [ ] `keyword` support
  - [ ] `vector` support (`Array` C type is implemented but not surfaced)
  - [ ] `hash-map` support (`Map` C type is available but not surfaced)
- [ ] Add a type system

