# The interface is a query

You do not get storage. You get your query's results, which is how an ECS is used:

    declare(...)   registers a component type
    system(...)    registers a query, a function and a phase together, and gets a slot

At each barrier Bridge-ASM700 writes, into the module's private memory, a span for every table its
query matches. Nothing else reads that memory; see `wasm.md`, *Memories*. One span for each table
that has rows, and a table without any does not exist - `storage.md`, *A table*.

**Only what changed is written.** Systems are fixed at load, so a table is matched against every
query once, when it is made; after that a span is rewritten when its table was touched, and a
query's list of them when the set of tables it matches changed - `storage.md`, *What a barrier
writes for a module*. A barrier at which nothing changed writes nothing.

    per query slot:
        u32 spans      how many tables matched
        u32 at         where this query's spans begin

    per span:
        u32 count      rows in this table
        u32 entities   where the entity-id array for these rows begins
        u32 base[n]    one offset per term, in declared order. zero = absent here

**Absent is a zero base, and nothing carries a flag.** flecs hands an iterator to an outside
reader the same way: its JSON serialiser tells a field that is not there by a null pointer
(`serialize_iter_result_query.c:177-186`). **Offset 0 of the shared memory is reserved** so that
zero is never a valid base, which also makes the same sentinel sound in `reference.md`, *Reference
following*.

Iterating is two loops and no call, with the branch outside the row loop:

    for each span:
        for each term t:
            if   base[t] == 0     term absent in this table
            else                  row r is at base[t] + r * sizeof(T)

`sizeof(T)` is a build-time constant from the schema, so stride is never published - the module's
compiler already knows it.

A span is three kinds of `u32` and nothing else, so it is `8 + 4n` bytes. How many terms a query
may have is the deployment's to state: `[budget] terms` - `budgets.md`, *Budgets*.

`entities` is there because a system that wants to destroy the row it is on, attach a component to
it, or store a reference to it needs the id and has no other route to one.

**Every term is a component the matched entity itself carries.** flecs' iterator has a third case,
a `source` for each field: the entity a value was found on when that is not the matched one, by
inheritance or a fixed source. The span had the field for it until stage 10, always zero: nothing
a module can ask for finds a term on another entity, and pairs, which inheritance is made of, are
refused. Refused, then: publishing a field nothing can fill. The condition that brings it back is a
term that may be matched on another entity, and it returns with that. Until then one entity's
value is reached from another by `reference.md`, *Reference following*.

## What a query may not ask for

**A term that is "this type or that one", and a term that is "any type".** Other ECS call them
or-terms and wildcards. The component such a term matched would vary from table to table. A
module reads a field through a **typed** accessor chosen at its build, and a term whose type
changes per table has no type to choose. So they are incompatible with the accessors, not merely
awkward in the span.

So there is no way to say either. A term is one declared type, and it must match, must not, or
may: three operators, and a value that is none of them is refused at `system` and `observe`, by
the number of the term. A table carries a type or does not, so every term that matches has a base
and a stride.

Refused: values in the ABI for both, kept so that the refusal could name them, which is what this
had. A seam that lists what it refuses has to be carried by everything that speaks it. The
condition that brings them back is authors who try one and cannot tell from the refusal why.

Many tables per query is the normal case: that is what archetypes mean. And publishing costs
**O(tables touched)**, not O(data) and not O(tables), which is the property that makes this scale.
A query over ten million entities costs the same to publish as one over a thousand, and one over
ten thousand tables that did not change costs nothing - `STAGE10.md`, *What it costs*.
