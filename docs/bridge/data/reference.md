# Reference following

Iterating a query covers the hot path, but not everything: a projectile crediting its owner, a
monster reading its target, a trigger reaching its door. Following a reference means reaching *one*
entity by id, and that is a lookup rather than a walk.

It is arithmetic, not a call. Bridge-ASM700 publishes two flat arrays and the whole of `get` is:

    record = entity_index[id & 0xffffffff]     // u32 table, u32 row, u32 generation
    if record.generation != id >> 32           -> stale id, refuse
    base   = column_base[record.table * terms + slot]
    if base == 0                               -> entity lacks this component
    ptr    = base + record.row * sizeof(T)

Two dependent loads and a multiply. No crossing, which is the point: a crossing per reference would
reintroduce the per-operation cost this design exists to remove.

`column_base` is needed because a component sits at a different offset in every table, so the table
alone is not enough. It is indexed by table and by the terms *this* module declared, so ten
components across a thousand tables is forty kilobytes.

**The stale-id check falls out of it.** The generation is in the record, so refusing an id whose
entity has died is one load and one compare.

**What it costs.** The index changes only for entities created, destroyed, or moved between tables,
so it is patched at the merge point rather than rebuilt - **O(structural changes), unrelated to
world size**. Measured at 1.6 records an operation, where the swap-remove suggests two:
`tests/rowmove.c` runs 44,845 operations, and 51,185 entities are named by an event and 19,584 by
the hook.

**Memory is twelve bytes times the highest entity id** rather than the live count. flecs recycles
ids, so that tracks peak concurrent entities and not everything ever spawned. It is a **budget
declared at startup**, and the `spawn` that would exceed it fails, fatally and where it happened.
Growing it instead is not available - `budgets.md`, *Budgets*.

**What `get` does not buy.** Retrieval is memory-bound, not lookup-bound: two dependent cache
misses, roughly ten nanoseconds warm and a hundred and sixty cold. A crossing would pay those same
misses and add the call, so this is cheaper by exactly the crossing. So `get` is for following a
reference a few times. A relationship you walk every frame belongs in a query, where access is
sequential and the prefetcher does the work.
