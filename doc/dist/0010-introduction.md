# Introduction

## Why?

“It seems to me that there have been two really clean, consistent models
of programming so far: the C model and the Lisp model. These two seem
points of high ground, with swampy lowlands between them.” – P. Graham

Invented in 1959 and published in 1960, LISP is the second oldest
programming language (one year after FORTRAN) and predates C by 12
years.

C: \* Very little abstraction between the system and the programmer \*
pervasive presence: operating systems, device drivers, compilers \* C++
and Java as “better C” \* fine-grained control over memory, CPU \*
de-facto standard \* impact of Rust remains to be seen

LISP: \* S-expressions and S-functions \* S-functions are expressed as
S-expressions -\> homoiconicity \* Garbarge collection (actually, a FREE
list)

> LISP programmers know the value of everyting and the cost of nothing.
> C programmers know the cost of everything and the value of nothing.

The reason Lisp failed was because it fragmented, and it fragmented
because that was the nature of the language and its domain-specific
solution style. The network effect worked in reverse. Less and less
programmers ended up talking the same dialect, and thus the total
“worth” ended up decreasing relative to the ALGOL family.

Lisp introduced the idea of the read-evaluate-print loop (REPL). The
environment read a line of Lisp code, evaluated it, printed the result,
and then looped back to the start of the process. This change allowed
for incremental development, where you gradually wrote a function, tried
calling it, tested that it worked, and then moved on to the next bit of
code.

Eric Raymond: “Lisp is worth learning for the profound enlightenment
experience you will have when you finally get it; that experience will
make you a better programmer for the rest of your days, even if you
never actually use Lisp itself a lot.”

Garbage Collection Lisp was the very first programming language to
provide automatic garbage collection, based on a stop-the-world
mark-and-sweep model. Periodically, the collector would interrupt the
program, stop its running, follow every pointer from a small set of
roots, and then collect all memory that it hadn’t managed to reach.

By modern standards, this design is very primitive; modern Lisp
implementations use concurrent generational garbage collection, as do
other languages such as Java. For the time, however, that first garbage
collection approach was innovative. The programmer didn’t have to think
about memory allocation; he just had to put up with occasional (brief)
pauses.

John McCarthy and Dennis Ritchie passed away late last year within a
couple of weeks of each other. They created two of my favorite
programming languages: Lisp (McCarthy) and C (Ritchie).

Lisp is the most historically important programming language. It
introduced three revolutionary features: automatic memory management
(via garbage collection), the bundling of code and data (via first-class
functions), and memory safety (e.g., no buffer overflows). I doubt that
any widely-used programming language going forward will be introduced
without these three features, and Lisp had them in 1959.

All of the core systems on computers today are written in C: operating
systems, file systems, network stacks, garbage collectors and runtime
systems for Lisp-y languages. Personally I think of it as the rotten
core, but that’s not C’s fault: no one has come up with a better
alternative, a language with safety but also C’s fine degree of control
over data representations, memory and CPU use.
