# src/abi/abi_on_destroy.c

This file checks the `on_destroy` that a declaration states.
The value is set, it fits the storage layout, and each declarer of a type states the same value.

## What a declaration states

`on_destroy` states whether a destroy removes the component, or keeps it on a husk: `docs/extern/bridge_abi.h.md`, *bridge_on_destroy*.
`abi_check_on_destroy` refuses a value that is not an item of `enum bridge_on_destroy`. Zero is no item.
It also refuses a singleton that states `BRIDGE_ON_DESTROY_KEEP`, since a singleton is on no entity, and no destroy reaches it.
`abi_check_stated` calls it after `check_policy`, which refuses a storage layout that is unset.
Thus the check of a singleton reads a storage layout that is an item.

## One on_destroy for each type

The type keeps one `on_destroy`, and storage keeps one set of the types that a husk keeps: `docs/intern/storage/storage_inside.h.md`, *A husk*.
Thus `abi_check_same_on_destroy` refuses a declaration whose `on_destroy` is not the one of the type.
`abi_replace_type` calls it when a type of the name exists: `docs/intern/abi/abi_replace.c.md`, *The steps*.

The refusal holds at a reload too, also when the reloading module alone declares the type: `docs/extern/bridge_abi.h.md`, *When a type changes*.
If a reload changed `BRIDGE_ON_DESTROY_KEEP` to `BRIDGE_ON_DESTROY_REMOVE`, each husk of the type would carry a component that a destroy removes.
To change the value, a module declares a component of a new name.

Storage learns the value from each new slot of a module: `docs/intern/storage/storage_husk.c.md`, *The types that a husk keeps*.

## Constants

| Constant | Value |
| --- | --- |
| `ON_DESTROY_NAMES` | The name of each item of `enum bridge_on_destroy`, by its value, as a refusal writes it. Index 0 is the empty text. Only this file reads it, so it is `static`. |

## Functions

| Function | Result |
| --- | --- |
| `abi_check_on_destroy` | Refuses an `on_destroy` that is no item of the enum, and a singleton that keeps its value at a destroy. 1 with a refusal. |
| `abi_check_same_on_destroy` | Refuses a declaration whose `on_destroy` differs from the one of the type `c` of its name. The refusal names the two items. 1 with a refusal. |
