# Memory

Which block each thing lives in, and who may read it. What it costs when one runs out is
`budgets.md`; where a string goes is `strings.md`; who keeps the rows is `storage.md`. What the
data *is* is `data.md`.

## Where the regions live

Nine things must be addressable with no call, and they do not have the same readers:

    region              memory    who reads it
    component columns   shared    every module that declared the component
    entity index        shared    every module that follows a reference
    sparse-set indexes  shared    every module that declared a component kept in one
    direct-indexed bits shared    every module that declared a direct-indexed component
    persistent arena    shared    every module holding a string in a component
    query spans         private   only the module they belong to
    column_base         private   only the module they belong to
    scratch arena       private   only the module they belong to
    log ring            private   one per module, and Bridge draining it

The private memory is Bridge's alone, so it is laid out from offset 0 with nothing to
negotiate. A fixed header says where this module's spans, `column_base`, arena and log ring begin,
the log level it is filtering at, and where the entity index begins in the shared memory - the one
thing it reads there that it cannot derive - and, for each type it declared that is not kept in a
table, where that is reached: `storage.md`, *A sparse set*. Every other shared-memory offset is
relative to the shared memory itself, including the `(offset, length)` a component holds into the
persistent arena, so nothing else needs a base.

The span region holds the query slots, the spans, `column_base`, a cell for each declared type
that says where a component not kept in a table is reached, and each observer's list of what
triggered it. Its size is the module's `spans` budget, in bytes - `budgets.md`, *Budgets* - and how
it is divided is Bridge's, redone at a barrier, where nothing holds an offset into it. That
is the memory layout of Quake's progs - fixed offsets at the bottom, everything else found through
them.

What fills it is `column_base`, which the deployment's `[budget] tables` sizes, and then spans,
which follow how many tables the module's queries match: both are properties of the deployment and
not of the module. So `spans` is the key an author states a guess for and a deployment is expected
to correct, and a reload that states more just gets it.

What stays exposed is the shared memory: the columns, because shared columns are the design, and the index,
because a wasm module must read it with plain loads and wasm has no read-only import.

**What storage itself keeps is in neither.** Where each entity is, which entity is in which row,
every table's set and offsets: memory of the library's own, which no module can address. The index
and the entity arrays in the shared block are copies of it, written and never read back -
`storage.md`, *Nothing in the shared block is read back*.
