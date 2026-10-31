# src/schedule/schedule_uses.c

This file keeps the index of the components: for each component, the systems that touch it or change it.
`docs/intern/schedule/schedule_unordered.c.md` reads the index.

## The index

Each declared type has the list `uses`: one use for each term that touches it, and one for each change of it.
Each list links the uses of one type, newest first.
The uses of a system lie after its bits of `stated_before`, in its block. So the index needs no memory of its own.
`list_system` puts a new system into the index: `docs/intern/abi/abi_subscribe.c.md`.
`schedule_leave_module` takes the systems of a module out before the module drops them: `docs/intern/schedule/schedule_marks.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `link_use` | Puts the next use of a system at the head of the list of a type. |
| `schedule_index_system` | Puts a use of the system for each term that touches a type and for each change into the index. |
| `schedule_unindex_system` | Takes each use of a system out of the list of its type. |
