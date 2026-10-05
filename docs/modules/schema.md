# Schema

## There is no schema format

The schema is a C header using flecs' declaration macros. Both sides compile it:

    ECS_STRUCT(Position, { float x; float y; })

Native build: that expands to the struct plus a descriptor string.
`ECS_META_COMPONENT(world, Position)` registers it; flecs parses the descriptor and computes the
offsets.

A wasm module cannot link flecs. It redefines the macro to emit only the typedef and the string:

    #define ECS_STRUCT(name, ...) \
        ECS_STRUCT_TYPE(name, __VA_ARGS__); \
        static const char name##_desc[] = #__VA_ARGS__

`ECS_STRUCT_TYPE` is `typedef struct name {...} name`. Four lines of shim, no flecs.

**Bridge-ASM700 parses no C.** It hands the descriptor string to flecs' meta, which answers with the
offsets and the field names. That is how `[.config]` seeds a component by matching keys to field
names: `config.md`, *A module's own settings are a component*.

**None of that crosses.** A module is held to three numbers: size, alignment, and a hash of that
string and its kind. The field table stays on Bridge-ASM700's side. No field name is in
`bridge-abi.h` and none reaches a module: `wasm.md`, *It holds no content*.

## Why a header and not a format

The schema has one job: put the same struct in two places, the declarer and whoever else names it.
A header does that directly. A format would describe the struct so a generator could re-emit it: a
longer path to the same bytes, and a second thing to keep in step.

**The header is part of what a module publishes**, alongside its wasm. A mod needs the base
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

**A field whose type is another declared type names it bare, and it is found in the component's
own namespace.** `Vec3 at;` in `game:Body` is `game:Vec3`, which a module declares first: a struct,
an enum or a bitmask, each by its own macro and each with its kind stated at `declare`. flecs
resolves a field's type by name from where it stands, so Bridge-ASM700 names each declared type
under its namespace and parses a descriptor from inside it. A type that was not declared there is
refused at the declaration that uses it, with the name flecs could not find.

An enum and a bitmask are 32 bits, which is what flecs lays them out as and what a C enum is on
every target a module is built for. Refused: reaching across namespaces for a field's type, which
would need the descriptor to carry a namespace C has no syntax for. The condition that brings it
back is a mod that must embed one of the game's structs in its own component, and until then it
declares a struct of its own with the same body.

**The fingerprint of a component covers its own body, not the bodies of the types it names.** Those
are declared, so each has a fingerprint of its own, checked where it is declared: a module built
against an older `Vec3` is refused at `Vec3`, before `Body`.

**One type is given and not declared: `bridge_text`**, the `(offset, length)` pair of
`strings.md`, *Variable length*. It is a layout of `bridge-abi.h` and a field may be of it under any
namespace. It exists so that a deployment's text can be told from two integers - `config.md`, *A
module's own settings are a component* - and it is no component: nothing carries it but a field.

**Rejected: pointer-sized or pointer-holding.** `EcsString`, `EcsVector`, `EcsUPtr`, `EcsIPtr`. A
wasm32 pointer is four bytes and the host's is eight, so flecs' host-computed offsets would not
match the module's struct. A host pointer is not addressable from wasm anyway. Same reason
`string` and `list` are not components: `strings.md`, *Variable length*.

Letting flecs compute offsets is sound **because** of that rejection.

**Three types dropped.** `option<T>`: optional means *not having the component*, which a query says
with `base[t] == 0`. `result<T, E>` and `variant`: return-value shapes, not stored state.

## Fingerprints

**One kind, nearly free.** A hash of a component's stringified struct body. Both sides have it,
because both expanded the same macro over the same tokens. No generator.

`#` stringification normalises whitespace to single spaces and strips comments. So the hash survives
reformatting and a new comment, and changes when a field is added, removed, retyped or reordered.

A module built against an older layout gets an error at init instead of reading the wrong bytes.

**Nothing else is fingerprinted: nothing else has two parties.** A query has one: the module
declares it and only that module walks it. A system has none: every function Bridge-ASM700 calls is
`() -> i32` - `signatures.md`, *A call takes nothing and answers a status* - so core wasm's numeric
match is the whole check.

No signature fingerprint either. One would guard a call between modules, where two arguments of one
width link in either order, and there is no such call.

A fingerprint detects disagreement and never describes it. That is why a hash is enough where the
component model needed a type system.

## What a module compiles against

Two files, and **nothing is generated**. `bridge-wasm.h` holds the imports, the `ECS_STRUCT` shim
above, and the span, the header and the log ring read through accessors. `bridge-wasm.wat` holds
the accessors themselves: a load or a store per width, and the row address carrying the
`row < count` test - **always, with no switch to remove it**: `validation.md`, *Always on*. They are
wasm text merged into the compiled module, because no C compiler can address the shared memory
itself: `wasm.md`, *A compiled module reaches them through accessors*.

A generator was planned and turned out to have nothing to compute. An accessor needs a field's
type, its offset and its struct's size, and the module's own compiler has all three: `_Generic`
picks the load by the field's type, `offsetof` and `sizeof` supply the rest. So a typed accessor is
a macro over the struct the schema header already declares, and a second description of that struct
for a tool to read would be the format *Why a header and not a format* refuses.

The published layouts are not here either. Those are `bridge-abi.h`: `wasm.md`, *The published
layouts*.

## Not WIT

WIT serves the canonical ABI, which this design does not use. It cannot express what the design
needs - layout, alignment, access mode, capacity - and it expresses what the design rejects:
`string`, `list`, resources, worlds.

What survives is the identity convention, `namespace:name`, already in the manifest section.
