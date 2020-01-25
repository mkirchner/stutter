Stutter - a LISP in C
=====================

### Getting started

Clone the repo and its submodules.

```bash
$ git clone --recursive git@github.com:mkirchner/stutter.git
$ cd stutter
$ make && make test
```

This should work on a Mac with a recent `clang`. No efforts to make it portable
(yet).

### Next steps

* Interpret/execute a file
* Nicer/more informative error handling and reporting
* Write tests in `stt`
* `try`/`catch` support
