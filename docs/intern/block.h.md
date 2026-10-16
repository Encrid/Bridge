# src/block.h

This header gives the shared block, the column region in it, the log ring of a module, and the barrier.
Storage keeps the rows behind `src/storage.h`.

## The regions of the memories

A module reaches nine regions with no call, and they do not have the same readers:

| Region | Memory | Who reads it |
| --- | --- | --- |
| The component columns | shared | Each module that declared the component. |
| The entity index | shared | Each module that follows a reference. |
| The indexes of the sparse sets | shared | Each module that declared a component kept in a sparse set. |
| The direct-indexed bits | shared | Each module that declared a direct-indexed component. |
| The persistent arena | shared | Each module that holds a string in a component. |
| The query spans | private | Only the module of the spans. |
| `column_base` | private | Only its module. |
| The scratch arena | private | Only its module. |
| The log ring | private | Its module, and Bridge, which drains the ring. |

Each module has a private memory of its own, and every module sees the shared memory.
The private memory is the library's alone, so the library lays it out from offset 0, with nothing to agree.
A header at offset 0 tells where each region of the module begins: `docs/intern/block_module.c.md`, *The header*.

Each other offset into the shared memory is from the start of the shared memory.
The `(offset, length)` that a component holds into the persistent arena is one of them, so nothing else needs a base.

The shared memory stays open to each module that it is handed to.
The columns are shared because shared columns are the design.
The entity index is in it because a wasm module reads the index with plain loads, and wasm has no import that is read-only.

What storage keeps for itself is in neither memory.
Where each entity is, which entity is in which row, and the sets and offsets of each table are in memory of the library's own.
No module can address that memory.
The entity index and the arrays of entity ids in the shared block are copies, which storage writes and never reads back.

## A phase and its barrier

A phase runs its systems, and then a barrier follows.
The host asks for the phases one at a time, and it decides their order.
A barrier applies the queued changes, changes the layout of storage, publishes the spans again, and then runs the observers of what changed.
One phase and its barrier make a deployment that works.

Rows move only at a barrier, so nothing moves while a phase runs.
Each phase puts off its structural changes, so the library has one safe moment to change storage.
It is the same moment at which the library publishes the spans again, so a span is a snapshot.

A barrier follows each phase, and not each frame.
So a spawn in one phase is visible in the next phase of the same frame.
A barrier costs what changed, and a visit to each module: its ids topped up, its failures counted, and its log drained.

A spawn is visible at the next barrier.
`spawn` gives a usable id at once, but the row does not exist until the barrier, and no query counts it.
A destroyed entity stays readable for the rest of the phase that destroyed it.
So each system sees the same state of the world for the whole of a phase. The systems that run at the same time agree.

## Types

| Type | Content |
| --- | --- |
| `block_layout_t` | Where each part of the shared block begins, how long the block is, and how much address space is reserved for it. |

The fields of `block_layout_t`:

| Field | Content |
| --- | --- |
| `index` | The start of the entity index. Offset 0 is none, so nothing is there: eight bytes keep the index aligned. |
| `sparse` | The start of the indexes of the sparse sets. |
| `sets` | How many sparse sets there may be. |
| `direct` | The start of the direct-indexed bits. |
| `cells` | How many direct-indexed components there may be. |
| `arena` | The start of the persistent arena. |
| `columns` | The start of the column region. |
| `len` | The length of the block. |
| `reserve` | The address space reserved for the block. |

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `block_create` | `src/block_create.c` | Makes the shared block that the budget of a deployment states, with storage in it, and has every backend adopt it. 0, or 1 with a record. |
| `block_destroy` | `src/block_destroy.c` | Gives the shared block back, with storage and every declared type. |
| `block_check_budget` | `src/block_budget.c` | Refuses a budget with a key unset, a size past what 32 bits reach, or a count that is neither a count nor none. 1 with a record. |
| `block_lay_out` | `src/block_layout.c` | Finds where each part of the shared block begins. 1, with a record, if together they are more than an offset of 32 bits can reach. |
| `block_refill` | `src/block_module.c` | Tops up the ids of a module until the next barrier: what it did not use, up to what its config stated. 1 if the entity ceiling cut them short. |
| `block_write_header` | `src/block_module.c` | Writes what a module reads at offset 0 of its private memory. |
| `block_hand_out` | `src/block_module.c` | Ends the hold on each id that a module handed out since the last barrier. |
| `block_get_offset` | `src/block_module.c` | Gives where a pointer is in the shared memory, or 0 for a pointer that is null or not in it. |
| `block_drain` | `src/block_log.c` | Hands the log ring of a module to the sink. Then gives the module its scratch arena back, which holds the text of the lines. |
| `block_barrier` | `src/block_barrier.c` | Runs the barrier: the queued changes applied, the spans published again, the observers run, the failure budgets settled, the log rings drained. |
| `block_column_regrow` | `src/block_column.c` | Gives `new_size` bytes of the column region that hold what the `old_size` bytes at `ptr` held, or fresh bytes if `ptr` is null. |
| `block_column_free` | `src/block_column.c` | Gives `size` bytes of the column region back. |
