# Bridge-ASM700

An orchestrator, written in C, that runs modules over shared ECS storage. The name is temporary:
this replaces the original Bridge when it is ready.

**It is its own design, not a rewrite.** The original Bridge is a reference here - cited where it
gives a reason, as idTech3, flecs and Godot are - and never the specification.

**Bridge-ASM700 is a library.** A host links it and owns `main`, the loop and every file. Hosts are
separate projects, and none is part of this one.

Three documents go with this one. `DESIGN.md` is why the shape is what it is and what it costs.
`PLAN.md` is the build order. `PHASE0.md` is how a decision gets made here at all, which is method
rather than design.

## Three layers, three seams

The three layers are a host, Bridge-ASM700 and the modules. A host is somebody else's project, so
it has no directory here: only its seam does. **The tree is the structure** otherwise, rather than a
diagram to keep in step with. One concern per document, in reading order:

    README.md            this file
    DESIGN.md            why, and what the design is
    PLAN.md              the nineteen phases, each with a done-when that can fail

    design/
        no-builtins.md       the rule, and its universality test
        split.md             which side owns what
        costs.md             what we give up
        build.md             the compiler, the platforms, the standard, the C library

    plan/
        PHASE0.md            how we decide things. method, not design
        PHASEN.md            one per phase, written before that phase starts

    seam/host-api/           host to Bridge-ASM700
        host-api.md          the surface and its structs
        create.md            bridge_create, and the sink
        loop.md              who advances, and who decides when it ends
        lifecycle.md         load, unload, reload
        introspection.md     the one pull about modules
        config.md            what Bridge-ASM700 requires of the values it is handed

    seam/backend/            Bridge-ASM700 to a backend
        backend.md           the seventeen entries, memory, the clock
        capabilities.md      what a backend says it can do
        signatures.md        imports declared, exports checked

    bridge/run/
        schedule.md          phases, barriers, observers, the ordering refusal
        threads.md           what runs at once
        limits.md            fuel, timeouts, the memory ceiling
        failure.md           what happens when something fails

    bridge/data/
        data.md              components, entities, access mode, structural change,
                               observers
        query.md             spans: how a system reads its data
        reference.md         following one entity by id
        lifetime.md          reload, unload, and a changed layout

    bridge/memory/
        memory.md            the seven regions
        strings.md           the two arenas, and which one a string belongs in
        budgets.md           what is sized, and by whom

    bridge/storage.md        who keeps the rows: tables, the barrier, what is written

    bridge/diagnostics/
        validation.md        what a bad write costs, and what finds it
        log.md               the ring a module writes
        profile.md           per-system counters

    modules/
        module.md            what a module is: identity, the rules, the guarantees
        abi.md               the eight entries
        grants.md            whose component it is
        pinned.md            one thread for one module
        engine-callbacks.md  answering an engine from storage
        schema.md            how a module declares its data

    backends/wasm/           the wasm backend, and how a module is written for it
        wasm.md              a wasm module: memories, accessors, exports
        wasmtime.md          the engine behind it: the pin, adoption, deadlines

    backends/native/         the native backend, and how a module is written for it
        native.md            a native module: the table of entries, the invariant

The seams are host to Bridge-ASM700, Bridge-ASM700 to a backend, and Bridge-ASM700 to modules. The
first two have a directory of their own under `seam/`: a separate build target sits on the far side,
and filing the contract under either side would suggest that one owns it. The module seam is in
two places because it has two owners. `modules/` is what is true of a module whatever runs it.
`backends/` is how one is written for the backend that runs it.

A backend is below the orchestrator rather than being a fourth layer: it has no modules and no
deployment, it runs code. The one thing it is configured with is its own - a cache directory, a tick
period: `backend.md`, *The backend owns its clock*.

**The repository is laid out the same way: a path says whose a thing is.**

    include/                 the three seams Bridge-ASM700 defines, and nothing else
        bridge.h                 host seam: the functions and their structs
        bridge-backend.h         backend seam: the seventeen entries in `backend.md`
        bridge-abi.h             module seam: what Bridge-ASM700 writes for a module to read,
                                 the same on every backend - `wasm.md`, *The published layouts*
    src/                     the library. Nothing outside it is compiled against it
    backends/wasm/           the wasm backend: its source, and what it needs of the system
        sdk/                     how a wasm module is written: `bridge-wasm.h`, and the
                                 accessors of `bridge-wasm.wat` that are merged into one
        wasmtime/                the engine it is linked against
    backends/native/         the native backend: its source, and what it needs of the system
        sdk/                     how a native module is written: `bridge-native.h`
    tests/                   the hosts that drive the cases
        modules/wasm/            the modules they load, by the backend that runs each
        modules/native/
    tools/                   the scans
    build.py                 the build - `build.md`, *One script*

A backend is compiled against `include/bridge-backend.h` and its own folder. It shares no code
with the library, not even the few calls each makes to the operating system: a backend that
somebody else writes has only the seam, and ours are held to the same.

**How a module is written is its backend's, and not Bridge-ASM700's.** A backend is handed bytes,
memories and host functions and asked to run a function - `backend.md`, *What the backend does not
do* - so Bridge-ASM700 never learns what a module is made of, and cannot say how one is written.
What it can say is what it puts in memory, and that is `bridge-abi.h`. Everything about reaching
it - imports or a table of pointers, accessors, the macros an author types, and the same for a
language that is not C - belongs to the backend that runs the module, and lives with it.

**Prose cites a document by name, never by path** - `data.md`, not `bridge/data/data.md` - because a
reader wants the document and not its location, and `tools/xref.py` resolves the name.

## Where to start

`DESIGN.md`, *Why*, is three paragraphs and explains the whole shape. Then `module.md` for what a
module is, and `data.md` for what it works on. Everything else answers a question those raise.
