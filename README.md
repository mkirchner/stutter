Stutter - a LISP in C
=====================

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

### Open issues

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

