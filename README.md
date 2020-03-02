Stutter - a Lisp in C
=====================

`stutter` is an educational Lisp implementation in C that follows a strict
DIY and *do it from scratch* ideology. As such it is also a very elegant
form of a CS101 practical that touches a wide array of topics: formal languages
(lexing, parsing, abstract syntax trees), metalinguistic evaluation
(eval/apply, macros), data structures (lists, trees, maps, arrays), hashing
functions, automatic memory management (mark & sweep garbage collection), all
wrapped into a thick educational, down-to-earth package.

I created this since *writing my own Lisp* was high on my bucket list. It has
been (and still is) an immensely satisfying fun ride that made me significantly
more competent. Come along!


### Rules

1. Write everything from scratch.
2. Do not question the rules.

### Getting started

Clone the repo and its submodules.

```bash
$ git clone --recursive git@github.com:mkirchner/stutter.git
$ cd stutter
$ make && make test
```

This should work on a Mac with a recent `clang`. No efforts to make it portable
(yet).

### Screenshot

![screenshot](doc/screenshot.png)

### Next steps

- [ ] Create `stutter` test suite
  - [ ] implement `(assert ...)` or `(is ...)`
  - [ ] write tests
- [ ] Better error reporting
  - [ ] Surface lexer token line/col info in the reader
- [ ] Support `try/catch`
  - [ ] Concept that avoids global error state
  - [ ] Implement
- [ ] Language document
  - [ ] define core language
- [ ] Missing core functions
  - [ ] ... tbd ...
- [ ] Refactor AST/IR to a single representation (this is an artifact)
- [ ] Core capabilities
  - [ ] `keyword` support
  - [ ] `vector` support (`Array` C type is implemented but not surfaced)
  - [ ] `hash-map` support (`Map` C type is available but not surfaced)
- [ ] Create a type system

