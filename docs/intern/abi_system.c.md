# src/abi_system.c

This file gives the entries by which a module registers a system and an observer, and what storage tells an observer.
`docs/intern/abi_subscribe.c.md` does the part that the two share.

## add_system

`add_system(at) -> slot` takes the offset of a `bridge_system_statement_t` in the scratch arena: the name, the phase, the terms, the changes and the systems before it.
It is refused outside the first call of the module: `init`, or `reload` at a reload.

A system states its phase, and in `before` the earlier systems of its module that run before it: `docs/intern/abi_before.c.md`.
It never names a system of another module.
A system takes no arguments: its spans are in the private memory of the module, found through the slot that `add_system` gives.

The phase is the name that the module uses: `docs/intern/abi_phase.c.md`.

## observe

`observe(at) -> slot` takes the offset of a `bridge_observer_statement_t`: the name, the events, the terms, the changes and the observers before it.
It is refused outside the first call of the module.
A system subscribes to a shape, and an observer subscribes to a change.
`events` is a mask of `BRIDGE_ON_ENTER`, `BRIDGE_ON_ATTACH` and `BRIDGE_ON_LEAVE`, and events that are none, or name one there is not, are refused.
An observer states no phase: its phase is the one past the last phase of the deployment, which is the barrier that applied its trigger.

A system that polls for a change costs a call in each pass, whether or not something changed.
An observer costs nothing when nothing changed, and that is why it is an entry of its own.

## The list of an observer

At a barrier, storage calls `abi_hear` for each change that an observer is owed: the id of the entity, and the event.
The change goes on the list of who triggered the observer since the last barrier.
An event of the same entity as the last record joins that record.

The list is as long as the spans of the module could publish of it.
One record past that is all there is to know: the count goes one past the room, and stops there.
`src/storage_publish.c` then finds that the list does not fit, and says so.

The observer runs at the barrier, after storage wrote its list: `docs/intern/block_barrier.c.md`, *The steps of a barrier*.
It runs once at each barrier, and only when its list is not empty.
It reads which entities, and not which rows: through the slot that `observe` gave, it finds its list in its private memory.
Each record is a `bridge_trigger_t`: the id, the events, and padding that is zero and that nothing reads.

The list holds ids and not a span, since the observer runs after the barrier moved the rows: an id does not move.
An id that finds nothing was removed or destroyed, and that is how a module reads `BRIDGE_ON_LEAVE` of a destroyed entity.

## Functions

| Function | Result |
| --- | --- |
| `abi_hear` | Adds a change that an observer is owed to the list of who triggered it. Storage calls it. |
| `abi_add_system` | `add_system(at) -> slot`. |
| `abi_observe` | `observe(at) -> slot`. |
