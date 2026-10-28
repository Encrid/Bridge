# src/block/block_module.c

This file gives a module what it gets at a barrier: ids to hand out, and the header by which it reads its private memory.

## The ids set aside

At each barrier, a module gets a block of ids for the time until the next barrier, as many as its `spawns`.
They are real entities with nothing on them, which no query matches, and they count against `[budget] entities`.
`spawn` hands them out: `docs/intern/abi/abi_calls.c.md`, *spawn*.
`block_hand_out` ends the hold on the ids that `spawn` handed out, in the first step of a barrier: `docs/intern/block/block_barrier.c.md`, *The steps of a barrier*.

`block_refill` keeps the ids that the module did not use, and moves them to the front of the block.
Storage then gives fresh ids for the rest.
A module that used no id since the last barrier has no id to move, so a quiet barrier costs little.

`block_refill` keeps at most `spawns` ids, and gives the others back to storage.
A reload that states fewer `spawns` than before leaves more ids than that: `docs/intern/reload/reload_record.c.md`, *The ids*.
A module that states no `spawns` gets no ids, and gives back the ids that it held.

The entity index is twelve bytes for each id up to the highest, and the budget states that size.
So a refill can come up short, and then `ids_short` is set: the next `spawn` past the ids names `[budget] entities`.

## The header

The header is the `bridge_header_t` at offset 0 of the private memory: `docs/extern/bridge_abi.h.md`, *bridge_header_t*.
`block_write_header` writes these fields:

| Field | Value |
| --- | --- |
| `abi` | `BRIDGE_ABI`: the number of the module seam. |
| `scratch`, `scratch_len` | Where the scratch arena begins, and its length in bytes. |
| `ring`, `ring_len`, `ring_tail` | Where the log ring begins, its length in records, and the count of records drained. |
| `queries` | The start of the span region. |
| `observers` | After the queries. |
| `column_base` | After the lists. |
| `types` | How many types the module declared. |
| `tables` | How many table ids there are, with 0, which is no table. |
| `kept` | After `column_base`. |
| `rows`, `blocks` | After `kept`: a cell per table for its row count, then a cell per table for its block. |
| `stride`, `write`, `kind` | A cell per type for its size, whether the module may write it, and its storage layout. |
| `owned` | After them: the count of owned ranges, then the ranges of the shared arena the module may write. |
| `arena_lo`, `arena_hi` | The bounds of the shared arena that the module may read. |
| `entity_index`, `entities` | The offset of the entity index, and `[budget] entities`. |
| `changes_left` | `queue_room` less `queue_len`, by `block_write_room`. |

Each offset is from the start of the private memory, except `entity_index`.
The entity index is the one thing in the shared memory that a module reads and cannot find from other values.
`block_get_offset` gives its offset: `docs/intern/block/block.h.md`, *The regions of the memories*.
`barriers`, `place`, `system` and `observer` are written before each dispatch: `src/schedule/schedule_dispatch.c`.
`log_level` is written when the private memory is made: `src/module/module_call.c`.
`changes_left` is written again after each change, and at the next barrier: `docs/intern/block/block.h.md`, *The regions of the memories*.

The span region holds the queries, the lists of the observers, `column_base`, `kept`, the capability table and the spans.
Its length is the `spans` of the module, and how it is divided is the library's.
The library divides it again at a barrier, where nothing holds an offset into it.
`column_base` follows `[budget] tables`, and the spans follow how many tables the queries match.
Both come from the deployment, and not from the module.
So `spans` is a key whose value an author can only estimate, and a deployment corrects it.

`write_caps` lays the capability table after `kept`: the row counts, the blocks, and the stride, write and kind of each type.
It also sets `arena_lo` and `arena_hi` to the bounds of the shared arena that a module may read.
It writes `entities` too, the value of `[budget] entities`.
`write_owned` writes the module's owned ranges after the table, up to `BRIDGE_OWNED_CAP` of them.
It writes the count first, then a pair of an offset and a length for each range.
It writes nothing if the ranges do not fit in the span region.

## Functions

| Function | Result |
| --- | --- |
| `block_refill` | Tops up the ids of a module. 1 if the entity ceiling cut them short. |
| `block_write_header` | Writes the header at offset 0 of the private memory of a module: where its regions begin, and how many of each thing there are. Gives 1 when the entity index is outside the block. |
| `write_caps` | Lays the capability table and the arena bounds into the header. |
| `write_owned` | Writes the owned ranges of the shared arena after the capability table. |
| `block_hand_out` | Ends the hold on the ids that each loaded module handed out since the last barrier, and on those that it took from its `lender`. |
