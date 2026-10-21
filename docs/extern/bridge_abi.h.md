# include/bridge_abi.h

`include/bridge_abi.h` is the module seam: Bridge and each module compile it.
It gives the eight entries that a module calls, and the layouts that a module reads with plain loads.

## The layouts

The header tells how to find data, and never what data there is: it holds no component and no field of a component.
A module reads the structs of the header with plain loads.
Thus each field has a fixed width, and the header asserts the size and the offset of each field.
A module and Bridge that disagree about a layout do not both compile.

The compiler adds no padding: a gap is a field `pad`. Write zero into it. Nobody reads it.

An enum has the width of its place: 1 byte in `bridge_term_t`, and 4 bytes in each other struct.
A module in another language cannot include this header. It needs the same layouts, written in its language.
A module that compiles with `include/bridge_abi.h` is a work based on Bridge: `readme.md`, *License*.

## BRIDGE_ABI

`BRIDGE_ABI` is the number of the module seam: the version of the layouts and the entries of this header.
A module states the number that it was built against.
The host gives it in `abi` of `bridge_module_config_t`: `docs/extern/bridge.h.md`, *bridge_module_config_t*.
A load refuses a module of another number. `abi` of `bridge_header_t` holds the number of Bridge.
The number changes as `BRIDGE_HOST_ABI` does: `docs/extern/bridge.h.md`, *BRIDGE_HOST_ABI*.

## The exports

A module is code that Bridge runs through a backend. How to write a module for a backend is the business of that backend.
A module exports these functions. Each one takes nothing and gives an `int32_t` status: 0 is a success, and each other value is a failure.

| Export | When Bridge calls it |
| --- | --- |
| `init` | One time, in `bridge_load`. It declares the types, registers the systems and the observers, and can make entities. |
| `reload` | In `bridge_reload`, in the new instance. It declares and registers as `init` did, and the storage stays. |
| `shutdown` | In `bridge_unload` and in `bridge_destroy`, and in `bridge_reload` for the old instance. |
| A system | One time in each run of its phase. |
| An observer | At a barrier, if an entity triggered it. |

Each module exports `init`, `shutdown` and `reload`. A load fails if one of them is missing.
A status that is not 0 and a call that did not return are the same failure.

No export runs for each frame. A thing to do in each frame is a system.
A module does not call another module. It writes data, and the module that subscribes to that data acts.

## The memories

A module imports each thing from the namespace `BRIDGE_NAMESPACE`, `bridge`. It imports two memories by name:

| Constant | Name | Memory |
| --- | --- | --- |
| `BRIDGE_SHARED` | `shared` | The shared memory of the world: the entity index, the components and the persistent arena. Each module reads it. |
| `BRIDGE_PRIVATE` | `private` | The private memory of the module: its header, its scratch arena, its log ring and its span region. |

A module gets only the memories that it imports. Only the module and Bridge read its private memory.
An offset into a memory counts from the start of that memory.
Offset 0 of the shared memory is reserved, and its first 8 bytes hold nothing. Thus an offset of 0 says absent.

The private memory holds, in this order: the header at offset 0, the scratch arena, the log ring and the span region.
The config of the module sizes them: `docs/extern/bridge.h.md`, *The private memory*.

## bridge_header_t

`bridge_header_t` is at offset 0 of the private memory, and tells where each region of the module begins.
Bridge writes it. The module writes only `scratch_used` and `ring_head`.

| Field | Content |
| --- | --- |
| `abi` | The `BRIDGE_ABI` of Bridge. |
| `scratch` | Where the scratch arena begins. |
| `scratch_len` | The size of the scratch arena, in bytes. |
| `scratch_used` | How many bytes of the scratch arena are in use. The module adds to it, and each barrier sets it to 0. |
| `ring` | Where the log ring begins. |
| `ring_len` | How many records the log ring holds. |
| `ring_head` | How many lines the module wrote since the instance started, the dropped lines included. |
| `ring_tail` | How many records Bridge read since the instance started. |
| `queries` | Where the query slots begin: a `bridge_query_t` for each slot that `add_system` gave. |
| `column_base` | Where `column_base` begins: a `uint32_t` for each table and each type that the module declared. |
| `types` | How many types the module declared. |
| `tables` | How many table ids there are, 0 included. |
| `entity_index` | Where the entity index begins, in the shared memory. |
| `entities` | How many items of `bridge_entity_t` the entity index has. |
| `barriers` | How many barriers the world made before it dispatched the system that runs. |
| `place` | The place of the system that runs in its phase, from 0. |
| `system` | The slot of the system that runs, among the systems or the observers of the module. |
| `observer` | 1 if an observer runs, and 0 if a system runs. |
| `log_level` | The `log_level` of the config of the module: `docs/extern/bridge.h.md`, *bridge_module_config_t*. |
| `observers` | Where the observer slots begin: a `bridge_list_t` for each slot that `observe` gave. |
| `kept` | Where the kept cells begin: a `uint32_t` for each type that the module declared. |
| `rows` | Where a `uint32_t` for each table begins: how many rows the table has. |
| `blocks` | Where a `uint32_t` for each table begins: where the ids of the rows of the table are, in the shared memory. |
| `stride` | Where a `uint32_t` for each declared type begins: the size of the type, in bytes. |
| `write` | Where a `uint32_t` for each declared type begins: 1 if the module can write the type, and 0 if not. |
| `kind` | Where a `uint32_t` for each declared type begins: the storage layout of the type, an item of `enum bridge_storage`. |
| `owned` | Where the owned ranges of the module begin: a count, and then that many ranges of the shared arena. |
| `arena_lo` | The low bound of the shared arena that the module can read, in the shared memory. |
| `arena_hi` | The high bound of the shared arena that the module can read, in the shared memory. |

Bridge writes `barriers`, `place`, `system` and `observer` before each dispatch.
`system` is `0xFFFFFFFF` and `observer` is 0 while no system runs, for example in `init`.

The arrays from `column_base` to `kind` make the capability table.
A module names a cell by a coordinate: a slot, a place, and an offset in the cell.
The place is a table and a row, or an entity id, or nothing for a singleton.
`column_base`, `kept`, `rows`, `blocks`, `stride`, `write` and `kind` turn the coordinate into an offset into the shared memory.
A backend that does not trust a module reads a cell only through them, and stops a module that reaches past what it declared.

`owned` lists the ranges of the shared arena that the module can write, up to `BRIDGE_OWNED_CAP` of them.
The count comes first, and then that many pairs of an offset and a length.
`arena_lo` and `arena_hi` bound the bytes of the shared arena that the module can read.

## The scratch arena

The scratch arena holds short-lived bytes: a value for an entry, the text of a log line, a name.
A module puts bytes above `scratch_used`, and adds their size to `scratch_used`.
Each barrier sets `scratch_used` to 0. Thus an offset into the scratch arena is good until the end of the phase.
Each address that an entry gets is an offset into the private memory, in the scratch arena.
A module writes the bytes there, and gives where they are.

An entry reads what it gets during the call. Thus a module can put a value above `scratch_used` for one call, and not count it.
The text of a log line stays until the barrier. Count it in `scratch_used`.

## The log

A module writes a line of its log into its log ring, with no call. Each barrier gives the lines to the sink of the host.
`enum bridge_log` names four of the eight levels of a line, with the numbers of `enum bridge_level`: `docs/extern/bridge.h.md`, *bridge_record_t*.

| Item | Value |
| --- | --- |
| `BRIDGE_LOG_ERR` | 3: an error. |
| `BRIDGE_LOG_WARN` | 4: a warning. |
| `BRIDGE_LOG_INFO` | 6: information. |
| `BRIDGE_LOG_DEBUG` | 7: detail. |

A line is a `bridge_log_record_t` of 32 bytes:

| Field | Content |
| --- | --- |
| `barriers` | `barriers` of the header, when the module wrote the line. |
| `place` | `place` of the header, when the module wrote the line. |
| `system` | `system` of the header, when the module wrote the line. |
| `observer` | `observer` of the header, when the module wrote the line. |
| `level` | The level of the line: 0 to 7. Lower is worse. Bridge gives a level above 7 to the sink as 7, with the text of the line. |
| `pad` | 2 bytes of padding. |
| `text` | Where the text of the line is: an offset into the private memory, in the scratch arena. |
| `len` | How many bytes the text has. |
| `arg` | Two numbers: a value, and what it is set beside. |

To write a line, do these steps:

1. If the level is above `log_level` of the header, stop.
2. Keep `ring_head` as the head, and write the head plus 1 into `ring_head`.
3. If the head minus `ring_tail` is `ring_len` or more, the ring is full: stop.
4. Put the text in the scratch arena, and count it in `scratch_used`.
5. Write the record at the index of the head modulo `ring_len`, in the ring.

A full ring drops the line, and the next barrier tells the host how many lines it dropped.
A line whose text is not in the scratch arena gets a text of Bridge: *The log line*.

## bridge_entries_t

`bridge_entries_t` gives the names and the signatures of the eight entries that a module calls.
It is no layout: no memory holds it.
A backend gives the entries to a module under these names, in the namespace `BRIDGE_NAMESPACE`.
How a module reaches an entry, for example as an import or in a table, is the business of the backend.

| Entry | Parameters | Result | When |
| --- | --- | --- | --- |
| `declare` | `at` | The slot of the type. | In `init` and `reload`. |
| `add_system` | `at` | The slot of the system. | In `init` and `reload`. |
| `observe` | `at` | The slot of the observer. | In `init` and `reload`. |
| `spawn` | None. | The id of a new entity. | At any time. |
| `destroy` | `id` | None. | At any time. |
| `attach` | `id`, `slot`, `ptr` | None. | At any time. |
| `detach` | `id`, `slot` | None. | At any time. |
| `alloc` | `len` | An offset into the persistent arena. | At any time. |

`at` is where the struct of the entry is, in the scratch arena. An id has 64 bits, and each other value has 32 bits.
A slot is the number of a type, a system or an observer among those of the module, from 0.
The barrier after the call applies `destroy`, `attach` and `detach`. No entry changes the storage at once.

An entry that cannot do what it gets refuses the call. Then the invocation that made the call fails.
The first refusal of an invocation is its reason: *Messages*.
On a backend that can stop the module, a refused call does not return.
On another backend, the call returns, and the invocation fails when it ends.

## declare

`declare` declares a type: a component, or a struct, an enum or a bitmask that a field names.
It gets `at`: where a `bridge_declaration_t` is, in the scratch arena. It gives the slot of the type.

| Field | Content |
| --- | --- |
| `name` | Where the name of the type is, in the scratch arena. |
| `name_len` | How many bytes the name has: 1 to 127. |
| `kind` | What the descriptor describes: an item of `enum bridge_kind`. |
| `size` | The size of the type in bytes: `sizeof` of the type. |
| `align` | The alignment of the type: `alignof` of the type. |
| `desc` | Where the descriptor of the type is, in the scratch arena. |
| `desc_len` | How many bytes the descriptor has, with no zero at its end. |
| `on_unload` | What the last unload of the type does to its entities: an item of `enum bridge_on_unload`. |
| `storage` | The storage layout of the type: an item of `enum bridge_storage`. |

Zero in `kind`, `on_unload` or `storage` is unset. `declare` refuses it, and each value that is not an item.
The name of a type has a namespace: `game:Position`. *Grants* tells which namespaces a module can declare.
`docs/extern/bridge_schema.h.md` tells how to write a descriptor.
Bridge lays the type out from its descriptor, and refuses a type whose `size` or `align` is different.

`declare` is idempotent. The first module that declares a type makes it.
A later module gets a slot of its own for the same type.
Each module that declares a type states the same descriptor, the same kind and the same storage layout.
`declare` refuses a different one, but at a reload: *When a type changes*.
Bridge compares the descriptors byte for byte: `docs/extern/bridge_schema.h.md`, *Layouts*.

## bridge_kind

`enum bridge_kind` tells what a declared type is.

| Item | Kind |
| --- | --- |
| `BRIDGE_STRUCT` | A struct. |
| `BRIDGE_ENUM` | An enum. |
| `BRIDGE_BITMASK` | A bitmask. |
| `BRIDGE_TAG` | A tag: a component with no bytes. Its `size` is 0, and it has no descriptor. |

A tag tells that an entity is one of something, and has no value. Standard C has no struct with no member, thus a tag is a kind of its own.
Each storage layout can keep a tag. A module gives, takes, matches and observes a tag as a component with bytes.

`BRIDGE_CARRIED`, 1, is what a module gets for a tag where it gets the place of a value.
It is the base in a span, the cell in `column_base`, and the cell in the index of a sparse set.
A tag that is a singleton has it in its kept cell while it is set.
1 is not zero, which says absent, and is the place of no value.
A toggled tag gets where its column would begin, and a direct-indexed tag where its cells would begin: a module finds its bits from there.

## bridge_on_unload

`enum bridge_on_unload` tells what happens to the entities of a component when the last module that declares it unloads.

| Item | The component goes, and |
| --- | --- |
| `BRIDGE_STRIP` | The entity stays, also with nothing on it. |
| `BRIDGE_SWEEP` | Bridge destroys an entity with nothing left on it. |
| `BRIDGE_DELETE` | Bridge destroys each entity that carried the component, with each other component on it. |

The policy of the module that unloads last applies. A component that another loaded module declares stays, with its rows.
A strip takes no memory and does not fail. A strip that has no room to join two tables leaves the two: `docs/extern/bridge.h.md`, *An unload*.
An id of an entity that a sweep or a delete destroyed is stale: *bridge_entity_t*.
An entity that must outlive the unload of another module carries a component of the module that holds it.

## Storage layouts

`enum bridge_storage` is the storage layout of a component: how Bridge keeps it.
The author states it, since the code that reads the component depends on it.

| Item | The component is | Use it for |
| --- | --- | --- |
| `BRIDGE_TABLE` | A column of the table of the archetype of its entity. | What systems read together, row by row. |
| `BRIDGE_SPARSE_SET` | An array of its own, with an index by entity. It is in no archetype. | What comes and goes, and what few entities carry. |
| `BRIDGE_TOGGLE` | A column of a table, and a bit for each row that tells whether the row is on. | What an entity always has, and sometimes does not use. |
| `BRIDGE_DIRECT_INDEXED` | A cell for each entity. It is in no archetype. | What a module reaches by id much more often than it walks. |
| `BRIDGE_SINGLETON` | One value for the deployment, on no entity. | One value, for example the settings of a module. |

In this section, `e` is the low 32 bits of the id of an entity. Each offset is an offset into the shared memory.

- A table: `column_base` gives where the column is.
- A sparse set: the kept cell holds where its index is: a `uint32_t` for each `e`, that holds where the value is, or 0.
- A toggle: the `uint32_t` 16 bytes before the column holds where its bits are. Bit `r` tells whether row `r` is on.
- Direct-indexed: the kept cell holds where the cells are, and cell `e` is the value of the entity.
- Direct-indexed: the `uint32_t` 16 bytes before the cells holds where its bits are. Bit `e` tells whether the entity has a value.
- A singleton: the kept cell holds where its value is, or 0 while it has none.

The bits are 64-bit words: bit `n` is bit `n` modulo 64 of word `n` divided by 64.
A system turns a row of a toggle on or off: it writes the bit, with a term of `BRIDGE_WRITE`. No observer hears it.

An attach gives a toggle that is on. An attach of a toggle that the entity carries writes the value, and keeps the bit.
`attach` and `detach` give and take a singleton with the id 0. A system writes a field of a singleton where it is, with a term of `BRIDGE_WRITE`.

The cells of a direct-indexed component take its size for each entity of `entities`, from `columns`, at `declare`.
A singleton takes its size from `columns`, at `declare`.
A system cannot walk a singleton or a direct-indexed component: *bridge_term_t*.

## Grants

The name of a component has a namespace, for example `mymod:Explosion`.
A module has the namespace that its identity names: `docs/extern/bridge.h.md`, *bridge_module_config_t*.
A module can always declare, read and write a component of its own namespace.
For a component of another namespace, it needs a grant: the name of the component in `read` or `write` of its config.
`read` lets the module declare the component and read it. `write` also lets it write, attach and detach it.

Bridge checks a grant when a module declares a type, registers a system or an observer, and calls `attach` or `detach`.
A module can destroy an entity that carries a component that it can write, or that carries nothing: *destroy*.
An id that `spawn` has not handed out yet is no entity: the entity index does not show it, and no module can destroy it.
The memory does not enforce a grant: a module can write what it reads. That breaks a rule: *Rules*. The confinement does not catch it: `docs/extern/bridge.h.md`, *Trust and isolation*.

## add_system

`add_system` registers a system: a query, a function, a phase, and the systems that run before it.
It gets `at`: where a `bridge_system_statement_t` is, in the scratch arena. It gives the slot of the system.

| Field | Content |
| --- | --- |
| `name` | Where the name of the system is, in the scratch arena. The module exports a function of that name. |
| `name_len` | How many bytes the name has: 1 to 127. |
| `phase` | Where the name of the phase of the system is, in the scratch arena. |
| `phase_len` | How many bytes the name of the phase has. |
| `terms` | Where the terms of the query are, in the scratch arena: `term_count` items of `bridge_term_t`. |
| `term_count` | How many terms the query has. |
| `changes` | Where the slots of the components that the system attaches or detaches are: `change_count` items of `uint32_t`. |
| `change_count` | How many slots `changes` holds. |
| `before` | Where the slots of the systems that run before this one are, in the scratch arena: `before_count` items of `uint32_t`. |
| `before_count` | How many slots `before` holds. |

The name of a phase is the name that the module uses: `docs/extern/bridge.h.md`, *bridge_phase_name_t*.
A system with no term runs one time in each run of its phase.

Each slot of `before` is a slot that `add_system` gave to an earlier system of the module, in the same phase: *Conflicts*.
`before` holds at most as many slots as the module has earlier systems. A slot can come two times.
If `before_count` is 0, Bridge does not read `before`.

`init`, `shutdown` and `reload` are reserved names. Each system and observer of a module has a different name.
The name of a system or of an observer has no `@` and no `:`, so that its full name has one meaning: `docs/extern/bridge.h.md`, *bridge_order_t*.

A system that attaches or detaches a component states it in `changes`.
An `attach` or a `detach` of a component that the system did not state fails its invocation.
No statement names `destroy`. A lifecycle call states nothing, since it runs alone.

## observe

`observe` registers an observer: a query, a function, and the events that it hears.
It gets `at`: where a `bridge_observer_statement_t` is, in the scratch arena. It gives the slot of the observer.

| Field | Content |
| --- | --- |
| `name` | Where the name of the observer is, in the scratch arena. The module exports a function of that name. |
| `name_len` | How many bytes the name has: 1 to 127. |
| `events` | The events that the observer hears: a mask of items of `enum bridge_event`. |
| `terms` | Where the terms of the query are, in the scratch arena: `term_count` items of `bridge_term_t`. |
| `term_count` | How many terms the query has: one at least. |
| `changes` | Where the slots of the components that the observer attaches or detaches are: `change_count` items of `uint32_t`. |
| `change_count` | How many slots `changes` holds. |
| `before` | Where the slots of the observers that run before this one are, in the scratch arena: `before_count` items of `uint32_t`. |
| `before_count` | How many slots `before` holds. |

An observer has no phase: it runs at the barrier that applied its trigger, after the barrier wrote the spans.
Each slot of `before` is a slot that `observe` gave to an earlier observer of the module: *Conflicts*.
`before` holds at most as many slots as the module has earlier observers. A slot can come two times.
If `before_count` is 0, Bridge does not read `before`.
`enum bridge_event` tells what a barrier made of an entity, against the terms of the observer:

| Item | The entity |
| --- | --- |
| `BRIDGE_ON_ENTER` | Did not match the terms, and now matches them. |
| `BRIDGE_ON_ATTACH` | Matches the terms, and the barrier applied an attach of one of them. |
| `BRIDGE_ON_LEAVE` | Matched the terms, and no longer matches them, or was destroyed. |

An observer of A and not B hears an entity with A enter when a module detaches B, and leave when a module attaches B.
A write in place raises no event. To have a new value heard, attach it.
An entity that a phase made and destroyed raises nothing.
An entity that loses a component and gets it again in one phase raises `BRIDGE_ON_ATTACH`, since it carries a new value.
A detach of what is not there, and a change of a dead id, raise nothing.

`observers` of the header is where the observer slots begin: a `bridge_list_t` for each slot.

| Field | Content |
| --- | --- |
| `count` | How many items of `bridge_trigger_t` the observer has at this barrier. |
| `at` | Where the first item is, in the private memory. |

Each `bridge_trigger_t` is one entity that triggered the observer:

| Field | Content |
| --- | --- |
| `id` | The id of the entity. It is 0 for a singleton. |
| `events` | Each event that holds for the entity at this barrier: a mask of items of `enum bridge_event`. |
| `pad` | 4 bytes of padding. |

Bridge calls an observer one time at a barrier, and only if its list is not empty.
An observer reads ids, and not rows: *bridge_entity_t*. An id that names nothing was destroyed.
What an observer queues applies at the next barrier. Thus a chain of three observers takes three barriers.
A list that does not fit in the span region of the module stops the module.

## bridge_term_t

A term is one component that a query asks for: a `bridge_term_t`.

| Field | Content |
| --- | --- |
| `slot` | The slot of the component, as `declare` gave it. |
| `access` | Whether the system reads or writes the component: an item of `enum bridge_access`. |
| `oper` | How the term matches: an item of `enum bridge_oper`. |
| `pad` | 2 bytes of padding. |

| Item | Meaning |
| --- | --- |
| `BRIDGE_READ` | The system reads the component. |
| `BRIDGE_WRITE` | The system writes the component. The module needs a grant to write it. |
| `BRIDGE_AND` | The entity must carry the component. |
| `BRIDGE_NOT` | The entity must not carry the component. |
| `BRIDGE_OPTIONAL` | The entity can carry the component. |

Zero in `access` or `oper` is unset. Bridge refuses it, and each value that is not an item.
The scheduler reads `access` to know which systems can run at the same time. Nothing else reads it.
Each term is a component that the matched entity itself carries.

A system walks the rows of tables, or the entries of one sparse set alone. Thus:

- Each term of `BRIDGE_AND` or `BRIDGE_NOT` names a component of a table or a toggle, or
- One term of `BRIDGE_AND` names a component of a sparse set, and each other term is `BRIDGE_OPTIONAL`.
- A term of a singleton or of a direct-indexed component is `BRIDGE_OPTIONAL`.

Bridge refuses another term at `add_system`, by its number. An observer walks nothing, thus these rules do not apply to it.
In a walk of tables, a component of a sparse set has a base of 0. The module reaches it by the id of the row, through its kept cell.

## Conflicts

Two systems conflict when one writes a component that the other reads or writes, as their terms tell.
They also conflict when both attach or detach one component, as their changes tell.
A term of `BRIDGE_NOT` touches nothing.
Systems of different modules that do not conflict can run at the same time. Systems that conflict never do.

Two systems of one module that conflict in one phase need an order that their module states in `before`.
Without it, the load fails, also if a pair of the deployment orders them.
`before` makes chains: if `c` names `b`, and `b` names `a`, then `a` runs before `c`.

Two systems of different modules that conflict in one phase need a pair of the deployment, or a chain of pairs: `docs/extern/bridge.h.md`, *bridge_order_t*.
Else the load that brings the second one fails. `before` does not order such a conflict.
Two observers that conflict at the barrier follow the same rules.

In one phase, an order that `before` or a pair states always holds, also between two systems that do not conflict.
If the pairs and `before` make a cycle, the load or the reload that closes it fails.
In the schedule, `before` and the pairs make chains together.
An unload can take such a chain away, but never the order of two systems that conflict.
A barrier applies the changes in the order that the systems ran. Of two attaches of one component to one entity, the later stays.

## Spans

At each barrier, Bridge writes into the private memory a span for each table that a query matches.
A span stays good until the next barrier, and it does not change before then.
`queries` of the header is where the query slots begin: a `bridge_query_t` for each slot of a system.

| Field | Content |
| --- | --- |
| `spans` | How many spans the query has: how many tables it matches. |
| `at` | Where the first span is, in the private memory. The spans follow one another. |

Bridge keeps its own copy of each slot, and does not read a slot back.
A module that writes a slot changes only what it reads.

A span has `BRIDGE_SPAN_SIZE` bytes, which is 8. It names one table that the query matches:

| Offset | Content |
| --- | --- |
| 0 | A `uint32_t`: the id of the table. |
| 4 | A `uint32_t`: how many rows the table has. |

A span no longer holds a column. A module names a cell, and the capability table bounds the access.

To read a component of a row, name its slot, the table and the row, and the offset in the cell.
The cell is at `column_base[table * types + slot]` plus `row` times `stride[slot]`, plus the offset.
`blocks` gives the id of a row: the id of row `r` of table `T` is at `blocks[T]` plus `r` times 8.
A query whose one selecting term is a sparse set has one span: its rows are the entities of the set.

```pseudo
FOR s <- 0 TO query.spans - 1
   span <- query.at + s * BRIDGE_SPAN_SIZE
   table <- read_u32(span)
   rows <- read_u32(span + 4)
   base <- column_base[table * types + slot]
   IF base <> 0 THEN
      FOR r <- 0 TO rows - 1
         CALL use_value(base + r * stride[slot])
      NEXT r
   ENDIF
NEXT s
```

A barrier writes only what changed. The first barrier after the load of a module writes all of its spans.

`column_base` of the header is where a `uint32_t` for each table and each declared type begins.
The cell of table `T` and slot `s` is at `column_base + 4 * (T * types + s)`.
It holds where the column of the type is in that table: 0 if the table does not carry it, and `BRIDGE_CARRIED` for a tag.

`stride` holds the size of each type, and `rows` the row count of each table.
`write` tells whether the module can write a type, and `kind` gives its storage layout.
`kept` of the header is where a `uint32_t` for each declared type begins: *Storage layouts*. It is 0 for a type of a table.

## bridge_entity_t

An entity is an id: a `uint64_t` with an index in its low 32 bits and a generation in its high 32 bits.
An id outlives the row behind it, and a row does not outlive a barrier.
A new entity can take the index of a destroyed one, with another generation. Thus the old id names nothing.
`entity_index` of the header is where the entity index is, in the shared memory: `entities` items of `bridge_entity_t`.

| Field | Content |
| --- | --- |
| `table` | The table of the entity. 0 is no table: the entity carries no component of a table. |
| `row` | The row of the entity in its table. |
| `generation` | The generation of the entity that has this index. |

This function finds the value of a component of an entity, or 0:

```pseudo
FUNCTION find_value(id : U64, slot : U32) RETURNS U32
   DECLARE record : bridge_entity_t
   DECLARE base : U32
   record <- entity_index[id MOD 4294967296]
   IF record.generation <> id DIV 4294967296 THEN
      RETURN 0
   ENDIF
   base <- column_base[record.table * types + slot]
   IF base = 0 THEN
      RETURN 0
   ENDIF
   RETURN base + record.row * stride[slot]
ENDFUNCTION
```

A generation that is different tells that the entity is dead. A base of 0 tells that the entity does not carry the component.
To follow a reference costs two loads. For a relation that a system walks in each phase, use a query.
The entity index is a copy that Bridge writes and does not read.
A module that writes over it misleads the modules that read it, and changes nothing else.

## spawn

`spawn` gives the id of a new entity at once. The entity has no component until a barrier applies an attach to it.
The id comes from a block that a barrier set aside for the module: `spawns` of its config.
Thus a system can use the new id at once, for example in the value of a component that it attaches to another entity.

`spawn` refuses the call when the module states `BRIDGE_NONE` for `spawns`, when the block is used up, and when the world has no more entities.
A query can match the new entity from the next barrier on.
The entity index shows it from that barrier on too. Before it, the generation there does not match the id.

## destroy

`destroy` queues the destruction of the entity `id`. The barrier destroys it, with each component on it.
A destroyed entity stays readable for the rest of the phase.

The module must be able to write a component that the entity carries, or the entity must carry nothing.
The barrier checks it against what the entity carried before that barrier.
An id that a module holds and that `spawn` has not handed out yet cannot be destroyed.
Else the entity stays as it was, a record counts it, and the module fails at that barrier: `docs/extern/bridge.h.md`, *Failures*.
A destroy of a dead id is nothing.

## attach and detach

`attach` queues a component for an entity, with its value. `id` is the entity, and `slot` is the component.
`ptr` is where the value is, in the scratch arena: as many bytes as the type has. Bridge copies the value during the call.

`BRIDGE_ZEROED` as `ptr` asks for a value of zeros. A tag takes only `BRIDGE_ZEROED`.
An attach of a component that the entity carries writes its new value.

`detach` queues the removal of the component `slot` from the entity `id`.
A detach of what the entity does not carry is nothing.
An attach or a detach of a dead id is nothing.
An attach or a detach of an id that `spawn` has not handed out yet fails the module at the barrier.
The module never had that id.

For each of the two, `id` is 0 for a singleton, and not 0 for each other component.
The module needs a grant to write the component, and the system that runs must state it in its `changes`.

## alloc

`alloc` gives `len` bytes of the persistent arena, from the share of the module: `persistent` of its config.
It gives their offset into the shared memory. Bridge rounds `len` up to a multiple of 8.
Nothing frees the bytes while the module is loaded.
A component keeps a string in a field of the type `bridge_text_t`:

| Field | Content |
| --- | --- |
| `at` | Where the bytes are, in the persistent arena. |
| `len` | How many bytes the string has. |

A field can have this type in each namespace. No module declares it.
Bridge reads these fields at an unload. A share that holds a string of a row that stays, stays: `docs/extern/bridge.h.md`, *The reservations*.

To change a string, write a string that is not longer over the old bytes, and set `len`.
A longer string needs a new `alloc`, and the old bytes stay unused.
An offset that a module keeps in another place than a `bridge_text_t` is good only while the module is loaded.

## When a type changes

At a reload, the new instance can declare a type with another layout or with another storage layout.
If another loaded module declares the type, the reload fails, and the record names the other modules.
To change such a type, unload each other module that declares it, and reload this module. Then load the others, built for the new type.
If this module alone declares the type, Bridge moves its rows to the new layout: all of them, or none.
Bridge matches the fields by their names:

| Change | Result |
| --- | --- |
| A field added | Its bytes are zero. `reload` can give it a value. |
| Fields in another order | Each field goes to its new offset. |
| An array made longer | It keeps its values, and the rest is zero. |
| A field removed | A loss: its values go. |
| An array made shorter | A loss: its end goes. |
| A field renamed | A field removed, which is a loss, and a field added. |
| A field of another type | The reload fails. |
| A struct that a field names, changed | The same rules, one level down. |
| A constant of an enum or a bitmask gone, or with another value | The reload fails. A constant added is nothing. |
| An enum or a bitmask of another integer | The reload fails. |
| A type of another kind | The reload fails. |
| A type that a component of another module has a field of | The reload fails. |

Bridge also lays out again a component with a field of a type that changed.
A loss needs its name in `losses` of the config: `docs/extern/bridge.h.md`, *The losses*.
Else the reload fails, and the record names each loss and how many rows hold it.

A reload can also move a component to another storage layout:

| From, to | Result |
| --- | --- |
| A table, a sparse set, direct-indexed or a toggle, to one of the others | Each entity keeps its value. In a toggle, each row is on. |
| Any, to a singleton | With no entity, the singleton has no value. With one, it has its value. With more, the reload fails. |
| A singleton, to any other | The reload fails: no entity can carry the value. |
| A toggle, to any other | Bridge carries a row that is off as the others. That is a loss, named by the component alone. |

No observer hears a move, since no entity gains or loses a component.
While the rows move, the type counts two times against `types`, and needs room in two places. Without the room, the reload fails.
For a change that no rule covers, declare a component of a new name, and copy the values with a system.
Then do not declare the old component any more.

## Rules

A module keeps these five rules on each backend.
Bridge does not enforce them: a module that breaks one gets wrong values, and no record.

- Do not write a component through a term of `BRIDGE_READ`.
- Do not keep a reference into storage across a barrier: a row, a span or an address in a column.
- Do not keep an offset into the scratch arena across a phase, also the text of a log line.
- Do not rely on the order of the rows and the spans: it changes when entities come and go.
- Do not rely on which of two systems runs first, if nothing orders them.

## Messages

An entry that refuses a call writes one of these reasons. The record of the host gives it: `docs/extern/bridge.h.md`, *Messages*.
In `init`, the reason follows `did not load:`. In `reload`, it follows `was not reloaded:`. In a system, it follows `failed:`.

Angle brackets mark a part that changes: `docs/extern/bridge.h.md`, *Messages*.
For an observer, a reason says `observer` and `an observer` in place of `system` and `a system`.
Only a system names a phase, so a reason about a phase has no form for an observer.

### A declaration

`declare` writes one of these reasons when it refuses a type:

- `declare: what it states lies outside the scratch arena`
- `declare is called outside init`
- `declare: its name has <count> bytes, and a name has 1 to 127`
- `declare: its name has a zero byte`
- `declare: a name or a descriptor lies outside the scratch arena`
- `type '<type>' has no namespace`
- `type '<type>': its kind is unset`
- `type '<type>' is a tag, and a tag has no bytes and no descriptor`
- `it may not declare '<type>': that is another namespace's, and no grant names it`
- `type '<type>': on_unload is unset`
- `type '<type>': its storage layout is unset`
- `type '<type>' is one more than [budget] types of <count>`
- `type '<type>' is one more than [budget] sparse_sets of <count>`
- `type '<type>' is one more than [budget] direct_indexed of <count>`
- `type '<type>' is a singleton, and [budget] columns of <bytes> bytes has no room for its <bytes> bytes`
- `type '<type>' is direct-indexed, and [budget] columns of <bytes> bytes has no room for a cell of <bytes> bytes for each of <count> entities`
- `type '<type>' has another component layout here: fingerprint <fingerprint>, and it was declared with <fingerprint>`
- `type '<type>' is stated as <layout> here, and it was declared as <layout>`
- `out of memory: there was none for what it declares`
- `out of memory: there was none for type '<type>'`
- `out of memory: there was none for the descriptor of '<type>'`

`declare is called outside init` also tells a call outside `reload`.
When other modules declare the type, the reason of a layout ends with `. It is also declared by '<module>'`, and the other modules.
`docs/extern/bridge_schema.h.md`, *Messages* gives the reasons of a descriptor.

### At a reload

`declare` writes one of these reasons at a reload, when a type cannot change as the new instance states it:

- `type '<type>' has another component layout here, and '<component>', which module '<module>' declares, has a field of it`
- `type '<type>' is <kind> here and was <kind>: a reload does not change what kind of type one is`
- `type '<type>' is an integer of <bytes> bytes here and was one of <bytes>: every value kept of it would be read as another`
- `type '<type>' has no constant '<name>' here, which it had: a value kept of it would mean nothing`
- `type '<type>': its constant '<name>' is <number> here and was <number>, and every value kept of it would mean something else`
- `field '<path>' is of another type than it was, and bytes of one type are not a value of another`
- `type '<type>' has another component layout here, and <count> rows would lose <names>. A reload that accepts that states each among its losses`
- `type '<type>' is a singleton, and is not moved to <layout>: there is no entity to carry it`
- `type '<type>' is carried by <count> entities, and is not moved to a singleton: nothing says which of them has its value`
- `type '<type>' is a toggle that is off for <count> rows, and as <layout> they are carried like the rest. That is a loss this reload does not accept: '<type>'`
- `out of memory: there was none to work out how '<type>' is laid out again`

### add_system and observe

`add_system` and `observe` write one of these reasons when they refuse a system or an observer:

- `add_system: what it states lies outside the scratch arena`
- `add_system is called outside init`
- `add_system: a phase lies outside the scratch arena`
- `a system names phase '<phase>', which the deployment did not declare`
- `a system names phase '<phase>' as it is, and its config also maps '<name>' to that phase`
- `observe: what it states lies outside the scratch arena`
- `observe is called outside init`
- `observe: its events are unset, or name one there is not`
- `it registers one more than [budget] systems of <count>`
- `a system asks for <count> terms, which is more than [budget] terms of <count>`
- `system: its name has <count> bytes, and a name has 1 to 127`
- `system: its name has a zero byte`
- `system: a name, a term or a change lies outside the scratch arena`
- `a system states <count> changes, and the module declared <count> types`
- `'<name>' is a reserved name, and no system may take it`
- `'<name>' has '<character>' in it, and no system may have '@' or ':' in its name`
- `'<name>' is registered twice`
- `observer '<name>' has no term, and so nothing to observe`
- `change <index> of system '<name>' names slot <slot>, which it never declared`
- `system '<name>' may not attach or detach '<component>': no grant says so`
- `system: before lies outside the scratch arena`
- `a system states <count> systems before it, and the module registered <count>`
- `before <index> of system '<name>' names slot <slot>, which is not an earlier system`
- `before <index> of system '<name>' names system '<name>', which is of another phase`
- `out of memory: there was none for system '<name>'`

### A term

`add_system` and `observe` write one of these reasons when they refuse a term:

- `term <index> of system '<name>' names slot <slot>, which it never declared`
- `term <index> of system '<name>': its access is unset`
- `term <index> of system '<name>': its access is <number>, and there is read and write`
- `term <index> of system '<name>': its operator is unset`
- `term <index> of system '<name>': its operator is <number>, and there are three`
- `system '<name>' may not write '<component>': no grant says so`
- `term <index> of system '<name>': '<component>' is kept in a sparse set, and does not select the rows of a table. It is stated optional and looked for at each row, or it is the one term that selects`
- `term <index> of system '<name>': '<component>' is kept in a sparse set, and a system cannot walk the entities that are not in it`
- `term <index> of system '<name>': '<component>' is a singleton, and is on no entity a system walks. It is stated optional and read where it is`
- `term <index> of system '<name>': '<component>' is direct-indexed, and does not select what a system walks. It is stated optional and reached by the entity's id`

### A change

`spawn`, `attach`, `detach`, `destroy` and `alloc` write one of these reasons when they refuse a call:

- `spawn: it stated that it spawns nothing`
- `spawn: its spawns of <count> between two barriers are used up`
- `spawn: [budget] entities of <count> is reached`
- `attach names slot <slot>, which it never declared`
- `it may not attach '<component>': no grant says so`
- `system '<name>' may not attach '<component>': it is not among the changes it stated`
- `attach: '<component>' is a tag, and is handed a value`
- `attach: the value lies outside the scratch arena`
- `attach: '<component>' is a singleton, and its id is 0`
- `attach: the id 0 is no entity, and '<component>' is not a singleton`
- `destroy: the id 0 is no entity`
- `it stated that it queues no change`
- `its changes of <bytes> bytes are used up`
- `alloc: it stated that it keeps nothing in the persistent arena`
- `alloc: its persistent of <bytes> bytes is exhausted`

For `detach`, a reason says `detach` in place of `attach`.

### A barrier

A barrier writes this record when it refuses a destroy:

- `module '<module>' asked to destroy entities that carry nothing it may write. Left as they were: <count>`

The count includes each id that no `spawn` handed out yet.

A barrier writes this record when it refuses an attach or a detach of an id that no `spawn` handed out yet:

- `module '<module>' asked to attach or detach a component of an id that spawn never handed it. Left undone: <count>`

### The log line

The host gets this text in place of the text of a line whose text is not in the scratch arena:

- `(a log line whose text lies outside the scratch arena)`
