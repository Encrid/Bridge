# Modules on the native backend

What was weighed here and refused is `native_refused.md`, *What was refused: Modules on the native
backend*.

A native module is a **module** like any other - same ABI, same rules, three guarantees fewer,
`module.md`. What it can *do* is there; this file is how a shared library is written against it: the
native backend's SDK.

The native backend exists for one reason: **something that cannot be reached from inside wasm.** A
GPU, an audio device, input, the clock, a filesystem, a socket - and an engine that owns
those, which is what Godot is. Hardware, and the native code that is its only door.

So the test is **could this run on the wasm backend?** A physics library, a codec, a bytecode
interpreter - anything that is pure computation - compiles to wasm, and whether that is worth the
three guarantees is `module.md`, *So what decides*. Bridge-Quake's QuakeC interpreter runs on the
wasm backend.

The test bends in one place: a native library that cannot realistically be built for wasm - threads,
intrinsics, hand-written assembly, sheer size. The rule is then "cannot reach **or** cannot run",
which still excludes "I would rather write C".

Read `module.md`, then `data.md` and `memory.md`.

## The invariant

**A native module's code runs inside a phase** - the scheduler called one of its systems, or one of
its observers at a barrier. Nothing moves inside a phase, and that is what makes reading storage
through a raw pointer safe with no handshake at all. The rest of this file rests on it.

An engine callback arriving on a foreign thread is the one case that escapes it, and it escapes by
**touching no storage at all**. It parks what it has in the module's own memory - a native queue,
the module's to size and to lock, the way Godot's `MessageQueue` is Godot's - and one of the
module's systems drains it into components in the next phase it runs in. So the phase that drain
sits in decides input latency, which is a deployment's choice rather than ours.

## Identity

A section named `.bridge` in the library, read by parsing the file before anything in it runs:

    const char bridge_manifest[] = "godot:adapter@0.1.0\0abi=1\0pinned=true";

placed in that section by whatever the author's compiler spells it with: an attribute, a pragma, a
`link_section`. **One name on every format**, and a short one, because a PE image's section table
holds eight characters and the name has to be the same thing everywhere to be a convention. On
Mach-O it sits in the `__TEXT` segment.

The same contents a wasm module's `bridge:manifest` carries - `module.md`, *The manifest*. What is
specific to this backend is **when** it is read.

**A section rather than an exported symbol, because the decision has to come before the code.**
`trusted` and `on_error` are config keyed on identity, and a symbol is only readable after `dlopen`
has run the library's static initialisers. So you would be checking whether a module is trusted
after executing it, and a library cannot be cleanly unloaded. Read, then decide, then load.

**The section is a convention the host reads, not something Bridge-ASM700 parses.** Bridge-ASM700 is
handed the identity as data - `host_api.md`, *The data* - so the section tables are to be walked
by `bridge_manifest_parse`, a helper beside the library rather than inside it, which `PLAN.md`,
stage 13, writes: ELF's `shdr` and
`shstrtab`, PE's section table after the optional header, Mach-O's load commands. One implementation
for every host that wants it, and none for a host whose module metadata comes from elsewhere.

So a sidecar is a **host's choice** rather than wrong. Godot's `example.gdextension` carries
`entry_symbol` and `compatibility_minimum` beside the binary; a host that prefers that shape reads
it and hands over the same struct. It costs a second file that can go missing, and that is the
host's trade to make.

## Lifecycle

Three exported symbols, the lifecycle every module has, and each is handed the table of
*What the backend provides*:

    int  bridge_init(const bridge_api_t *api);
    int  bridge_shutdown(const bridge_api_t *api);
    int  bridge_reload(const bridge_api_t *api);

**The argument is the native backend's, not Bridge-ASM700's.** A wasm module gets its imports when
it is linked; a shared library has no import section to fill, so this backend hands the same things
over with every call it makes into the module. Bridge-ASM700 never sees it: it asks a backend to
call a function, and every function **Bridge-ASM700** calls takes no arguments and answers a
status - `signatures.md`, *A call takes nothing and answers a status*. What a backend passes to
what it runs is the backend's SDK.

**Exports are named `bridge_<name>`.** Bridge-ASM700 asks the backend for `init`, or for a system called
`movement`, and the native backend looks up `bridge_init` and `bridge_movement`. A C process has one
flat symbol namespace, so `init` is not a name a library can safely export; the prefix is the
backend's stated rule rather than a rename nobody can see.

All three are every module's, on this backend as on the other. A native module that **cannot** be
reloaded says so with `reloadable` in its manifest, and a deployment that says it **may not** with
`hot_reload`: `wasm.md`, *Exports*, has why those are two statements and why neither is made by
leaving a symbol out. Each symbol returns a code, which `on_error` governs - `failure.md`, *When
something fails*.

`bridge_init` runs once per load, before any of the module's systems or observers run. When that
is, the host decides - `lifecycle.md`, *The runtime lifecycle*. Everything the module sets up
happens there: declaring components, registering its systems and its observers, taking hold of
whatever device it exists for.

**A native module need not load before a wasm one.** The original Bridge loaded every plugin first
because its modules imported plugin functions. Here nothing calls, so a module loaded before the
one that feeds it iterates zero spans until that one arrives. The condition that brings an order
back is a module needing another's code, rather than its data, at init.

`bridge_shutdown` closes a leak the original Bridge has: a module holding a device releases it on
teardown rather than when whichever other module would have closed it happens to get there - or
does not, having crashed.

## What the backend provides

`bridge_api_t`, declared in **`bridge_native.h`**, in this backend's `sdk/` - a native module compiles
that and `bridge_abi.h`:

    the eight entries     `abi.md`, *The ABI*, as function pointers, each taking the table first
    shared_base           where the shared memory begins
    private_base          where this module's private memory begins

The same eight functions and the same two memories a wasm module has, because what an ABI entry
does and what is in memory are Bridge-ASM700's and the same on every backend. How a module
reaches them is this backend's: a table, passed.

**`add_system` and `observe` take a name, as they do everywhere.** The backend resolves it to an
exported symbol, so the phase, the events, the terms and the slot are identical on both backends and
the scheduler cannot tell them apart. An entry's arguments are offsets into the scratch arena, on
this backend as on any: Bridge-ASM700 reads them, and the scratch arena is the one memory of a
module it can bounds-check - `abi.md`, *The ABI*. The SDK does the copy: `bridge_native_put`.

**An entry is told whose it is by the table it is given.** A module's table is part of its
instance, so `api->spawn(api)` says which module is asking, and nothing is found by which thread
happens to be running. *The invariant* is then checked and not implied: the backend notes the
thread it calls a module on, and an entry reached on any other thread, or outside a call, answers
zero. `tests/stage5.c`, case `foreign`, reaches one from a thread the module started.

**A refused entry answers zero, and so does every entry after it until the call returns.** Nothing
can unwind native code - `budgets.md`, *What running out does* - so the backend does the nearest
thing: the system runs to its end reaching nothing, and Bridge-ASM700 fails the invocation when it
returns. Without the second half a loop of `spawn` past the entity ceiling would go on minting ids
for as long as the loop ran.

**A library is taken to import both memories.** It has no import section to read, so
`module_list_imports` answers `shared` and `private` for every native module.

And nothing else. No value marshalling, no call context, no buffer interface, no service registry,
and **no way to call another module** - `abi.md`, *The ABI*. The first two have nothing to do
when data does not travel through calls, and the rest answer a question subscription answers.

## Reaching a component

**The same spans, through the same header** - `query.md`, *The interface is a query* - rewritten at
every barrier. A query matches many tables, so there is one span per table and a native module
iterates them the way a wasm module does. What differs is only the address space: an offset is
added to a base the module was handed, where wasm adds it to a memory.

    span    = private + what the private memory's header and the query slot say
    row r   = shared + span.base[t] + r * sizeof(T)

`base` is an offset from the start of the shared memory, identical to what a wasm module sees, so a native
module adds `shared`, which does not move.

**`sizeof(T)` comes from the schema header, not from the span.** Stride is never published, for the
reason `query.md` gives: both sides compile the same declaration, so the compiler already knows it.

One form of span for every backend rather than a resolved copy for native code: two forms would be
two things to write at a barrier, and two things to disagree.

A pointer computed this way is good for the phase it was computed in, for the same reason a row
number is: rows move at a barrier, when an entity is placed in another table or into the place
of one that left.

## What a native module must not do

Not *cannot* - **nothing here is enforced**, and that is the whole difference. The three-memory
split in `wasm.md` buys a native module nothing: it holds real pointers, so every region is
reachable to it, including another module's private memory. A wasm module is protected from other
wasm modules, never from a native one. Component grants are the same story - a native module is
given them to stay honest, not because anything could stop it reading what it was not granted.

**Put a native handle in a component.** That is this backend's only addition to the list in
`module.md`, and it is the subtle one.

A `VkDevice` or a GL context would fit in a `u64` and the component layout would even be sound, but
**storage survives an unload by design**, so the handle outlives the module that made it and the
next reader uses it happily. Two native modules that must share a device share it the way two native
libraries in one process always have - by being built to - and that is not Bridge-ASM700's business.
Bridge-ASM700's storage is for data a component describes, and a pointer is not that.
