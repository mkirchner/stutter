# The Array

  - heap-based array
  - uses `char*` as base type (since C guarantees `sizeof(char)` == 1)
  - one (\!) `char*` pointer to contiguous memory
  - implements front- and back operations, front-aligned, hence
    back-operations are fast, front operations require shifting the
    memory around
  - concepts
      - difference between size & capacity
      - using `memcopy()` and `memmove()` (the latter for overlapping
        mem regions)
      - using `realloc()` for resizing
          - [dlmalloc implementation
            notes](http://gee.cs.oswego.edu/dl/html/malloc.html)
          - refer to how a memory allocator work
            [0](http://dmitrysoshnikov.com/compilers/writing-a-memory-allocator/),
            [1](http://gchandbook.org)
          - also the `brk`, `sbrk`, `mmap` calls
  - nifty things
      - using a macro to enable multiple return values in all
        `array_typed_*` functions
      - finding the next largest power of two: from [bit twiddling
        hacks](http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2).
