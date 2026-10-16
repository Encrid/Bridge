# src/block_module.c

This file gives a module what it gets at a barrier: ids to hand out, and the header by which it reads its private memory.

## The ids set aside

At each barrier, a module gets a block of ids for the time until the next barrier, as many as its `spawns`.
They are real entities with nothing on them, which no query matches, and they count against `[budget] entities`.
`spawn` hands them out: `docs/intern/abi_calls.c.md`, *spawn*.
Before storage applies changes, `block_hand_out` ends the hold on the ids that each module handed out: `docs/intern/storage_held.c.md`.

`block_refill` keeps the ids that the module did not use, and moves them to the front of the block.
Storage then gives fresh ids for the rest.
A module that used no id since the last barrier has no id to move, so a quiet barrier costs little.

`block_refill` keeps at most `spawns` ids, and gives the others back to storage.
A reload that states fewer `spawns` than before leaves more ids than that: `docs/intern/reload_record.c.md`, *The ids*.
A module that states no `spawns` gets no ids, and gives back the ids that it held.

The entity index is twelve bytes for each id up to the highest, and the budget states that size.
So a refill can come up short, and then `ids_short` is set: the next `spawn` past the ids names `[budget] entities`.

## The header

At offset 0 of the private memory, a `bridge_header_t` tells where each region of the module begins.
The library writes all of the header.
A module writes `scratch_used` and `ring_head`, and nothing else.
`block_write_header` writes these fields:

| Field | Value |
| --- | --- |
| `abi` | `BRIDGE_ABI`: the number of the module seam. |
| `scratch`, `scratch_len` | Where the scratch arena begins, and its length in bytes. |
| `ring`, `ring_len`, `ring_tail` | Where the log ring begins, its length in records, and the count of records drained. |
| `queries` | The start of the span region: a `bridge_query_t` for each slot that `add_system` gave. |
| `observers` | After the queries: a `bridge_list_t` for each slot that `observe` gave. |
| `column_base` | After the lists: a `u32` for each type of the module in each table id. |
| `types` | How many types the module declared. |
| `tables` | How many table ids there are, with 0, which is no table. |
| `kept` | After `column_base`: a `u32` for each type, which tells where a type that is not kept in a table is reached. |
| `entity_index`, `entities` | Where the entity index begins in the shared memory, and `[budget] entities`. |

Each offset is from the start of the private memory, except `entity_index`.
The entity index is the one thing in the shared memory that a module reads and cannot find from other values.
`barriers`, `place`, `system` and `observer` are written before each dispatch: `src/schedule_dispatch.c`.
`log_level` is written when the private memory is made: `src/module_call.c`.

The span region holds the queries, the lists of the observers, `column_base`, `kept` and the spans.
Its length is the `spans` of the module, and how it is divided is the library's.
The library divides it again at a barrier, where nothing holds an offset into it.
`column_base` follows `[budget] tables`, and the spans follow how many tables the queries match.
Both come from the deployment, and not from the module.
So `spans` is a key whose value an author can only estimate, and a deployment corrects it.

## Functions

| Function | Result |
| --- | --- |
| `block_get_offset` | Gives where a pointer is in the shared memory, or 0 for a pointer that is null or not in it. |
| `block_refill` | Tops up the ids of a module. 1 if the entity ceiling cut them short. |
| `block_write_header` | Writes the header at offset 0 of the private memory of a module: where its regions begin, and how many of each thing there are. |
| `block_hand_out` | Ends the hold on the ids that each loaded module handed out since the last barrier, and on those that it took from its `lender`. |
