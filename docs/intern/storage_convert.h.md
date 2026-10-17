# src/storage_convert.h

This header declares what the two files of a move share.
A move writes the rows of a component again as another type, at a reload.

## A move

A reload moves a component when its `declare` states another storage layout, or another component layout.
The reload makes a type for what the component becomes, beside the type it is.
`src/reload_rows.c` then hands `storage_convert` a move for each such type.
For each move, the runs say which bytes of the old value become which bytes of the new value.
The runs are the whole value, or field by field where the component layout changed.

A move is all of the rows or none:

1. Storage folds what each entity that carries a moved type will carry, and moves nothing.
2. It finds a table for each entity, and room in it, before any value moves.
3. If it cannot have the room, it forgets the fold, moves nothing, and gives `STORAGE_NO_ROOM`.
4. Otherwise it places each entity once, and writes each value across.

During a reload, the old and the new type both count and both take room: `docs/intern/reload_rows.c.md`, *All the rows or none*.

What a move does to the values:

| From, to | The values |
| --- | --- |
| Any layout, to a layout that is not a singleton | Each entity that carries the component keeps its value. As a toggle, each row is on. |
| Any layout, to a singleton | With no carrier, the singleton has no value. With one carrier, its value becomes the value of the singleton. |
| A toggle, to another layout | A row that is off moves as the others do, with its value. |

`src/abi_moved.c` refuses the moves that storage does not do, before storage gets them.
It refuses a move from a singleton, and a move to a singleton from more than one carrier.
It also refuses a move out of a toggle with rows that are off, unless the reload accepts that loss.

## Functions

| Function | Result |
| --- | --- |
| `storage_fold_moves` | Folds the moves of a reload: `docs/intern/storage_refold.c.md`, *Functions*. |
