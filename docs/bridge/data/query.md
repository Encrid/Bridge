# The interface is a query

You do not get storage. You get your query's results, which is how an ECS is used:

    declare(...)   registers a component type
    system(...)    registers a query, a function and a phase together, and gets a slot

At each barrier Bridge-ASM700 runs every registered query and writes the matched spans into the module's
private memory, which nothing else reads; see `wasm.md`, *Memories*. One span per
matched table - exactly what `ecs_query_next` yields inside Bridge-ASM700, materialised for a reader that
cannot call it.

    per query slot:
        u32 spans      how many tables matched
        u32 at         where this query's spans begin

    per span:
        u32 count      rows in this table
        u32 entities   where the entity-id array for these rows begins
        u32 base[n]    one offset per term, in declared order. zero = absent here
        u64 source[n]  entity a term came from. zero = per-row on the matched entity

**This is flecs' encoding, not an invention.** `ecs_iter_t` (`flecs/include/flecs.h:1192`) carries
`entities`, `ids` and `sources` per field. The three cases a reader must tell apart are the three
flecs' own JSON serialiser tells apart when it publishes an iterator to an external consumer, in
`src/addons/json/serialize_iter_result_query.c:177-186`. It does so with two null checks, `!ptr`
then `!it->sources[f]`, which is why nothing here carries a flag. **Offset 0 of the shared memory is
reserved** so that zero is never a valid base, which also makes the same sentinel sound in
`reference.md`, *Reference following*.

Iterating is two loops and no call, with the branch outside the row loop:

    for each span:
        for each term t:
            if   base[t] == 0     term absent in this table
            elif source[t] != 0   one value, at base[t], for every row
            else                  row r is at base[t] + r * sizeof(T)

`sizeof(T)` is a build-time constant from the schema, so stride is never published - the generator
already knows it.

The span is laid out at natural alignment, so `source[]` begins at the next multiple of eight after
`base[]`. A query takes at most 32 terms, inherited from flecs' `FLECS_TERM_COUNT_MAX`.

`entities` is there because a system that wants to destroy the row it is on, attach a component to
it, or store a reference to it needs the id and has no other route to one. `source[t]` does double
duty: zero says the term is per-row on the matched entity; non-zero says it is shared and names the
entity it came from. That makes an inherited term one load per span rather than one per row.

## What a query may not ask for

**Or-terms and wildcards.** Their matched id varies per table, which is why flecs publishes `ids`
per field. Our generator emits a **typed** accessor at build time, and a term whose type changes per
table has no type to emit. So a wildcard is incompatible with the generator, not merely awkward in
the span.

**Terms needing `field_at`** - flecs' `row_fields` - because a field that is not an array has no
base-and-stride form at all.

Both are refused at `system` and `observe`, by name - that is where terms are handed over, not
`declare`, which only registers a type. Three mechanisms cover it. `EcsQueryCacheAll` makes flecs
fail query creation unless every term matches whole archetypes, which is what one-span-per-table
assumes. Wildcards pass that test, so Bridge-ASM700 refuses those by inspecting the terms it was handed.
`row_fields` catches the rest after creation. So every refusal has a diagnostic and none costs a
frame.

Many tables per query is the normal case: that is what archetypes mean. And publishing costs
**O(matched tables)**, not O(data), which is the property that makes this scale. A query over ten
million entities costs the same to publish as one over a thousand.
