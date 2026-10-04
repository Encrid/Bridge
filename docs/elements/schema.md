# Schema

## There is no schema format

The schema is a C header using flecs' declaration macros. Both sides compile it:

    ECS_STRUCT(Position, { float x; float y; })

Native build: that expands to the struct plus a descriptor string.
`ECS_META_COMPONENT(world, Position)` registers it; flecs parses the descriptor and computes the
offsets.

A wasm element cannot link flecs. It redefines the macro to emit only the typedef and the string:

    #define ECS_STRUCT(name, ...) \
        ECS_STRUCT_TYPE(name, __VA_ARGS__); \
        static const char name##_desc[] = #__VA_ARGS__

`ECS_STRUCT_TYPE` is `typedef struct name {...} name`. Four lines of shim, no flecs.

**Bridge-ASM700 parses no C.** It hands the descriptor string to flecs' meta, which answers with the
offsets and the field names. That is how `[.config]` seeds a component by matching keys to field
names: `config.md`, *An element's own settings are a component*.

**None of that crosses.** An element is held to three numbers: size, alignment, and a hash of that
string. The field table stays on Bridge-ASM700's side. No field name is in `bridge-abi.h` and none reaches
an element: `wasm.md`, *It holds no content*.

## Why a header and not a format

The schema has one job: put the same struct in two places, the declarer and whoever else names it.
A header does that directly. A format would describe the struct so a generator could re-emit it: a
longer path to the same bytes, and a second thing to keep in step.

**The header is part of what an element publishes**, alongside its wasm. A mod needs the base
game's. Both references ship a mod API this way: Q3 mods compile against `g_local.h` and
`q_shared.h`; QuakeC mods include the game's `defs.qc`.

## The type set

flecs', minus pointers:

    bool, char, byte          EcsBool, EcsChar, EcsByte
    s8..s64, u8..u64          EcsI8..EcsI64, EcsU8..EcsU64
    f32, f64                  EcsF32, EcsF64
    enum, bitmask             EcsEnum, EcsBitmask
    struct, inline array      EcsStructType, EcsArray
    entity id                 EcsEntity - what `reference.md`, *Reference following* is built on

**Rejected: pointer-sized or pointer-holding.** `EcsString`, `EcsVector`, `EcsUPtr`, `EcsIPtr`. A
wasm32 pointer is four bytes and the host's is eight, so flecs' host-computed offsets would not
match the element's struct. A host pointer is not addressable from wasm anyway. Same reason
`string` and `list` are not components: `strings.md`, *Variable length*.

Letting flecs compute offsets is sound **because** of that rejection.

**Three types dropped.** `option<T>`: optional means *not having the component*, which a query says
with `base[t] == 0`. `result<T, E>` and `variant`: return-value shapes, not stored state.

## Fingerprints

**One kind, nearly free.** A hash of a component's stringified struct body. Both sides have it,
because both expanded the same macro over the same tokens. No generator.

`#` stringification normalises whitespace to single spaces and strips comments. So the hash survives
reformatting and a new comment, and changes when a field is added, removed, retyped or reordered.

An element built against an older layout gets an error at init instead of reading the wrong bytes.

**Nothing else is fingerprinted: nothing else has two parties.** A query has one: the element
declares it and only that element walks it. A system has none: every function Bridge-ASM700 calls is
`() -> i32` - `signatures.md`, *A call takes nothing and answers a status* - so core wasm's numeric
match is the whole check.

No signature fingerprint either. Core wasm links `(entity, slot)` and `(slot, entity)` alike, both
being `(i64, i64)`, but no call takes arguments, so there is no argument order to confuse.

A fingerprint detects disagreement and never describes it. That is why a hash is enough where the
component model needed a type system.

## What the generator still emits

One thing: accessors over a span, carrying the `row < count` test - **always, with no switch to
remove it**: `validation.md`, *Always on*.

Not structs, not the component fingerprint, and not the published layouts. Those are `bridge-abi.h`:
`wasm.md`, *The published layouts*.

## Not WIT

WIT serves the canonical ABI, which this design does not use. It cannot express what the design
needs - layout, alignment, access mode, capacity - and it expresses what the design rejects:
`string`, `list`, resources, worlds.

What survives is the identity convention, `namespace:name`, already in the manifest section.
