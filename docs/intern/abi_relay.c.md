# src/abi_relay.c

This file works out how a reload carries over the values of a type that has another component layout.
If the values cannot be carried over, it says why.

## What each change does

The fingerprint finds a changed layout: `docs/intern/abi_stated.c.md`, *The fingerprint*.
What follows depends on who else holds the component: `docs/intern/abi_replace.c.md`.
A component that only the reloading module declares has its rows carried over to the new layout, field by field, by name.
Bridge keeps the fields of every type, and the new `declare` gives the new descriptor, so Bridge has both.

```text
a field added                its bytes are zero, and the reload of the module gives it a value
fields in another order      each field goes to its new offset
an array made longer         what it held is kept, and the rest is zero
a field removed              a loss: it goes from every row
an array made shorter        a loss: the first of it is kept, and the rest goes
a field renamed              a field removed and a field added: the old values go
a field of another type      refused, naming the field
a struct a field names,      the same rules, one level down
changed
an enum or a bitmask         refused if a constant is gone or has another number,
changed                      or if its integer has another width; a constant added is nothing
a type with a field of a     laid out again all the same, since its field is
type that changed
a type that a component of   refused, naming that component and that module
another module holds
another kind                 refused: a reload does not change the kind of a type
```

A loss is refused until the reload states it: `docs/intern/abi_relay_run.c.md`, *A loss stated by name*.
A tag has no value to carry.

The values are carried over as a move to another storage layout is, by the same code.
A type is made for what the component becomes, and it takes every row or none when the reload holds.
So a component can change its layout and its storage layout in one reload.

A module cannot give a function that converts a value: the function would have to read a layout that its new build does not hold.
For a change that no rule covers, a module declares a new name.
A system reads the old component and writes the new one, and then the module stops declaring the old one.

## The steps

1. Refuse a type that has another kind than it had.
2. Do nothing for a tag.
3. For an enum or a bitmask, refuse a new width, and each constant that is gone or has another value.
4. For a struct, count the rows that carry the type, and match its fields: `docs/intern/abi_relay_fields.c.md`.
5. Refuse the struct if a run had no memory, or if a loss is not accepted.
6. Keep the runs on the old type, in `runs` and `run_count`, for the reload to use if it holds.

The refusal of a loss names every field that goes, and how many rows hold the type.
A struct that keeps no bytes still gets a list of runs, so that `runs` is not nullptr once the relay is worked out.

## Constants

| Constant | Value |
| --- | --- |
| `KIND_NAMES` | The name of each kind as a refusal says it, by `enum bridge_kind`: a struct, an enum, a bitmask, a tag. |

## Functions

| Function | Result |
| --- | --- |
| `check_constant` | Refuses a constant of an enum or a bitmask that is gone, or has another value. A value that a row holds must keep its meaning. |
| `check_constants` | Refuses an enum or a bitmask whose integer, or one of whose constants, changed. |
| `keep_runs` | Keeps on a type how its values are carried over, or refuses for want of memory or for a loss that is not accepted. |
| `relay_struct` | Works out how the values of a struct are carried over to what it becomes. 1 with a refusal. |
| `abi_relay` | Works out how the values of a type are carried over to what it becomes, and keeps that on the type. 1 with a refusal. |
