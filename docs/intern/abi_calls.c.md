# src/abi_calls.c

This file gives the four entries that change an entity in a phase: `spawn`, `destroy`, `attach` and `detach`.
Each puts a change on the queue of the module, and the next barrier applies it: `docs/intern/abi_entry.h.md`, *The queue*.

## spawn

`spawn` gives the next of the ids set aside for the module at the last barrier: see `src/block_module.c`.
When none is left, `spawn` is refused, and the refusal says which of three causes left the module with none:

1. The module stated that it spawns nothing: its `spawns` is `BRIDGE_NONE`.
2. The last barrier could not set aside all its ids: `[budget] entities` is reached.
3. The module used all of its `spawns` ids between two barriers.

The second and the third are the two ways that `spawn` runs out.
In the third, the module is past its own `spawns`.
In the second, the module is past the `entities` of the deployment: the barrier gave it as many ids as fit.

## destroy

`destroy(id)` puts the destroy on the queue, and checks nothing at the call.
An id that is already dead is no mistake: two modules can both want the entity gone.
The barrier looks at the id, and there it finds which module to tell.
Whether the module may destroy the entity is checked at the barrier too: see `src/storage_stake.h`.

## attach and detach

`check_written` lets a module attach or detach a component only if all of these are true:

- The module declared the slot.
- The module may write the component: it is of its own namespace, or a grant lets the module write it.
- What runs in the module stated that it changes the component.

`attach(id, slot, ptr)` carries the value, which is as long as the size that the type was declared with.
`BRIDGE_ZEROED` as `ptr` asks for a zeroed value: the `zero` of the declared type.
A tag has no value, so `BRIDGE_ZEROED` is the one `ptr` that a tag takes, and any other is refused.
A value that does not lie in the scratch arena is refused.

`detach(id, slot)` makes the same checks, and carries no value.
Both refuse a singleton with an id that is not 0, and anything else with the id 0.

## Functions

| Function | Result |
| --- | --- |
| `abi_spawn` | `spawn() -> id`: the next id set aside for the module, or for its `lender`, or a refusal that says why none is left. |
| `abi_destroy` | `destroy(id)`: puts the destroy of the entity on the queue. Refuses the id 0, which is no entity. |
| `check_written` | The component that a slot names, if the module may attach or detach it. Else nullptr, with a refusal. |
| `abi_attach` | `attach(id, slot, ptr)`: puts the attach on the queue, with a copy of the value. |
| `abi_detach` | `detach(id, slot)`: puts the detach on the queue. |
