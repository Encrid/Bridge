# src/abi/abi_stated.c

This file checks what a declaration states before anything is made of it: every value is stated, and none is taken for a default.
It also works out the fingerprint of the declaration.

## What a declaration states

A field of the declaration that is left at zero is unset, and `declare` refuses that for `kind`, `on_unload` and `storage`.

- A name with no namespace is refused.
- A kind that is unset, or that is not one there is, is refused.
- A tag states no bytes and no descriptor, and a tag that states either is refused.
- A name of another namespace is refused if no grant names it: `docs/intern/abi/abi_names.c.md`, *Whose a component is*.
- An `on_unload` policy that is unset is refused: `docs/intern/module/module_types.c.md`, *The policies at unload*.
- A storage layout that is unset is refused.

A tag is the kind `BRIDGE_TAG`, a component with no value: `docs/extern/bridge_abi.h.md`, *bridge_kind*.

## The fingerprint

The fingerprint of a type is the 64-bit FNV-1a hash of its descriptor, combined by exclusive or with its kind.
The hash itself is `hash_fnv1a`: `docs/intern/hash.h.md`.
It names a layout in a record, and decides nothing.
FNV-1a is not a cryptographic hash, so two different descriptors can have one fingerprint, and they are still two layouts.

## Functions

| Function | Result |
| --- | --- |
| `abi_hash_text` | The hash of `n` bytes of text, from `hash_fnv1a`: the fingerprint that names a layout. |
| `check_kind` | Refuses a name with no namespace, an unset kind, and a tag that states bytes or a descriptor. Sets `bare` to the colon of the name. |
| `check_policy` | Refuses a name of another namespace that no grant names, and an unset policy or storage layout. |
| `abi_check_name` | Refuses a name that has 0 bytes, or more than `NAME_MOST`. It checks the names of types, systems and observers. 1 with a refusal. |
| `abi_check_stated` | Refuses a declaration that states what it may not, and works out its fingerprint. 1 with a refusal. |
