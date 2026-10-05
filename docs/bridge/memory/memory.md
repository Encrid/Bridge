# Memory

Which block each thing lives in, and who may read it. What it costs when one runs out is
`budgets.md`; where a string goes is `strings.md`; where flecs allocates is `flecs.md`. What the
data *is* is `data.md`.

## Where the seven regions live

Seven things must be addressable with no call, and they do not have the same readers:

    region              memory    who reads it
    component columns   shared    every module that declared the component
    entity index        shared    every module that follows a reference
    persistent arena    shared    every module holding a string in a component
    query spans         private   only the module they belong to
    column_base         private   only the module they belong to
    scratch arena       private   only the module they belong to
    log ring            private   one per module, and Bridge-ASM700 draining it

The private memory is Bridge-ASM700's alone, so it is laid out from offset 0 with nothing to
negotiate. A fixed header says where this module's spans, `column_base`, arena and log ring begin,
the log level it is filtering at, and where the entity index begins in the shared memory - the one
thing it reads there that it cannot derive. Every other shared-memory offset is relative to the
shared memory itself, including the `(offset, length)` a component holds into the persistent arena,
so nothing else needs a base.

The span region holds the query slots, the spans and `column_base`. Its size is the module's
`spans` budget, in bytes - `budgets.md`, *Budgets* - and how it is divided is Bridge-ASM700's, redone
at a barrier, where nothing holds an offset into it. That is Quake's progs layout - fixed offsets at
the bottom, everything else found through them.

What fills it is archetype count, which is a property of the deployment and not of the module. So
`spans` is the key an author states a guess for and a deployment is expected to correct, and a
reload that states more just gets it.

What stays exposed is the shared memory: the columns, because shared columns are the design, and the index,
because a wasm module must read it with plain loads and wasm has no read-only import.
