# What was refused: Stage 12

What was weighed in `stage12.md` and refused, each with the condition that would bring it back.

## One operating-system layer

Of `stage12.md`, *One operating-system layer*.

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

Of `stage12.md`, *The three entries a module calls at load*.

Refused: the arguments grouped into four, as spans and small structs or as words of 64 bits.
Nothing more is read. A struct passed by value crosses a wasm import as a hidden pointer in C
and otherwise in another language, and a packed word is arithmetic both sides must reproduce
with nothing that asserts it. No condition brings it back.

Refused: several calls, one that names a type and answers a handle and others that state its
shape and how it is kept. Each call fits the rule. A type is then half stated between two calls,
and what is unset is known by a call that was not made, which `stage0.md` step 6 refuses under
explicit. No condition brings it back.

Refused: an exception for the three in `style.md`. It was proposed with no reason: the seams
are where a rule about arguments is read by the most people. No condition brings it back.

## The three entries of the backend seam over four

Of `stage12.md`, *The three entries of the backend seam over four*.

Refused: `instance_create` keeping its backend, with the memories and their count in a struct
of their own. It is a struct for a pointer and a count, and it keeps an argument neither backend
reads. The condition that brings it back is a backend whose module cannot hold what it is of.

## A line of the log

Of `stage12.md`, *A line of the log*.

Refused: one number a line where there are two. The native SDK's function is still over four,
and the layout of a record in the ring changes for it. The condition that brings it back is
that no line anywhere uses the second number.

## attach, which runs in a phase

Of `stage12.md`, *attach, which runs in a phase*.

Refused: `len` kept, and the id and the slot passed as one struct by its offset. It is a write
in the module and a checked read in the library at every attach. The condition that brings it
back is a component whose size is not known at `declare`.

Refused: the table of a native module found by the thread, so that no entry is handed it. Stage
7 removed exactly that - `build.md`, *The standard*. No condition brings it back.

Refused: an exception in `style.md` for the native entry and its SDK function. The length it
would keep states what the declaration states. The condition that brings it back is the line
of `detach` not being explained and not being undone.

## Names

Of `stage12.md`, *Names*.

Refused: the verb for what is inside only, with the seams' names left. Two conventions, and the
seams are what the most people read. No condition brings it back before a release; after one,
a rename at a seam is weighed against who it breaks.

Refused: a participle counting as the verb, so that `moved` and `killed` stay. About
twenty-five renames where there are sixty. `moved(e)` reads as "has it moved" and as "it is
moved now", which is two meanings for one word. No condition brings it back.

Refused: a tag ending in `_s` beside the typedef ending in `_t`. Two suffixes to know, and the
tag is written in one place. No condition brings it back.

Refused: what is built keeping its hyphen. Rule O4 is about what is tracked and does not ask
for it. A backend would then be spelled two ways. No condition brings it back.

## How a file of many functions is cut

Of `stage12.md`, *How a file of many functions is cut*.

Refused: optimisation at link time, which would inline across `.c` files. Probed with zig
0.16: the link fails for `x86_64-windows-gnu` on symbols of the C runtime, `aarch64-macos`
answers that it requires another linker, and an object becomes LLVM bitcode, so the static
library could not be linked by a host's own linker - `host_api.md`, *A host links a built
library*. The condition that brings it back is all three ceasing to be so.

Refused: the pieces as `.c` files that one `.c` includes, which alone is compiled. The unit is
the same and no code is in a header. A piece does not compile alone, and the build and a reader
must both know which `.c` is a piece. No condition brings it back.

Refused: an exception to rule O3 for the files a barrier runs through. The least work, and the
same speed. The rule would then not hold where the code is hardest to read. The condition that
brings it back is a line that stays slower after the cut and cannot be explained.

## The test programs

Of `stage12.md`, *The test programs*.

Refused: one program holding every case in one table. One `main` and one fixture. Every change
then links every case, the cases `build.py` lists are all renamed, and the programs that need a
process of their own, the one that counts allocations and the one linked statically, stay apart
anyway, which makes two kinds. The condition that brings it back is link time of the test
programs becoming what a change waits for.

## Comments

Of `stage12.md`, *Comments*.

Refused: citations in code that nothing checks, as they were. A heading is renamed and the code
points at nothing, silently. No condition brings it back.

Refused: code citing nothing. 136 ways from a function to its reason are lost. The condition
that brings it back is the documents naming every function they are the reason for.

Refused: a comment above every function. After the cut there are about six hundred, and many
would say again what the name says. The condition that brings it back is a reader finding
`static` functions whose name did not say it, often enough.

Refused: no function bound to have one. Nothing is held, and the functions another file calls
are the ones read without their body. No condition brings it back.
