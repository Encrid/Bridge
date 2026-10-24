# src/abi/abi_names.c

This file gives what the declaring files need of names.
It tells whose a name is, where a bare type name is found, and who else declares a type.

## Whose a component is

`docs/extern/bridge_abi.h.md`, *Grants*, tells which components a module may reach.
A name is foreign to a module if it does not start with the namespace of the module and a colon.

The grants let a deployment say what its mod API is, in place of every component that it holds.
The components of a mod need no grant, and to touch the components of the game needs one line.
Bridge checks a grant by name when the module registers: one check for each declaration, and nothing for each frame.

| Where | What is checked |
| --- | --- |
| `declare` | A foreign name needs a read or a write grant: `docs/intern/abi/abi_stated.c.md`. |
| A term that writes | The module may write the component: `docs/intern/abi/abi_term.c.md`. |
| A change that a system states | The module may write the component: `docs/intern/abi/abi_subscribe.c.md`. |
| `attach`, `detach` | The module may write the component: `docs/intern/abi/abi_calls.c.md`. |

Grant two modules write on one component, and their systems conflict.
Then Bridge refuses to load the second module until the deployment puts the two in order: see `src/schedule/schedule_order.c`.

## A bare name

A field names a declared type bare.
The type is found in the namespace of the type being declared: `Vec3` in `game:Body` is `game:Vec3`.
A tag is not found: a tag has no bytes for a field to be.
A type that a reload replaced is not found, so the type that the reload made anew is the one meant.
The descriptor reader, `src/schema/schema.c`, calls `abi_find_type` with the `declaring_t` as `ctx`.
It gets the kind, the size, the alignment and the declared type in `named`.

## Who else declares a type

`abi_say_others` writes the end of a refusal: ". It is also declared by", then the identity of each module.
It names the loaded modules that declare the type. It passes over the module that the declaring module reloads, and every module that is regrouping.

A module is regrouping while a group reload replaces it.
So the modules of one group may change a type that they share: `docs/intern/reload/reload_group.c.md`, *As one*.
A host needs these names to change the layout: it reloads every declarer as one group, or unloads the rest.

## Functions

| Function | Result |
| --- | --- |
| `abi_is_listed` | Whether a list of names holds a name. |
| `abi_is_foreign` | Whether a name is of another namespace than the module's. |
| `abi_find_type` | Finds the type that a field names bare, and writes what the descriptor reader needs of it in `named`. False if there is none. |
| `declares_it` | Whether a module declares a type, if it is not the module that the declaring module reloads, and is not regrouping. |
| `abi_say_others` | Writes who else declares a type, as the end of a refusal. Nothing if nobody does. |
