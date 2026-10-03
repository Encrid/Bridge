# Schema

**Draft.** Open items are tracked in OPEN.md; nothing here is settled until that register is
empty.

## There is no schema format

The schema is a C header using flecs' own declaration macros, compiled by both sides:

    ECS_STRUCT(Position, { float x; float y; })

In a native build that expands to the struct plus a descriptor string, and
`ECS_META_COMPONENT(world, Position)` registers it - flecs parses the descriptor and computes the
offsets.

A wasm module cannot link flecs, so it redefines the macro to emit the typedef and the string and
nothing else:

    #define ECS_STRUCT(name, ...) \
        ECS_STRUCT_TYPE(name, __VA_ARGS__); \
        static const char name##_desc[] = #__VA_ARGS__

`ECS_STRUCT_TYPE` is `typedef struct name {...} name`. Four lines of shim, no flecs.

**BridgeC parses no C.** It hands the descriptor string to flecs' meta, which parses it and answers
with the offsets and the field names - the same work the macro above does in a native build. That is
why `[.config]` can seed a component by matching keys to field names: `config.md`, *An element's own
settings are a component*.

**None of that crosses.** What an element is held to is three numbers - size, alignment, and a hash
of that same string - and the field table stays on BridgeC's side of the boundary with flecs. No
field name is in `bridge-abi.h` and none reaches an element, which is `module-abi.md`, *It holds no
content*.

## Why a header and not a format

The schema's only job was to put the same struct in two places: whoever declared the component,
and whoever else names it. A header does that directly. A format would have described the struct
so a generator could re-emit it, which is a longer path to the same bytes and a second thing to
keep in step.

So **the header is part of what an element publishes**, alongside its wasm, and a mod needs the
base game's. That is how both references ship a mod API: Q3 mods compile against `g_local.h` and
`q_shared.h`, QuakeC mods include the game's `defs.qc`.

## The type set

flecs', minus pointers:

    bool, char, byte          EcsBool, EcsChar, EcsByte
    s8..s64, u8..u64          EcsI8..EcsI64, EcsU8..EcsU64
    f32, f64                  EcsF32, EcsF64
    enum, bitmask             EcsEnum, EcsBitmask
    struct, inline array      EcsStructType, EcsArray
    entity id                 EcsEntity - what *Reference following* is built on

**Rejected, for one reason: pointer-sized or pointer-holding.** `EcsString`, `EcsVector`,
`EcsUPtr`, `EcsIPtr`. A wasm32 pointer is four bytes and the host's is eight, so flecs'
host-computed offsets would not match the module's struct - and a host pointer is not addressable
from a module in any case. That is the same reason `string` and `list` were never components; see
`memory.md`, *Variable length*.

Letting flecs compute offsets is sound **because** of that rejection. Admit a pointer and the two
sides stop agreeing.

**And three types dropped rather than missed.** `option<T>` is redundant: in an ECS, optional means
*not having the component*, which a query already says with `base[t] == 0`. `result<T, E>` and
`variant` are return-value shapes, not stored state. All three were WIT heritage.

## Fingerprints

**One kind, and it is nearly free.** A hash of a component's stringified struct body, which both
sides have because both expanded the same macro over the same tokens. No generator, and nothing to
keep in step.

`#` stringification normalises whitespace to single spaces and strips comments, so the hash survives
reformatting and a new comment, and changes when a field is added, removed, retyped or reordered -
exactly the set of edits that should invalidate it.

A module built against an older version of a component then gets an error at init instead of reading
the wrong bytes for the rest of the process.

**Nothing else is fingerprinted, because nothing else has two parties.** A query has one - the
element declares it and only that element walks it. A system has none: every function BridgeC calls
is `() -> i32`, `vm-surface.md`, *A call takes nothing and answers a status*, so core wasm's own
numeric match is the whole check.

There used to be a second fingerprint, over a function's signature, and dropping calls is what
retired it. It existed because core wasm matches on module name, field name and numeric signature -
`(entity, slot)` and `(slot, entity)` are both `(i64, i64)` and link happily - and with no arguments
anywhere, there is no argument order left to confuse.

A fingerprint only has to detect disagreement, never describe it, which is why a hash is enough
where the component model needed a type system.

## What the generator still emits

Not structs, and not the component fingerprint. Not the published layouts either - those are
`bridge-abi.h`, which both sides compile; see `module-abi.md`, *The published layouts*.

What is left is one thing: accessors over a span, carrying the `row < count` test - **always, with
no switch to remove it**, for the reason in `diagnostics.md`, *Always on*. Call casts were the other
half, and calls are gone.

## Not WIT

WIT was here for the canonical ABI, and the canonical ABI is gone. It cannot express what this
design needs - layout, alignment, access mode, capacity - and it can express what this design must
reject: `string`, `list`, resources, worlds.

What survives is the identity convention, `namespace:name`, which is independent of all this and
already in the manifest section.
