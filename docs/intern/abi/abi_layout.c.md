# src/abi/abi_layout.c

This file reads and lays out the descriptor of a type that a module declares.
It holds the layout against what the compiler of the module made.
It also refuses a type that is one more than a budget allows.

## The descriptor

The schema of a module is a C header.
One text gives the body of a type to two macros of `include/bridge_schema.h`: one makes the type, and one makes its descriptor.
So the type and its descriptor cannot be different.

The compiler of the module lays out the type, and the module states its size and its alignment at `declare`.
Bridge reads only the descriptor, and `src/schema/schema.c` lays the type out from the text as C does: `docs/extern/bridge_schema.h.md`, *The type set*.
A declaration whose stated size or alignment is not what the text lays out to is refused, with both.

The layout also gives the fields by name, and they stay in Bridge: a module is held to the size, the alignment and the descriptor.
Bridge uses the fields to write the settings of a module, `docs/intern/config/config.c.md`, *The settings component*.
It also uses them to carry values over at a reload: `docs/intern/abi/abi_relay.c.md`, *What each change does*.

A field names its type as a bare C name, which is looked for in the namespace of the declarer.
For example, `Vec3` in `game:Body` is `game:Vec3`: `docs/intern/abi/abi_names.c.md`, *A bare name*.
While the text is read, `name` is cut at the colon, so that it holds only the namespace.
The descriptor is copied out of the scratch arena before it is read, and a copy that has no memory refuses the type.
The type keeps the copy, to compare it with a later declaration: `docs/intern/abi/abi_make.c.md`, *A type kept*.
A tag has nothing to read: it is what it was stated to be, which is nothing.

## One layout

Two declarations of one type have one layout if they have the same kind and the same descriptor, byte for byte.
A tag has no descriptor, so two tags of one name have one layout.

Two modules that declare one component from the same schema header hand over the same descriptor.
Each expanded the same macro over the same tokens, so no generator is needed.
`docs/extern/bridge_schema.h.md`, *Layouts*, tells which edits of the text change the descriptor.

A module built against another layout of a component is refused when it declares it, and does not read the wrong bytes.
At a reload, `docs/intern/abi/abi_replace.c.md` compares the descriptors, and lays out the type again where it may.
The descriptor of a component covers its own body, and not the bodies of the types that it names.
Those are declared, each with a descriptor of its own, and checked where they are declared.
So a module built against an older `Vec3` is refused at `Vec3`, before `Body`.

Nothing else has a layout to compare, since nothing else has two parties.
A query has one party: the module declares it, and only that module walks it.
A system has none: every function that Bridge calls is `() -> i32`.

## The budgets

At a reload, a type that changed has its values worked out first: `docs/intern/abi/abi_relay.c.md`, *What each change does*.
`docs/intern/abi/abi_replace.c.md`, *The steps*, tells when a type changed.

Then a type that is one more than `[budget] types` is refused.
A type kept in a sparse set is refused if `[budget] sparse_sets` is used up.
A type kept direct-indexed is refused if `[budget] direct_indexed` is used up.
At a reload, the type that a new type replaces still counts, so the two count twice until the reload ends.

## Functions

| Function | Result |
| --- | --- |
| `abi_is_same_layout` | Whether a declared type has the kind and the descriptor that a declaration states. |
| `parse_descriptor` | Reads the descriptor of the type being declared into its layout. 1 with a refusal. |
| `abi_check_size` | Refuses a type that its descriptor lays out as another size, or another alignment, than the compiler of the module did. |
| `check_room` | At a reload, works out how values are carried over where the type changed. Refuses a type that is one more than a budget allows. |
| `abi_read_layout` | Reads and lays out the descriptor, and checks it against what the module stated and against the budgets. 1 with a refusal, and nothing left allocated. |
