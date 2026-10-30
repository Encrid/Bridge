# src/block/block.h

This header gives the shared block, the column region in it, the log ring of a module, and the barrier.
Storage keeps the rows behind `src/storage/storage.h`.

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

Each module has its own private memory, and all modules see one shared memory: `docs/extern/bridge_abi.h.md`, *The memories*.
The private memory is the library's alone, so the library lays it out from offset 0, with nothing to agree.
A header at offset 0 tells where the regions are: `docs/extern/bridge_abi.h.md`, *bridge_header_t*, and `docs/intern/block/block_module.c.md`, *The header*.

`block_write_room` writes `changes_left` of the header: the room of the queue of changes that the module did not use.
`block_write_header` calls it when the instance starts, and `queue_change` after each change.
`empty_queues` calls it at a barrier, for a module whose queue is not empty: `docs/intern/storage/storage_settle.h.md`, *What a module hears*.
Each of these calls comes after the private memory of the module is made, so the write needs no check.

Each other offset into the shared memory is from the start of the shared memory.
The `(offset, length)` that a component holds into the persistent arena is one of them, so nothing else needs a base.
`block_get_offset` gives the offset of a pointer into the shared memory, or 0 if the pointer is null or lies outside it.

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

A spawn is visible at the next barrier: `docs/extern/bridge_abi.h.md`, *spawn*.
`spawn` gives a usable id at once, but the row does not exist until the barrier, and no query counts it.
A destroyed entity stays until the barrier too: `docs/extern/bridge_abi.h.md`, *destroy*.
So the state of the world does not change during a phase, and the systems that run at the same time agree: `docs/extern/bridge.h.md`, *bridge_run_phase*.

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
| `block_get_offset` | `src/block/block.h` | Gives the offset of a pointer into the shared memory: *The regions of the memories*. |
| `block_write_room` | `src/block/block.h` | Tells a module what is left of its queue of changes: *The regions of the memories*. |
| `block_create` | `src/block/block_create.c` | Makes the shared block: `docs/intern/block/block_create.c.md`, *Functions*. |
| `block_destroy` | `src/block/block_destroy.c` | Gives the shared block back: `docs/intern/block/block_destroy.c.md`, *Functions*. |
| `block_check_counts` | `src/block/block_counts.c` | Checks the counts of a budget against 32 bits and the hash of tables: `docs/intern/block/block_counts.c.md`, *Functions*. |
| `block_check_budget` | `src/block/block_budget.c` | Checks the budget of a deployment: `docs/intern/block/block_budget.c.md`, *Functions*. |
| `block_lay_out` | `src/block/block_layout.c` | Lays out the shared block: `docs/intern/block/block_layout.c.md`, *Functions*. |
| `block_refill` | `src/block/block_module.c` | Tops up the ids of a module: `docs/intern/block/block_module.c.md`, *The ids set aside*. |
| `block_write_header` | `src/block/block_module.c` | Writes the header of a module: `docs/intern/block/block_module.c.md`, *Functions*. |
| `block_hand_out` | `src/block/block_module.c` | For each module, ends the hold on the ids that it handed out: `docs/intern/block/block_module.c.md`, *Functions*. |
| `block_drain` | `src/block/block_log.c` | Drains the log ring of a module, and gives back its scratch arena: `docs/intern/block/block_log.c.md`, *The log ring*. |
| `block_barrier` | `src/block/block_barrier.c` | Runs the barrier: `docs/intern/block/block_barrier.c.md`, *The steps of a barrier*. |
| `block_publish_all` | `src/block/block_publish.c` | Publishes the spans of each running module: `docs/intern/block/block_publish.c.md`, *Functions*. |
| `block_column_regrow` | `src/block/block_column.c` | Gives a piece of the column region a new length: `docs/intern/block/block_column.c.md`, *Functions*. |
| `block_column_free` | `src/block/block_column.c` | Frees a piece of the column region: `docs/intern/block/block_column.c.md`, *Functions*. |
