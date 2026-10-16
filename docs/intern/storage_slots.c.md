# src/storage_slots.c

This file tells what is left of what a budget counts, records what a module may write, and forgets a type.

## Functions

| Function | Result |
| --- | --- |
| `storage_count_free_sets` | Gives how many more types a sparse set can keep: the free slots of `[budget] sparse_sets`. |
| `storage_count_free_direct` | Gives how many more types can be direct-indexed: the free slots of `[budget] direct_indexed`. |
| `storage_make_writable` | Makes the set `writable` of a module, once. Gives false if there is no memory. Bridge calls it before it makes a type, so `storage_let_write` cannot fail. |
| `storage_let_write` | Puts a type in the set `writable` of a module. That gives the module a stake in each entity that carries the type: `docs/intern/storage_stake.h.md`, *What a module may destroy*. |
| `storage_undeclare` | Forgets a type that nobody declares any more. Frees its id and its slot, takes it out of the counts, and gives back the block of a direct-indexed type or of a singleton. |
