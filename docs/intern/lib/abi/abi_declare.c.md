# src/lib/abi/abi_declare.c

This file gives `declare(at) -> slot`, the entry by which a module declares a type.

## The steps

1. Copy the `bridge_declaration_t` out of the scratch arena, at the offset that the module gave.
2. Refuse the call if it is not in the first call of the module: `init`, or `reload` at a reload.
3. Find the name and the descriptor in the scratch arena, and copy the name. A tag has no descriptor.
4. Make room for one more slot of the module, and for the set of the types that it may write.
5. Refuse what the declaration may not state: `docs/intern/lib/abi/abi_stated.c.md`.
6. Decide what becomes of an earlier type of that name: `docs/intern/lib/abi/abi_replace.c.md`.
7. If no type of that name is there, make one: `docs/intern/lib/abi/abi_make.c.md`.
8. Give the slot of the module for the type.

The declaration is copied because a module can put it at any offset, aligned or not.
`abi_check_name` refuses a name that is empty or too long: `docs/extern/lib/bridge_abi.h.md`, *A declaration*.
Step 4 has all its memory before anything is made that would have to be unmade.
If a piece is missing, the call is refused for want of memory.

## The slots

`declare` is idempotent: `docs/extern/lib/bridge_abi.h.md`, *declare*.
A module that declares a type again gets the slot that it has.
So there is no entry to find a type: `declare` does it.

A new slot keeps three things: the type, the `on_unload` policy that the module stated, and whether the module may write the type.
A module may write a type of its own namespace, and a type that a write grant names.
If it may, storage records it: that gives the module a stake in an entity that carries the type.
The type counts one more declarer, and the world notes in `layout_moved` that a declaration changed.

## Functions

| Function | Result |
| --- | --- |
| `read_stated` | Copies the declaration and its name out of the scratch arena. 1 with a refusal, also outside the first call. |
| `make_slots` | Makes room for one more slot of the module, and for the set of the types that it may write. 1 with a refusal if there is no memory. |
| `abi_is_writable` | Whether a module may write a type: one of its own namespace, or one that a write grant names. |
| `take_slot` | The slot of the module for the type: the slot that it had, or a new one. |
| `abi_declare` | `declare(at) -> slot`. |
