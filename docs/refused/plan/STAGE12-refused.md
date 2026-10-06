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
