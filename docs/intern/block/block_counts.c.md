# src/block/block_counts.c

This file refuses the counts of a budget that storage cannot hold in 32 bits. It also refuses table ids that the hash of tables cannot hold.

## The checks

`block_check_budget` calls these checks after the keys of the budget: `docs/intern/block/block_budget.c.md`, *The checks*.

1. Refuse `types` past `UINT32_MAX` once rounded up to a multiple of 64. Storage keeps a bit for each type.
2. Refuse `tables`, `sparse_sets` and `types` whose sum and 1 pass `UINT32_MAX`. The column region keeps a range for each.
3. Refuse table ids past `STORAGE_HASH_MOST` times `table_hash_fill`, divided by 100. The table ids are `tables`, `sparse_sets` and 1.

Each check adds in 64 bits, and a `sparse_sets` of `BRIDGE_NONE` counts as 0.
The third check keeps the hash of tables from a wrap: `docs/intern/storage/storage_made.c.md`, *The hash of tables*.
At the most table ids, the hash has `STORAGE_HASH_MOST` places. One id more would need twice that, and a count of 32 bits does not hold it.

## Functions

| Function | Result |
| --- | --- |
| `get_sets` | Gives the count of sparse sets: 0 for `BRIDGE_NONE`. |
| `is_too_many` | Refuses `types`, `tables` and `sparse_sets` whose counts storage cannot hold in 32 bits. 1 with a record. |
| `is_past_hash` | Refuses table ids that the hash of tables cannot hold at `table_hash_fill`. 1 with a record. |
| `block_check_counts` | Runs the three checks of *The checks*, in order, and stops at the first that refuses. 1 with a record. |
