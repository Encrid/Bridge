# src/lib/inside.h

This header gives the inside of the library: what a world, a module, a system and a declared type are.
Nothing here crosses a seam.

The header includes `include/bridge.h`, `include/bridge_abi.h`, `src/lib/backend/backend.h`, `src/os/os.h`, `src/lib/pool/pool.h`, `src/lib/region/region.h`, `src/lib/schema/schema.h` and `src/lib/storage/storage.h` first.
After its types, it includes the headers that use them: `src/lib/say/say.h`, `src/lib/schedule/schedule.h`, `src/lib/bridge/bridge_inside.h`, `src/lib/block/block.h` and `src/lib/abi/abi.h`.

## Nothing is allocated in a phase

A phase and the barrier after it never call the allocator.
Each thing that they use got its room when the world was made or when a module loaded, from a number that somebody stated:

| What | When | How much |
| --- | --- | --- |
| The rows, tables and indexes of storage | At create | From `[budget]`. |
| What a dispatch works in | At create | `[budget] systems` long. |
| What a schedule is made in | At create | `[budget] systems` squared, in bits. |
| The block of ids of a module | At load | `spawns` long. |
| The queue of changes of a module | At load | `changes` bytes. |
| The list of who triggered an observer | At load | As many records as the `spans` of the module can publish. |
| The marks that order the queues | When a module gets a system, and before a lifecycle call | One for each system of the module. |

A load, a reload and an unload of a module allocate, and so do the creation and the destruction of a world.
They allocate the record of the module, of each of its systems and of each of its types, and the rooms above.
None of them is on the hot path.

## Types

| Type | Content |
| --- | --- |
| `open_backend_t` | A backend that the deployment configured, opened. |
| `declared_type_t` | A component or another declared type, by its namespaced name. |
| `pending_t` | Who triggered an observer since the last barrier, and how: `id` and `events`. |
| `system_term_t` | What the conflict graph reads of a term: `touches`, the component or null where the term reads nothing, and `writes`. |
| `bridge_system_t` | A system or an observer. |
| `module_mark_t` | Where the changes of one turn begin in the queue of a module: `turn` and `at`. |
| `module_t` | A loaded module. |
| `change_t` | One queued change. |
| `kept_text_t` | A share of an unloaded module that stays: `stem`, the place of its settings text `at` and `room`, and its ranges `owned` and `own_count`. |
| `schedule_matrix_t` | Who runs before whom among `n` systems or names. |
| `schedule_pairs_t` | The names of the pairs of `[order]`, and their closed matrix. |
| `bridge_t` | A world. |

### An open backend

| Field | Content |
| --- | --- |
| `name` | The name of the backend in the deployment. |
| `lib` | Its library, opened. |
| `api` | Its entries, found by name. |
| `be` | The backend, started with its settings. |
| `capabilities` | What the backend states that it can do. |
| `shared` | The shared block, as this backend sees it. |
| `reservation`, `guard`, `granule` | The address space that the backend needs around a block it maps: `docs/intern/lib/block/block_create.c.md`, *The shape of a block*. |

### A declared type

Storage holds the rows of a component, and this record is what a load checks.

| Field | Content |
| --- | --- |
| `name` | The namespaced name. |
| `type` | The type as storage knows it: a size. |
| `s` | Its fields or its constants, read from its text. |
| `kind` | What it is: a struct, an enum, a bitmask or a tag. |
| `storage` | The storage layout that each declarer stated. |
| `size`, `align` | Its size and its alignment. |
| `fingerprint` | The fingerprint of its descriptor, which names its layout in a record: `docs/intern/lib/abi/abi_stated.c.md`, *The fingerprint*. |
| `desc`, `desc_len` | A copy of its descriptor, which a later declaration is compared with: `docs/intern/lib/abi/abi_layout.c.md`, *One layout*. |
| `declarers` | How many loaded modules declared it. |
| `users` | How many components name it as the type of a field. |
| `deps`, `dep_count` | The declared types that its own fields name. |
| `zero` | `size` bytes of zero: what an attach with no value copies. |
| `replaces` | At a reload, the type of this name whose rows become the rows of this type when the reload holds. |
| `superseded` | That another type of this name will take the place of this type. |
| `runs`, `run_count` | On the type that is superseded, which bytes of its value are which bytes of the value of its successor. |

`runs` is null if the bytes are the same: `docs/intern/lib/abi/abi_relay_run.c.md`, *A run*.

### A system

A system and an observer are a query and a function either way, so one record serves both.

| Field | Content |
| --- | --- |
| `m` | Its module. |
| `name` | The name that it registered. |
| `full` | How `[order]` names it: the stem of its module, a colon, then its name. |
| `named` | The index of `full` among the names of `[order]`: `docs/intern/lib/schedule/schedule_names.c.md`, *The index of a system*. |
| `phase` | Its phase, or the barrier for an observer: `docs/intern/lib/abi/abi_system.c.md`, *observe*. |
| `matched` | Its terms, as storage holds them. Null with no terms. |
| `spans_at` | Where its spans begin in the private memory of its module, as storage last wrote them. |
| `asked` | What it asked storage for, kept to put a system back as it was after a reload took it out of storage. |
| `events` | The events that an observer asks for. |
| `terms` | How many terms it has. |
| `fn` | Its function. |
| `level` | Its level in its phase: a level runs before the next. |
| `parked` | Whether it is parked: `docs/intern/lib/schedule/schedule_edges.c.md`, *The systems of a phase*. |
| `observer` | Whether it is an observer. |
| `pending`, `pending_count`, `pending_room` | Who triggered it since the last barrier, how many, and the room of the list. |
| `place` | Its place in its phase, from 0, at its dispatch: `docs/intern/lib/schedule/schedule_jobs.c.md`, *The turn of a system*. |
| `turn` | Where it comes among all that ran before the barrier. |
| `failed`, `why` | How its dispatch ended, said after the join. |
| `failures` | Its own count of failures. |
| `changes`, `change_count` | The components that it stated that it attaches or detaches. They lie after the terms, in the same block. |
| `slot` | Its slot among the systems of its module, or among its observers: what `add_system` or `observe` gave. |
| `stated_before` | The order that its module states, as bits: `docs/intern/lib/schedule/schedule_before.c.md`, *The bits*. They lie after the changes, in the same block. |
| `term` | Its `terms` terms, as the conflict graph reads them. |

### A module

| Field | Content |
| --- | --- |
| `b` | The world. |
| `identity`, `ns`, `stem` | Its identity, its namespace, and its identity without the version. |
| `on`, `image`, `instance` | Its backend, its loaded image and its instance. |
| `memory` | Its private block, as its backend sees it. |
| `priv`, `priv_len`, `priv_reserve` | The private block, its length, and the address space reserved for it. |
| `scratch`, `scratch_len` | Where the scratch arena is, and its length. |
| `ring`, `ring_len`, `ring_tail` | The start of the log ring, how many records it holds, and how many records were drained. |
| `spans`, `spans_len` | The span region: the query slots, the lists of the observers, `column_base` and the spans. |
| `had_triggers`, `spans_end` | What storage last wrote in the span region: that the list of an observer was not empty, and where the spans end. |
| `declared`, `policy`, `may_write`, `declared_count` | By slot: the component, what the module stated for unload, and whether its grants let it write the component. |
| `systems`, `system_count`, `observers`, `observer_count` | Its systems and its observers, by slot. |
| `work_limit` | The work limit set before each dispatch. |
| `ids`, `id_count`, `id_next`, `id_room` | The ids that it may hand out, taken from storage at a barrier: the block, how many, the next, and the room. |
| `ids_short` | That the entity ceiling cut its last refill short. |
| `spawns`, `persistent` | What its config states for `spawns` and `persistent`. |
| `arena_at`, `arena_left`, `arena_share` | Its share of the persistent arena: where `alloc` takes next, what is left of the range, and the size of the share. |
| `owned`, `own_count`, `own_next` | The ranges of the arena that are its share, how many, and the next that `alloc` moves to. |
| `queue`, `queue_len`, `queue_room` | What it asked to change, applied at the next barrier in the order it asked, and the bytes used and stated. |
| `read`, `write`, `read_count`, `write_count` | Its grants. |
| `phase_from`, `phase_to`, `phase_names` | The phases that it names and the deployment calls otherwise, and which phase of the deployment each is. |
| `pinned`, `thread` | Whether it is pinned, and the thread that loaded it. |
| `hot_reload`, `on_error` | What its config states for them. |
| `shutdown`, `shutdown_timeout` | Its export `shutdown`, and the timeout of that call. |
| `max_failures`, `failures` | The `max_system_failures` of its config, and the count of the failures that are no system's. |
| `changed_now`, `fault_now` | At this barrier: it changed an entity, and the words of its fault, or null. |
| `reached` | The system whose count reached the number, if one did. |
| `left` | How many of its changes this barrier did not apply, by cause: no table, or no columns. |
| `writable` | The types that it may write, as storage numbers them. |
| `barred` | How many entities it asked to destroy at this barrier that carry none of those types. |
| `config`, `text_at`, `text_room` | The component that its settings seeded, and the place and the room of its settings text. |
| `in_init`, `stopped` | It is in its first call, and it is stopped. |
| `running`, `turn` | What runs in it now, or null for a lifecycle call, and the turn of that call. |
| `marks`, `mark_count`, `mark_room`, `mark_at` | Where the changes of each turn begin in its queue, and the next mark that the barrier takes. |
| `reloads` | The instance that this one is to take the place of. |
| `lender` | The new record, during the `shutdown` of this old instance in a reload: `spawn` and `alloc` take from it. Else null. |
| `losses`, `loss_count` | What a reload accepts to lose. |
| `loss_rows` | By loss: how many rows lost something under that name, plus one. Zero if none did. |
| `refused`, `why` | An entry refused during the call in progress, and why. |
| `next` | The next loaded module. |

The library keeps here each offset that it acts on, and never reads the offset back from the header.
`docs/intern/lib/block/block_log.c.md`, *A line handed to the sink*, tells why.
`declared`, `policy` and `may_write` are found once, at `declare`, and each attach reads them.

Only the thread of the module touches what is set aside for it alone: its ids, its share of the arena and its queue.
Each system has its own count of failures, and `failures` counts what is no system's.
A barrier that could not place an entity that the module changed raises it, and a barrier that placed all it changed lowers it.

With `marks`, the barrier can apply the queues in order: `docs/intern/lib/storage/storage_runs.h.md`, *The order of the queues*.

A reload can give the share one more range: `docs/intern/lib/reload/reload_share.c.md`, *A share that a reload grows*.
`alloc` moves to the next range as `docs/intern/lib/abi/abi_alloc.c.md`, *The persistent arena*, tells.
At unload, the ranges go back to the arena if nothing that stays points into them: `docs/intern/lib/module/module_share.c.md`, *When a share goes back*.

### A queued change

`change_t` is one change in the queue of a module: `id`, the component `c`, the `size` of the value, and the operation `op`.
`size` bytes of value follow it, padded to a multiple of eight.
A change is 24 bytes.

### A matrix of reach

Bit `j` of row `i` of a `schedule_matrix_t` says that `i` runs before `j`.

| Field | Content |
| --- | --- |
| `bits` | The rows, one after the other. |
| `n` | How many systems or names. |
| `words` | How many 64-bit words a row has: `(n + 63) / 64`. |

### The pairs of a world

A `schedule_pairs_t` holds the pairs of `[order]`: `docs/intern/lib/schedule/schedule_order.c.md`.

| Field | Content |
| --- | --- |
| `names` | Each name of a pair, copied, in the order met. |
| `n` | How many names. |
| `reach` | Who runs before whom among the names, closed at create: `docs/intern/lib/schedule/schedule_order.c.md`, *The pairs*. |

### A world

| Field | Content |
| --- | --- |
| `sink`, `ud` | The sink of the host, and what it is handed with each record. |
| `caller` | The id of the thread whose call of the world runs, or 0: `docs/intern/lib/bridge/bridge_check.c.md`, *A call from another thread*. |
| `backends`, `backend_count` | The backends of the deployment, opened. |
| `reservation`, `guard`, `granule` | The shape of the shared block: the largest that any backend stated. Each block is allocated with this address space. |
| `stopped_by`, `stopped_stem` | Why nothing runs any more, and the stem of the module whose return lifts the stop. Null while the world runs. |
| `stopped_why`, `stopped_name` | The two arrays that `stopped_by` and `stopped_stem` point at. |
| `phases`, `phase_count` | The phases of the deployment, by name. |
| `barriers` | The count of barriers that ran. |
| `turns` | How many lifecycle calls ran since the last barrier. |
| `parked` | The systems out of storage while their module reloads. |
| `arena_ranges` | How many ranges of the arena the modules hold. |
| `run_jobs`, `run_handed`, `run_lists`, `run_callers` | What the dispatch of a phase works in, `[budget] systems` long. |
| `build_in`, `build_bits` | What the schedule is made in, when a module comes or goes. |
| `order` | The names of the pairs of `[order]`, and their closed matrix. |
| `pool` | The threads that run a phase. |
| `shared`, `shared_len`, `shared_reserve` | The shared block, its length, and the address space reserved for it. |
| `index`, `entities` | The entity index in the shared block, and `[budget] entities`. |
| `arena`, `arena_len` | Where the persistent arena begins, and its length. |
| `columns`, `columns_len` | Where the column region begins, and its length. |
| `sparse_sets`, `sparse` | `[budget] sparse_sets`, and where their indexes begin: one after another, each a `u32` for each entity there may be. |
| `direct_indexed`, `direct` | `[budget] direct_indexed`, and where their bits begin: one after another, each a bit for each entity there may be. |
| `arena_space`, `column_space` | What of the arena and of the column region is handed out. |
| `layout_moved` | Set when a module comes or goes, or when a declaration changes. |
| `tables` | `[budget] tables`: how many tables may exist at one time. |
| `max_types`, `max_systems`, `max_terms` | `[budget] types`, `systems` and `terms`. |
| `storage` | What keeps the rows: `src/lib/storage/storage.h`. |
| `types`, `type_count` | The declared types. |
| `kept`, `kept_count` | The shares that stay after their module unloaded: `docs/intern/lib/config/config_kept.c.md`, *The list of shares*. |
| `systems`, `system_count` | The systems of every module, in the order of registration: `docs/intern/lib/abi/abi_subscribe.c.md`, *The slot*. |
| `modules`, `module_count`, `max_modules` | The loaded modules, how many, and `[budget] modules`. |

A world is stopped at a barrier, where nothing is allocated.
So `stopped_by` and `stopped_stem` point at the two arrays after them.
The list of gaps of the arena has room for one gap more than `arena_ranges`, made when a range is taken.
So a range given back never asks for memory.
`docs/intern/lib/schedule/schedule.h.md`, *Cost*, tells why a load, a reload and an unload cannot fail for the room of the schedule.

`docs/intern/lib/block/block_layout.c.md`, *The parts*, gives the parts of the shared block, and where each begins.

## Constants

| Constant | Value |
| --- | --- |
| `CHANGE_DESTROY` | 1: a queued `destroy`. |
| `CHANGE_ATTACH` | 2: a queued `attach`. |
| `CHANGE_DETACH` | 3: a queued `detach`. |
| `TURN_LIFECYCLE` | `1 << 32`: the turns of the lifecycle calls. |
| `TURN_SYSTEM` | `2 << 32`: the turns of the systems of a phase. |
| `NAME_MOST` | 127: the longest name, in bytes, that a declared type, a system or an observer can have. |

A turn orders all the calls that ran before a barrier.
The observers of the last barrier come first, then the lifecycle calls, then the systems of the phase, each in the order it ran.
The low 32 bits of a turn give that order.

## Functions

| Function | Source | Result |
| --- | --- | --- |
| `module_discard` | `src/lib/module/module_discard.c` | Frees a module: `docs/intern/lib/module/module_discard.c.md`, *Functions*. |
| `module_shut_down` | `src/lib/module/module_call.c` | Runs the `shutdown` of a module: `docs/intern/lib/module/module_call.c.md`, *Functions*. |
| `module_free_type` | `src/lib/module/module_types.c` | Frees a declared type: `docs/intern/lib/module/module_types.c.md`, *Functions*. |
