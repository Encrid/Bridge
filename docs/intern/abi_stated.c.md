# src/abi_stated.c

This file checks what a declaration states before anything is made of it: every value is stated, and none is taken for a default.
It also works out the fingerprint of the declaration.

## What a declaration states

A field of the declaration that is left at zero is unset, and `declare` refuses that for `kind`, `on_unload` and `storage`.

- A name with no namespace is refused.
- A kind that is unset, or that is not one there is, is refused.
- A tag states no bytes and no descriptor, and a tag that states either is refused.
- A name of another namespace is refused if no grant names it: `docs/intern/abi_names.c.md`, *Whose a component is*.
- An `on_unload` policy that is unset is refused: `docs/intern/module_types.c.md`, *The policies at unload*.
- A storage layout that is unset is refused.

A tag is a kind of its own, `BRIDGE_TAG`, because standard C has no struct with no member.
A tag says that an entity is one of something, and it carries no value.

## The fingerprint

The fingerprint of a type is the 64-bit FNV-1a hash of its descriptor, combined by exclusive or with its kind.
A tag has no descriptor, so its fingerprint is the hash of no text with its kind.

Two modules that declare one component from the same schema header hand over the same descriptor.
Each expanded the same macro over the same tokens, so no generator is needed.
The stringification of C makes each run of white space one space and drops the comments.
So the fingerprint stays the same when the text gets another format or a new comment.
It changes when a field is added, removed, given another type, or put in another place.

A module built against another layout of a component is refused when it declares it, and does not read the wrong bytes.
At a reload, `docs/intern/abi_replace.c.md` compares the fingerprints, and lays out the type again where it may.
The fingerprint of a component covers its own body, and not the bodies of the types that it names.
Those are declared, each with a fingerprint of its own, and checked where they are declared.
So a module built against an older `Vec3` is refused at `Vec3`, before `Body`.

Nothing else has a fingerprint, since nothing else has two parties.
A query has one party: the module declares it, and only that module walks it.
A system has none: every function that Bridge calls is `() -> i32`.
A fingerprint finds a difference, and never describes it.

## Functions

| Function | Result |
| --- | --- |
| `abi_hash_text` | The 64-bit FNV-1a hash of `n` bytes of text: the fingerprint of a descriptor. |
| `check_kind` | Refuses a name with no namespace, an unset kind, and a tag that states bytes or a descriptor. Sets `bare` to the colon of the name. |
| `check_policy` | Refuses a name of another namespace that no grant names, and an unset policy or storage layout. |
| `abi_check_name` | Refuses a name of a type, a system or an observer that has 0 bytes, or more than 127. 1 with a refusal. |
| `abi_check_stated` | Refuses a declaration that states what it may not, and works out its fingerprint. 1 with a refusal. |
