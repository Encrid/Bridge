# Prior art

What Bridge reads for its reasoning and not for its answer: `contributing.md`, step 3 of how a
decision is made. Each is what it is, where it is read, and what was taken from it or refused,
with the document that says so and gives the reason. Nothing here is the specification: a
choice of theirs is evidence of what they were weighing, and it is used where the same force
acts on us.

What is linked or carried is not here: that is `dependencies.md`.

## idTech3

The engine of Quake III Arena, its source as released. Read by file and line, at no tag:
`qcommon/vm.c`, `qcommon/common.c`, `qcommon/zone.c`, `win32/win_main.c`, `game/g_public.h`.
Fifteen documents name it, more than any other.

    taken      the arrangement of a host around an engine that is a library: `host_api.md`,
               `create.md`
               budgets stated at startup, and running out refused by name: `design.md`
               allocation for a lifetime, with no bookkeeping for each object: `strings.md`
               a reload that keeps the data, as `VM_Restart` does: `lifetime.md`, `costs.md`
               private memory held by a bounds check, as `& dataMask` did: `wasm.md`
               one ABI header that both sides compile, as `g_public.h` was, and the header
               as part of what a module publishes: `wasm.md`, `schema.md`
               two guards of `Com_Error`, repetition that escalates and a failure while
               handling one: `failure.md`, `loop.md`
               the line between a trusted native VM and a bytecode one: `capabilities.md`
    refused    content in the ABI header - `entityState_t` and a prefix a game must begin its
               struct with - since it is a game engine and knows what an entity is: `wasm.md`
               one entry that dispatches on a number, `vmMain`: `wasm.md`
               an ABI held by a comment in capitals: `wasm.md`
               a silent fall back to the safe path when the fast one is missing:
               `capabilities.md`
               budgets with a default and a clamped floor: `budgets.md`
               a hunk and a zone, and a mark and reset on an allocator: `strings.md`
               validation that is always on and always paid for: `validation.md`
               three functions as the whole of a host's seam, with no sink: `create.md`
               a journal that is a sequential file: `log.md`

## Quake

The engine before it, and QuakeC's bytecode: `pr_exec.c`, `pr_edict.c`, `defs.qc`.

    taken      a work limit set again before every dispatch, its `runaway` count: `limits.md`
               fixed offsets at the bottom of a module's memory, as its progs have:
               `memory.md`
               a mod compiling against the game's own definitions: `schema.md`
    refused    a reload that is a level restart, which avoids a changed layout and does not
               answer it: `lifetime.md`

## flecs

An entity component system in C. Bridge's storage was flecs', patched, and is its own; what it
was measured beside is flecs and EnTT at pinned tags, by `tools/reference.py`. Read at
`flecs.h` and its pipeline addon and JSON serialiser, by line.

    taken      the shape of the macro that describes a struct: `schema.md`
               what deleting a component does to what holds it, remove or delete:
               `lifetime.md`
               an absent term told by a zero base: `query.md`
               one metric for each system: `profile.md`
               offsets that stay good when an arena grows: `budgets.md`
    refused    the macro's name, which a native module linking flecs would define twice:
               `schema.md`
               an emptied entity left alive, since no module here can enumerate entities:
               `lifetime.md`
               a source for each field of an iterator: `query.md`
               eight phases shipped, and relations between phases: `schedule.md`
               a system that may ask for the actual world: `schedule.md`
               checks that are compiled out: `validation.md`
               a cursor that is marked and restored: `strings.md`

## EnTT, Bevy

Two more entity component systems, named with flecs. EnTT is the second thing the storage
layouts are measured beside. Refused of all three: an emptied entity left alive -
`lifetime.md`.

## Godot

The engine that owns a thread, a GPU and an input queue, and so the real case for a native
module. Read through its GDExtension interface and its documentation.

    taken      a module that must run on the thread that made it, which is why a module
               may be pinned: `pinned.md`
               a callback from a foreign thread parked in a queue the module owns, as its
               `MessageQueue` is its own: `native.md`
               a file beside a library that names its entry, as a host's choice: `native.md`
               that its own documented example needs no call at all: `engine_callbacks.md`
    refused    a call from the wrong thread deferred or proxied: `pinned.md`
               a string copied at the boundary and answered as a handle: `log.md`
               a callback whose answer the engine consumes inside its own loop, which
               storage cannot serve and is a stated loss: `engine_callbacks.md`, `costs.md`

## `legacy/rust-components`

The design before this one: a host for modules of the WebAssembly Component Model, in Rust.
What Bridge replaces, and so what its costs are counted against.

    taken      its measurements, as the reason for the shape: what an empty tick, a call out
               of a module and a call between modules cost: `design.md`, `no_builtins.md`,
               `log.md`
               forgiving failures on sustained health and not on one good pass: `failure.md`
               that a count nobody asked for is still paid for: `limits.md`
               what is kept of it, listed: `costs.md`
    refused    data sent through calls at all: `design.md`
               interfaces the host implements itself: `no_builtins.md`
               a memory for each module, and with it isolation between modules, which is a
               stated regression: `costs.md`
               a reloaded module losing its state: `costs.md`
               a module that cannot set its own property, since the call came back as a
               re-entry: `engine_callbacks.md`

## The Component Model, and WIT

WebAssembly's model for describing and copying values across a language boundary, and its
interface language.

    taken      the memory fixed where a function is declared: `log.md`
               the convention for an identity, a namespace and a name: `schema.md`
    refused    components, for core wasm: nothing is copied here, and a component cannot
               import or export a memory: `design.md`, `wasm.md`
               its type system, where a fingerprint is enough: `schema.md`
               WIT as the schema, which cannot say a layout, an alignment or an access mode:
               `schema.md`, `design.md`

## The Linux kernel

    taken      a manifest readable before any code runs, the shape of `.modinfo`: `module.md`
               that a module marks its own parameters: `config.md`
               the length of a function, from its coding style: `style.md`

## The Epitech C Coding Style

Version 3.1, itself after the Linux kernel's style and Robert C. Martin's Clean Code. Taken
whole, as `style.md`. What differs is said there: the file header, the comment above a
function, and two sections at the end.

## Others, each read for one thing

    syslog, RFC 5424     the numbers of a severity: `log.md`. Refused: zero as a level, which
                         is unset here: `config.md`
    TOML                 how a key is spelled in these documents: `config.md`. Refused:
                         parsing it, or anything else a host hands over: `host_api.md`
    Erlang               new code given an export of its own, which is why reload is not
                         init: `lifecycle.md`
    Minecraft            the largest ecosystem of mods there is, with no isolation between
                         them: `costs.md`. Refused: what it suffers for that, crashes nobody
                         can attribute, which attribution and grants are for
    Source               one of the allocators read for mark and reset, which is refused:
                         `strings.md`
    musl                 `strtoll` and `strtoull`, carried in `src/text.c` under its licence:
                         `style.md`
