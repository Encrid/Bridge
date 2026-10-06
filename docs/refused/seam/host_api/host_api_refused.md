# What was refused: Host API

The record of `host_api.md`: `README.md`, *Three layers, three seams*.

## A host links a built library

Of `host_api.md`, *A host links a built library*.

Refused: a shared library only. It fails nothing, and it is one file fewer; it was not chosen,
because a host that must be a single file with nothing beside it has no other way. The condition
that brings it back is `PLAN.md`, stage 13, finding no host that needs a single file.

Refused: a static library only, which a language that can only load a shared library cannot use at
all. No condition brings it back.

## A seam has a number

Of `host_api.md`, *A seam has a number*.

Refused: one number for the three. A change to what a module reads would then refuse every host and
every backend, which it does not concern. The condition that brings it back is seams that can no
longer change apart.

Refused: a number for the module seam alone, which is what this had: a host or a backend built at
another time was read as it lay. No condition brings it back.

## Bridge-ASM700 parses nothing

Of `host_api.md`, *Bridge-ASM700 parses nothing*.

Refused: the host opening the library and handing over a table of functions, which keeps
Bridge-ASM700 away from every file and costs every host the same loader. The condition that brings
it back is an embedder that must link a backend statically.

## The data

Of `host_api.md`, *The data*.

Refused: the phase's index in the declared list, which saves the comparing and runs the wrong phase
without a word when the list is reordered. No condition brings it back.

Refused: a handle answered once for a name, which is one more call and one more thing a host holds.
The condition that brings a handle back is the lookup measured as a cost, in a deployment with
enough phases or enough calls for it to show.
