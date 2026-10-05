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
alone is not enough. It is indexed by table and by the terms *this* module declared, and it has a
cell for every table there may be - `[budget] tables` - so that a table made at any barrier has
its place: ten components at a thousand tables is forty kilobytes of the module's `spans`.

**The stale-id check falls out of it.** The generation is in the record, so refusing an id whose
entity has died is one load and one compare.

**What it costs.** A record is written when its entity is placed: made, destroyed, moved to another
table, or moved within one to fill the hole a departing row left. Storage does the placing, so it
writes the record at the same place in the code and there is nothing to keep in step -
**O(structural changes), unrelated to world size**. `column_base` is written for the tables that
were touched.

**Both are copies.** Where an entity is, storage keeps for itself, outside the block; what a module
reads is written from that and never read back - `storage.md`, *Nothing in the shared block is read
back*. A module that writes over the index misleads whoever follows a reference next, itself
included, and changes nothing else.

**Memory is twelve bytes times the highest entity index** rather than the live count. A destroyed
entity's index is handed out again, so that tracks peak concurrent entities and not everything ever
spawned. It is a **budget declared at startup**, and the `spawn` that would exceed it fails, fatally
and where it happened. Growing it instead is not available - `budgets.md`, *What running out does*.
The ids a module holds ready count against it: a row of the index each, pointing at no table. Index
0 is no entity, so `entities` of N holds N - 1.

**What `get` does not buy.** Retrieval is memory-bound, not lookup-bound: two dependent cache
misses, roughly ten nanoseconds warm and a hundred and sixty cold. A crossing would pay those same
misses and add the call, so this is cheaper by exactly the crossing. So `get` is for following a
reference a few times. A relationship you walk every frame belongs in a query, where access is
sequential and the prefetcher does the work.
