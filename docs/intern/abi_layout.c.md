# src/abi_layout.c

This file reads and lays out the descriptor of a type that a module declares.
It holds the layout against what the compiler of the module made.
It also refuses a type that is one more than a budget allows.

## The descriptor

The schema of a module is a C header.
One text gives the body of a type to two macros of `include/bridge_schema.h`: one makes the type, and one makes its descriptor.
So the type and its descriptor cannot be different.

The compiler of the module lays out the type, and the module states its size and its alignment at `declare`.
Bridge reads only the descriptor: `src/schema.c` lays the type out from the text by the rule of C.
Each field is at the next multiple of its alignment, and the whole is a multiple of its widest field.
A declaration whose stated size or alignment is not what the text lays out to is refused, with both.

The layout also gives the fields by name, and they stay in Bridge: a module is held to the size, the alignment and the fingerprint.
Bridge uses the fields to write the settings of a module, `docs/intern/config.c.md`, *The settings component*.
It also uses them to carry values over at a reload: `docs/intern/abi_relay.c.md`, *What each change does*.

A field names its type as a bare C name, which is looked for in the namespace of the declarer.
For example, `Vec3` in `game:Body` is `game:Vec3`: `docs/intern/abi_names.c.md`, *A bare name*.
While the text is read, `name` is cut at the colon, so that it holds only the namespace.
The descriptor is copied out of the scratch arena before it is read, and a copy that has no memory refuses the type.
A tag has nothing to read: it is what it was stated to be, which is nothing.

## The budgets

At a reload, a type that changed has its values worked out first: `docs/intern/abi_relay.c.md`, *What each change does*.
A type changed if its fingerprint is not the same, or if a field names a type that this reload made anew.

Then a type that is one more than `[budget] types` is refused.
A type kept in a sparse set is refused if `[budget] sparse_sets` is used up.
A type kept direct-indexed is refused if `[budget] direct_indexed` is used up.
At a reload, the type that a new type replaces still counts, so the two count twice until the reload ends.

## Functions

| Function | Result |
| --- | --- |
| `parse_descriptor` | Reads the descriptor of the type being declared into its layout. 1 with a refusal. |
| `check_size` | Refuses a type that its descriptor lays out as another size, or another alignment, than the compiler of the module did. |
| `check_room` | At a reload, works out how values are carried over where the type changed. Refuses a type that is one more than a budget allows. |
| `abi_read_layout` | Reads and lays out the descriptor, and checks it against what the module stated and against the budgets. 1 with a refusal, and nothing left allocated. |
