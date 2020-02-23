# Implementing the Garbage Collector

  - A garbage collector has two tasks: (1) keep track of all used
    memory; and (2) free unused memory once we run out of memory (or
    reach a pre-defined limit).

## Basic implementation strategy

  - Do not implement the `malloc()` part (e.g. using
    [`sbrk()`](http://man7.org/linux/man-pages/man2/sbrk.2.html) or
    [`mmap()`](http://man7.org/linux/man-pages/man2/mmap.2.html)) and
    managing chunk sizes.
  - Instead use `malloc()` itself but wrap it into a function than keeps
    track of allocated memory and its size.
  - Once the total size to allocated memory crosses a specified
    threshold, start garbage collection: identify and free all memory
    that is not in use anymore
  - In order to determine if memory is in use, we use a mark-and-sweep
    algorithm, as described in [1](http://gchandbook.org/).

## Adding housekeeping capabilities to `malloc()`

## Mark & Sweep

### Mark

### Sweep

## Putting it all together
