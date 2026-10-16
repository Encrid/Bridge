# src/abi_moved.c

This file tells what a reload may not change of a type.
It may not change a type that the rows of another module hold, or move a type to a storage layout that cannot take it.

## A move to another storage layout

A reload whose `declare` states another storage layout moves the component to it.
Only the one module that declares the component may move it: `docs/intern/abi_replace.c.md`, *Shared and exclusive*.

```text
from         to             what becomes of the values
any          any other but  every entity that carries it keeps its value;
             a singleton    as a toggle, each row is on
any          a singleton    no entity carries it: the singleton has no value;
                            one does: its value is the value of the singleton;
                            more than one: refused, since nothing says which
a singleton  any            refused: there is no entity to carry it
a toggle     any other      a row that is off is carried like the rest, with its value
```

When the component leaves a toggle, which rows were off is lost.
The reload is refused, naming the component and how many rows are off, unless the name of the component is among its losses.
The value is kept, and the `reload` of the module, which runs next, can detach what it does not want.
No observer is told of a move, since no entity gains or loses a component.

Storage writes the rows again: see `src/storage_convert.h`.
All the rows move or none do: `docs/intern/reload_rows.c.md`, *All the rows or none*.

## A type that other rows hold

A type with a field of another type holds values of that type in its rows.
If another loaded module declares such a type, no reload of somebody else may change the type of the field.
`abi_find_holder` finds such a type, and the module that declares it, for the refusal.
It skips the module that reloads and the instance that it replaces.

A type with a field of a type that this reload made anew is laid out again, though its own text is the same.
What a reload makes anew of a type, it makes of every type that has a field of it.

## Functions

| Function | Result |
| --- | --- |
| `abi_names_superseded` | Whether a field of a type names a type that this reload made anew. |
| `has_field_of` | Whether a type has a field of type `c`. |
| `find_declarer` | A loaded module, other than the one that reloads and the one that it replaces, that declares a type. nullptr if none. |
| `abi_find_holder` | A type with a field of type `c` that another loaded module declares, and that module in `by`. nullptr if none. |
| `abi_check_move` | Refuses a move that the table of *A move to another storage layout* refuses, and a loss of a toggle that is not accepted. 1 with a refusal. |
