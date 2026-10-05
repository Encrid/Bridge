# Schema

## There is no schema format

The schema is a C header, written with three macros of the binding headers:

    BRIDGE_SCHEMA_STRUCT(Position, { float x; float y; })

It expands to the type and a descriptor string, on either backend:

    #define BRIDGE_SCHEMA_STRUCT(name, ...) \
        typedef struct name __VA_ARGS__ name; \
        static const char name##_desc[] = #__VA_ARGS__

The module's compiler lays the struct out; the string is handed over at `declare`.
`BRIDGE_SCHEMA_ENUM` and `BRIDGE_SCHEMA_BITMASK` do the same for the other two kinds.

The shape is flecs' `ECS_STRUCT`, and until phase 10 so was the name. It is ours now because the
name was a promise: a reader expected flecs behind it, and a native module that links flecs for
its own use would have had the macro defined twice. Refused: `BRIDGE_STRUCT`, which is already the
kind a `declare` states.

**Bridge-ASM700 reads the string, and only the string.** `src/schema.c` lays the same struct out
from its text, by the rule C states - each field at the next multiple of its alignment, the whole a
multiple of its widest - and answers with the size, the alignment, and the fields by name. That is
how `[.config]` seeds a component by matching keys to field names: `config.md`, *A module's own
settings are a component*. A `declare` whose stated size or alignment is not what the text lays out
to is refused with both.

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

What a descriptor may say, in full. It is the spelling a module's own header uses, and nothing else
is accepted:

    a struct      { field field ... }
    a field       type name ;   or   type name [ count ] ;
    a type        bool char int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t
                  uint64_t float double bridge_text, or a type its namespace declared
    an enum       { name , name = integer , ... }     counted from zero, or from the last
    a bitmask     { name = integer , ... }            every bit stated

An entity id is a `uint64_t` - `reference.md`, *Reference following*, is built on it. An inline
array has one length. Anything else is refused at `declare`, and the refusal quotes the word the
reading stopped at: `unsigned`, `const`, `*`, a second dimension, a name nothing declared.
`tests/schema.c` reads every accepted spelling against what the compiler lays the same struct out
as, and has every other refused by its word.

Refused: the wider grammar of the parser this replaced, which took `_Bool`, qualifiers, pointers
and C++ spellings. No descriptor in the tree used one, and each is a second way to say something or
a thing no column holds. The condition that brings one back is a module header that cannot be
written without it.

**A field whose type is another declared type names it bare, and it is found in the component's
own namespace.** `Vec3 at;` in `game:Body` is `game:Vec3`, which a module declares first: a struct,
an enum or a bitmask, each by its own macro and each with its kind stated at `declare`. A type that
was not declared there is refused at the declaration that uses it, by its name.

An enum is four bytes signed and a bitmask four unsigned, which is what `BRIDGE_SCHEMA_ENUM` and
`BRIDGE_SCHEMA_BITMASK` state of them in the binding headers and so what a module's compiler lays out.
Refused: reaching across namespaces for a field's type, which
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

**Rejected: pointer-sized or pointer-holding.** A pointer, `size_t`, `intptr_t`. A wasm32 pointer
is four bytes and the host's is eight, so one text would lay out to two structs. A host pointer is
not addressable from wasm anyway. Same reason `string` and `list` are not components: `strings.md`,
*Variable length*.

Laying a struct out from its text is sound **because** of that rejection: every type left has one
size and one alignment on every target a module is built for.

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

Two files, and **nothing is generated**. `bridge-wasm.h` holds the imports, the `BRIDGE_SCHEMA_STRUCT` shim
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
