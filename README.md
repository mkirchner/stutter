![Build Status](https://github.com/mkirchner/stutter/workflows/C/C++%20CI/badge.svg)

Stutter - Lisp, from scratch, in C
==================================

![screenshot](doc/screenshot.png)

`stutter` is an educational Lisp implementation in C that follows a pretty strict
*do it from scratch* ideology. It is also a very elegant
form of a CS101 practical since it touches a wide array of topics: formal languages
(lexing, parsing, abstract syntax trees), metalinguistic evaluation
(eval/apply, macros), data structures (lists, trees, maps, arrays), hashing
functions, automatic memory management (mark & sweep garbage collection), all
wrapped into a thick educational, down-to-earth package.

I created this since *writing my own Lisp* was high on my bucket list. It has
been (and still is) an immensely satisfying fun ride... come along!

See [the tests](https://github.com/mkirchner/stutter/blob/867b6c1f5137cc16f0492003f270514cfcea29c7/test/test.stt#L1) to get an idea of what `stutter` already is capable of.

### Rules

1. Write everything from scratch.
2. Do not question the rules.

### Getting started

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

