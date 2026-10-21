# src/lib/reload/reload_rows.c

This file moves the rows of each type that a reload made in the place of another.

## All the rows or none

A reload makes a new type for a component whose component layout or storage layout changes.
`docs/intern/lib/abi/abi_moved.c.md`, *A move to another storage layout*, and `docs/intern/lib/abi/abi_relay.c.md`, *What each change does*, tell when.
The new type takes all the rows of the type that it replaces, or none of them.
The move comes after the `reload` of the new instance has returned, and after the schedule is built again.
`src/lib/storage/storage_convert.c` first gets each table and each row that the new types need, while each value is still where it was.
Then it places each entity once, writes its value across, and the old type is gone.

If the room cannot be had, nothing has moved, and the reload fails and says why.
The new type is made beside the old one, so for the length of a reload the component is two types.
It counts twice against `[budget] types`.
It counts twice against `[budget] sparse_sets` or `[budget] direct_indexed` too, if both types are kept that way.
Its rows need room where they go, before they leave where they are: `docs/extern/bridge.h.md`, *A reload*.
The observers hear nothing, because no entity gains or loses a component.

## The move of a type

A move copies each value whole: as many bytes as the shorter of the two types has.
Where the component layout changed, it copies the value field by field, in the runs that `declare` worked out: `src/lib/abi/abi_relay.c`.
`move_all` takes room for a move of each type that replaces another, however many there are.

## A move refused

- If there is no memory to carry the values across, the reload fails with "out of memory", and the count of components.
- If `[budget] tables` or `[budget] columns` has no room for the rows, the reload fails with a record that names both keys: `docs/extern/bridge.h.md`, *A reload*.

## After the move

No type replaces another any more.
A warning names each loss that the reload accepted, with the number of rows that lost something under that name.

## Functions

| Function | Result |
| --- | --- |
| `reload_set_move` | Writes the move of the rows of a type to the type made in its place: the whole value, or the runs of `declare`. |
| `reload_settle` | Makes the type that replaces the settings component the settings component of the record. Marks that no type replaces another any more. Says each loss that the reload accepted, with its number of rows. |
| `move_all` | Takes room for the moves, writes the move of each type that replaces another, and hands them to `storage_convert`. Gives `STORAGE_NO_MEMORY` if there is no room. |
| `reload_refuse_move` | Writes why the rows of `count` components did not move: no memory to carry the values, or no room where they go. Gives 1. |
| `reload_move_rows` | Moves the rows of each type that the reload made in the place of another, all of them or none. 1 and why if not. |
