# What was refused: Stage 12

What was weighed in `STAGE12.md` and refused, each with the condition that would bring it back.

## One operating-system layer

Of `STAGE12.md`, *One operating-system layer*.

Refused: the library handing a backend the layer over the backend seam, as a table of functions
given at load. One compiled copy, and a backend in any language would have threads and mapping
without writing them. It adds about nineteen entries to the seam, makes every lock, wait and
reading of the clock in the wasm backend a call through a pointer, and has the library promise
an operating-system interface to whoever writes a backend. The condition that brings it back is
a backend written in a language that cannot reach the operating system itself.

Refused: the three copies staying, each made to follow the style on its own. No rule of stage
10 is reworded. Every platform the stage adds is then written three times, and about 250 lines
are kept twice. The condition that brings it back is a backend whose layer has to differ from
the library's in what a function does, and not only in which functions it has.

Refused: the folder named `shared/`. `shared` names the memory every module reads. No condition
brings it back.

## The three entries a module calls at load

Of `STAGE12.md`, *The three entries a module calls at load*.

Refused: the arguments grouped into four, as spans and small structs or as words of 64 bits.
Nothing more is read. A struct passed by value crosses a wasm import as a hidden pointer in C
and otherwise in another language, and a packed word is arithmetic both sides must reproduce
with nothing that asserts it. No condition brings it back.

Refused: several calls, one that names a type and answers a handle and others that state its
shape and how it is kept. Each call fits the rule. A type is then half stated between two calls,
and what is unset is known by a call that was not made, which `STAGE0.md` step 6 refuses under
explicit. No condition brings it back.

Refused: an exception for the three in `STYLE.md`. It was proposed with no reason: the seams
are where a rule about arguments is read by the most people. No condition brings it back.

## The three entries of the backend seam over four

Of `STAGE12.md`, *The three entries of the backend seam over four*.

Refused: `instance_create` keeping its backend, with the memories and their count in a struct
of their own. It is a struct for a pointer and a count, and it keeps an argument neither backend
reads. The condition that brings it back is a backend whose module cannot hold what it is of.

## A line of the log

Of `STAGE12.md`, *A line of the log*.

Refused: one number a line where there are two. The native SDK's function is still over four,
and the layout of a record in the ring changes for it. The condition that brings it back is
that no line anywhere uses the second number.

## attach, which runs in a phase

Of `STAGE12.md`, *attach, which runs in a phase*.

Refused: `len` kept, and the id and the slot passed as one struct by its offset. It is a write
in the module and a checked read in the library at every attach. The condition that brings it
back is a component whose size is not known at `declare`.

Refused: the table of a native module found by the thread, so that no entry is handed it. Stage
7 removed exactly that - `build.md`, *The standard*. No condition brings it back.

Refused: an exception in `STYLE.md` for the native entry and its SDK function. The length it
would keep states what the declaration states. The condition that brings it back is the line
of `detach` not being explained and not being undone.
