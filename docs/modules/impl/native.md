# Modules on the native backend

A native module is a **module** like any other - same ABI, same rules, three guarantees fewer,
`module.md`. What it can *do* is there; this file is how a shared library binds to it.

The native backend exists for one reason: **something that cannot be reached from inside wasm.** A
GPU, an audio device, input, the system clock, a filesystem, a socket - and an engine that owns
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

A named section in the library, read by parsing the file before anything in it runs:

    __attribute__((section("bridge.manifest")))
    const char bridge_manifest[] = "godot:adapter@0.1.0\0abi=1\0pinned=true";

The same contents a wasm module's `bridge:manifest` carries - `module.md`, *The manifest*. What is
specific to this backend is **when** it is read.

**A section rather than an exported symbol, because the decision has to come before the code.**
`trusted` and `on_error` are config keyed on identity, and a symbol is only readable after `dlopen`
has run the library's static initialisers. So you would be checking whether a module is trusted
after executing it, and a library cannot be cleanly unloaded. Read, then decide, then load.

**The section is a convention the host reads, not something Bridge-ASM700 parses.** Bridge-ASM700 is
handed the identity as data - `host-api.md`, *The data* - so the section tables are walked by
`bridge_manifest_parse`, a helper beside the library rather than inside it: ELF's `shdr` and
`shstrtab`, PE's section table after the optional header, Mach-O's load commands. One implementation
for every host that wants it, and none for a host whose module metadata comes from elsewhere.

So a sidecar is a **host's choice** rather than wrong. Godot's `example.gdextension` carries
`entry_symbol` and `compatibility_minimum` beside the binary; a host that prefers that shape reads
it and hands over the same struct. It costs a second file that can go missing, and that is the
host's trade to make.

## Lifecycle

Four exported symbols. Three are the lifecycle every module has, and one is this backend's own:

    void bridge_bind(const bridge_api *api);   /* the backend's, once, before bridge_init */
    int  bridge_init(void);
    int  bridge_shutdown(void);
    int  bridge_reload(void);                  /* optional; absent means cannot be reloaded */

**`bridge_bind` is the native backend's, not Bridge-ASM700's.** A wasm module gets its imports when it is
linked; a shared library has no import section Bridge-ASM700 could fill, so the native backend hands the
same things over in one call before `init`. Bridge-ASM700 never calls it and never sees it: it calls
`define_func` once per entry, as it does on every backend, and what the backend does with them is
the backend's convention. So every function **Bridge-ASM700** calls still takes no arguments and answers a
status - `signatures.md`, *A call takes nothing and answers a status*.

Refused: handing the table to `bridge_init` as an argument, which would make `init` differ by
backend and need a native case in Bridge-ASM700.

**Exports are named `bridge_<name>`.** Bridge-ASM700 asks the backend for `init`, or for a system called
`movement`, and the native backend looks up `bridge_init` and `bridge_movement`. A C process has one
flat symbol namespace, so `init` is not a name a library can safely export; the prefix is the
backend's stated rule rather than a rename nobody can see.

An absent `bridge_reload` is how a native module says it **cannot** be reloaded, the same
convention a wasm module's missing `reload` export uses. `hot_reload = false` is a deployment
saying it **may not**, which is a different statement: `wasm.md`, *Exports*, has why that means the
key can only ever forbid. Each symbol returns a code, which `on_error` governs - `failure.md`, *When
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

`bridge_api`, declared in **`bridge-native.h`** - a native module compiles that and `bridge-abi.h`,
where a wasm module compiles only the second:

    the eight entries     `abi.md`, *The ABI*, as function pointers, unchanged
    shared_base           where the shared memory begins
    private_base          where this module's private memory begins

That is the `bridge` import namespace of `wasm.md`, *Memories*, as a C struct: the same eight
functions and the same two memories, because the memory layout and the function semantics are one
contract on every backend.

**`system` and `observe` take a name, as they do everywhere.** The backend resolves it to an
exported symbol, so the phase, the events, the terms and the slot are identical on both backends and
the scheduler cannot tell them apart. Its arguments are
offsets into the scratch arena, as on the wasm backend - `abi.md`, *The ABI*.

**An entry knows whose it is by who is running.** A C function pointer carries no context, and the
entries are unchanged, so none takes one. *The invariant* supplies it: native code runs only inside
a call the backend made, so the backend notes the instance it is calling, per thread, and an entry
reads that. An entry called from anywhere else - a foreign thread's callback - finds nobody and
answers zero, which is the invariant enforced at the one place it can be.

Refused: a context argument on every entry, which is a second ABI for one backend. The condition
that brings it back is native code that must reach an entry from outside a call.

**A refused entry answers zero, and so does every entry after it until the call returns.** Nothing
can unwind native code - `budgets.md`, *What running out does* - so the backend does the nearest
thing: the system runs to its end reaching nothing, and Bridge-ASM700 fails the invocation when it
returns. Without the second half a loop of `spawn` past the entity ceiling would go on minting ids
for as long as the loop ran.

**A library is taken to import both memories.** It has no import section to read, so
`module_imports` answers `shared` and `private` for every native module.

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

One span layout for every backend rather than a resolved copy for native code: two layouts would be
two rewrites at every barrier, and two things to disagree.

A pointer computed this way is good for the phase it was computed in, for the same reason a row
number is: storage may be compacted or migrated at a barrier.

## What a native module must not do

Not *cannot* - **nothing here is enforced**, and that is the whole difference. The three-memory
split in `wasm.md` buys a native module nothing: it holds real pointers, so every region is
reachable to it, including another module's private memory. A wasm module is protected from other
wasm modules, never from a native one. Component grants are the same story - a native module is
given them to stay honest, not because anything could stop it reading what it was not granted.

**Put a native handle in a component.** That is this backend's only addition to the list in
`module.md`, and it is the subtle one.

A `VkDevice` or a GL context would fit in a `u64` and the layout would even be sound, but **storage
survives an unload by design**, so the handle outlives the module that made it and the next reader
uses it happily. Two native modules that must share a device share it the way two native libraries
in one process always have - by being built to - and that is not Bridge-ASM700's business. Bridge-ASM700's
storage is for data a component describes, and a pointer is not that.
