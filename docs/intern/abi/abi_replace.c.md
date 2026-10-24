# src/abi/abi_replace.c

This file decides what a declaration of a name that is there already becomes.
It is that type, or, at a reload of the one module that declares it, the type that takes its place.

## Shared and exclusive

Storage belongs to the world and lives on after the module that declared a component, so a reloaded module finds its components there.
A changed layout of a component is refused if another module declares the component, and the refusal names the other modules.
That module runs with the old struct, and a column has one component layout only.
To drop the rows takes the data of the other module, and to carry them over gives it new bytes to read with old code.

A host has a way through with the calls it has:

1. Unload each other module that declares the type. The component stays, since one module still declares it.
2. Reload the one module that is left, which is now the only declarer, so its rows are carried over.
3. Load the others again, built against the new struct.

The rows of the component are kept through the three steps.
The refusal lists the modules to unload: `docs/intern/abi/abi_names.c.md`, *Who else declares a type*.

A changed layout is refused too if another loaded module declares a type with a field of it.
The rows of that type hold the type as it is.
If the reloading module alone declares the component, Bridge lays it out again: `docs/intern/abi/abi_relay.c.md`, *What each change does*.

A module reads a component by code that is compiled for one storage layout, so every declarer states the same storage layout.
A declaration that states another storage layout is refused, naming both layouts and the other declarers.
At a reload of the one module that declares it, it is a move: `docs/intern/abi/abi_moved.c.md`, *A move to another storage layout*.

A declaration of a type that is there already, with the same layout, states the same size and alignment as the type.
If not, it is refused as a type whose descriptor lays it out another way: `docs/intern/abi/abi_layout.c.md`.

## The steps

1. Find the type of the name that no reload made anew. If there is none, the declaration makes a new type.
2. It changed if its descriptor or kind is new, or if `abi_names_superseded` gives true: `docs/intern/abi/abi_moved.c.md`, *Functions*.
3. If it did not change and has the same storage layout, it is the type declared.
4. Else refuse it, unless this is a reload of its one declarer that may replace it.
5. If it may, the declaration makes a new type, and keeps the old one in `was`.

The new type takes the rows of the old one if the reload holds.
The runs that an earlier reload that failed worked out for the old type are not this reload's, so they are freed.

## Functions

| Function | Result |
| --- | --- |
| `find_named` | The type of a name that no reload made anew, or nullptr. |
| `is_reloaded` | Whether a module is a reload of one that declares a type. |
| `replace_changed` | Refuses a type with another component layout, unless the reloading module declares it alone and no rows of another module hold it. Then checks a move. |
| `replace_moved` | Refuses a type stated in another storage layout, unless the reloading module declares it alone. Then checks the move. |
| `abi_replace_type` | Decides what becomes of the type of that name that is there already. 1 with a refusal. |
