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
    PLAN.md              the fourteen phases, each with a done-when that can fail

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

    seam/backend/impl/       one document per backend
        wasmtime.md          the wasm backend: the pin, adoption, deadlines
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

    bridge/flecs.md          why flecs, where it allocates, and the fork

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

    modules/impl/            how a module binds, per backend
        wasm.md              on the wasm backend
        native.md            on the native backend

The seams are host to Bridge-ASM700, Bridge-ASM700 to a backend, and Bridge-ASM700 to modules. The
first two have a directory of their own under `seam/`: a separate build target sits on the far side,
and filing the contract under either side would suggest that one owns it. The module seam is
documented in `modules/`, beside what a module is.

A backend is below the orchestrator rather than being a fourth layer: it has no modules and no
deployment, it runs code. The one thing it is configured with is its own - a cache directory, a tick
period: `backend.md`, *The backend owns its clock*.

**A header per seam, and one more for the native binding**, so nothing compiles a declaration it
cannot use:

    bridge.h          host seam: the functions and their structs
    bridge-abi.h      module seam: the layouts both sides read with plain loads -
                      `wasm.md`, *The published layouts*
    bridge-native.h   module seam, native backend only: `bridge_api` - a wasm
                      module gets imports, not pointers
    bridge-wasm.h     module seam, wasm backend only: the imports, and the
                      accessors of `bridge-wasm.wat` - `schema.md`, *What a module
                      compiles against*
    bridge-backend.h  backend seam: the seventeen entries in `backend.md`

**Prose cites a document by name, never by path** - `data.md`, not `bridge/data/data.md` - because a
reader wants the document and not its location, and `tools/xref.py` resolves the name.

## Where to start

`DESIGN.md`, *Why*, is three paragraphs and explains the whole shape. Then `module.md` for what a
module is, and `data.md` for what it works on. Everything else answers a question those raise.
