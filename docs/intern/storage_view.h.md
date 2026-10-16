# src/storage_view.h

This header gives which entity is in a row, and what a module gets for a column.
It also reads and switches the bit of a toggled row.

## A tag

A tag is a component with no bytes: it says that an entity is one of something, and it has no value.
Standard C has no struct without a member, so a module declares a tag as a fourth kind, `BRIDGE_TAG`, with size 0 and no descriptor.
Each of the 5 storage layouts can keep a tag, and storage gives, takes, matches and tells it as any other component.

A tag has no column, no values and no cell:

| Layout | What the tag is |
| --- | --- |
| A table | Nothing in the block. |
| A sparse set | The ids of the entities in the set. |
| Direct-indexed | The bits alone. |
| A singleton | The flag `set`. |

Where a module gets the offset of a value, for a tag it gets `BRIDGE_CARRIED`, which is 1.
Offset 0 of the shared memory is reserved, and its first 8 bytes hold nothing.
So 1 is not 0, which says absent, and it is the place of no value.
A module gets `BRIDGE_CARRIED` as the base of the term of a tag in a span, and as its cell of `column_base`.
The index of a sparse set holds it for an entity in the set.
The cell of a singleton that has a value holds it too.

Two tags get an offset all the same, because a module finds their bits from it.
A toggled tag gets where its column would begin, and a direct-indexed tag gets where its cells would begin.

## Functions

| Function | Result |
| --- | --- |
| `get_who` | Gives which entity is in each row of a table: the twin of its block in `who`. |
| `get_handed` | Gives what a module gets for a column: the column, or `BRIDGE_CARRIED` for a tag that is not toggled. While no tag is declared, it gives the column and looks at no type. |
| `get_cell` | Gives what a module gets for the value of row `row` of a sparse set: where the value is, or `BRIDGE_CARRIED` for a tag. |
| `is_on` | Gives whether row `row` of the toggled column at `column` is on, in a block of `room` rows. |
| `turn_row` | Switches a row of a toggled column on or off. `bits` is where the bits of the column are, which `get_bits` gives. |
