# Bridge-ASM700

An orchestrator, written in C, that runs modules over shared ECS storage. The name is temporary:
this replaces the original Bridge when it is ready.

**It is its own design, not a rewrite.** The original Bridge is prior art here - cited where it
gives a reason, as idTech3, flecs and Godot are - and never the specification.

**Bridge-ASM700 is a library.** A host links it and owns `main`, the loop and every file. Hosts are
separate projects, and none is part of this one.

Three documents go with this one. `design.md` is why the shape is what it is and what it costs.
`plan.md` is the build order. `stage0.md` is how a decision gets made here at all, which is method
rather than design.

## Three layers, three seams

The three layers are a host, Bridge-ASM700 and the modules. A host is somebody else's project, so
it has no directory here: only its seam does. **The tree is the structure** otherwise, rather than a
diagram to keep in step with. One concern per document, in reading order:

    readme.md            this file
    design.md            why, and what the design is
    plan.md              the eighteen stages, each with a done-when that can fail
    style.md             how the C is written: the rules, and the exceptions to them

    refused/             what was weighed and not chosen, in the same tree as what was:
                         `refused/bridge/data/lifetime_refused.md` is the record of
                         `bridge/data/lifetime.md`

    design/
        no_builtins.md       the rule, and its universality test
        split.md             which side owns what
        costs.md             what we give up
        build.md             the compiler, the platforms, the standard, the C library
        dependencies.md      every tool and every import, with why it is there

    plan/
        stage0.md            how we decide things. method, not design
        stagen.md            one per stage, written before that stage starts

    seam/host_api/           host to Bridge-ASM700
        host_api.md          the surface and its structs
        create.md            bridge_create, and the sink
        loop.md              who advances, and who decides when it ends
        lifecycle.md         load, unload, reload
        introspection.md     the one pull about modules
        config.md            what Bridge-ASM700 requires of the values it is handed

    seam/backend/            Bridge-ASM700 to a backend
        backend.md           the eighteen entries, memory, the clock
        capabilities.md      what a backend says it can do
        signatures.md        imports declared, exports checked

    bridge/run/
        schedule.md          phases, barriers, observers, the ordering refusal
        threads.md           what runs at once
        limits.md            the work limit, timeouts, the memory ceiling
        failure.md           what happens when something fails

    bridge/data/
        data.md              components, entities, access mode, structural change,
                               observers
        query.md             spans: how a system reads its data
        reference.md         following one entity by id
        lifetime.md          reload, unload, and a changed component layout

    bridge/memory/
        memory.md            the regions
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
        engine_callbacks.md  answering an engine from storage
        schema.md            how a module declares its data

    backends/wasm/           the wasm backend, and how a module is written for it
        wasm.md              a wasm module: memories, accessors, exports
        wasmtime.md          the engine behind it: the pin, adoption, deadlines

    backends/native/         the native backend, and how a module is written for it
        native.md            a native module: the table of entries, the invariant

**A document says what the design is, and its record says what it is not.** What was weighed
and refused, with why and with the condition that would bring it back, is in `refused/`, in a
file of the same name with `_refused` after it, under the same headings. A document names its
record once, at its head; a record names the section each of its entries belongs to, and the
citation check reads both, so a section renamed leaves no entry behind without saying so. It is
two files and not one because they are two concerns: a reader who wants to know what happens
reads the first, and one who wants to know why not otherwise reads the second.

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
        bridge_backend.h         backend seam: the eighteen entries in `backend.md`
        bridge_schema.h          module seam: how a type is stated in C, for a module on any
                                 backend
        bridge_abi.h             module seam: what Bridge-ASM700 writes for a module to read,
                                 the same on every backend - `wasm.md`, *The ABI layouts*
    src/                     the library. Nothing outside it is compiled against it
    backends/wasm/           the wasm backend: its source
        sdk/                     how a wasm module is written: `bridge_wasm.h`, which
                                 includes the others, and the accessors of `bridge_wasm.wat`
                                 that are merged into one
        wasmtime/                the engine it is linked against
    backends/native/         the native backend: its source
        sdk/                     how a native module is written: `bridge_native.h`, which
                                 includes the others
    utils/os/                the operating system's layer: one source, compiled into the
                             library and into each backend - `build.md`, *The C library*
        windows/, posix/         the same functions, for each kind of platform
    tests/                   the hosts that drive the cases
        modules/wasm/            the modules they load, by the backend that runs each
        modules/native/
    tools/                   the scans, and `reference.py`, which measures the storage
                             layouts beside flecs and EnTT
    build.py                 the build - `build.md`, *One script*

A backend is compiled against `include/bridge_backend.h` and its own folder. It shares no code
with the library, not even the few calls each makes to the operating system: a backend that
somebody else writes has only the seam, and ours are held to the same.

**How a module is written is its backend's, and not Bridge-ASM700's.** A backend is handed bytes,
memories and functions to offer a module, and asked to run a function - `backend.md`, *What the
backend does not do* - so Bridge-ASM700 never learns what a module is made of, and cannot say how
one is written. What it can say is what it puts in memory, and that is `bridge_abi.h`. Everything
about reaching it - imports or a table of pointers, accessors, the macros an author types, and the
same for a language that is not C - belongs to the backend that runs the module, and lives with it.

**Prose cites a document by name, never by path** - `data.md`, not `bridge/data/data.md` - because a
reader wants the document and not its location, and `tools/xref.py` resolves the name.

## Words

One word, one thing. Each is defined here once and treated where the document named says. A
word that has more than one sense in English is used in these documents in the one given, and
where two things could be meant the qualified form is the only form.

**Who is who**

    host              the program that links Bridge-ASM700 and drives it. `host_api.md`
    backend           a shared library Bridge-ASM700 opens, which runs modules. `backend.md`
    module            what a backend runs: a wasm binary or a native library. `module.md`
    deployment        one host's choice of backends, modules and config
    seam              where two of them meet: host, backend, module. Each has one header
    SDK               the files a module is written with. It is its backend's, in `sdk/`
    the operating     always in full. "system" alone is a module's
    system

**Running**

    execution model   how work is arranged in time. Here: task parallelism between modules,
                      and sequential within one. `threads.md`. "Parallel" is two things
                      running at the same instant; "concurrent" is two in progress at once,
                      which one core can do by switching. Not the same word
    system            a function of a module, run in a phase over what its query matches
    observer          a function of a module, run at a barrier for what changed. `data.md`
    phase             a named step a host asks for: its systems run. `schedule.md`
    barrier           what follows every phase: changes applied, then observers
    pass              one walk of a deployment's phases, in the host's order
    stage             a step of `plan.md`. Never "phase"
    ABI entry         one of the eight things a module may call. `abi.md`
    backend entry     one of the things a backend offers Bridge-ASM700. `backend.md`
    export            a function a module offers: `init`, `reload`, `shutdown`, a system
    call              what a host makes of `bridge.h`. Not an "entry"

**Data**

    type              anything `declare` registers: a struct, an enum, a bitmask. `schema.md`
    component         a type that entities carry
    type kind         struct, enum or bitmask. Also "setting kind", "field kind": never bare
    component layout  the fields of a component: names, types, offsets, size
    descriptor        the text of a type's body, as the module's compiler stringified it
    schema            the header a module publishes, which declares its types
    entity            an id: an index and a generation. `data.md`
    reference         an entity id kept in a component and followed. `reference.md`
    term              one component a query asks for, and how
    work limit        the most work one invocation of a system may do, counted by the backend
                      in its own units: the key `work_limit` and the capability of that name.
                      Not "fuel", which is what wasmtime calls its count, nor "meter"
    trusted           what a deployment states of a module: that its code needs no containing.
                      The backend's side of it is the capability "untrusted". Not "sandbox",
                      which is how one backend keeps it
    change            an attach, a detach or a destroy asked of an entity, applied at the next
                      barrier. A system's "changes" are the components it stated that it
                      attaches or detaches
    slot              the number `declare`, `add_system` or `observe` answers a module. A position
                      in an array is a "place"
    event             what an observer is told of an entity against its terms: enter, leave,
                      attach. `data.md`, *Observers*
    signal            something a module tells the others, sent as an entity that carries a
                      component for it. Not an "event"

**Storage**

    storage           what keeps the rows: `src/storage.c`. `storage.md`. Never "store",
                      which is wasmtime's word for one of its own things
    archetype         a set of components
    table             the storage of one archetype: a column a component
    storage layout    how a component is kept: table, sparse set, toggle, direct-indexed,
                      singleton. Each is named for what keeps the component.
                      `storage.md`, *Storage layouts*
    sparse set        a component's own packed array, and a lookup by entity
    toggle            a bit a row that turns a component on or off where it is
    direct-indexed    the value of entity i at place i
    singleton         one value for the deployment
    tag               a component with no bytes
    the entity index  the array a module reads to find an entity. "an id's index" is the
                      number in the low half of an id
    span              what a system is handed for one table: a count, and a base a term

**Memory**

    memory            what a module addresses: the shared memory, its private memory
    block             the allocation behind a memory
    region            a named part of a memory. `memory.md` has the seven
    arena             a region handed out by advancing an offset: scratch, persistent
    memory layout     the regions of a memory and where each begins
    ABI layout        the bytes of a struct `bridge_abi.h` states. `wasm.md`
    shared memory     the one every module sees
    shared component  one that more than one module declares
    share             a module's own part of the persistent arena, and nothing else

**Saying things**

    record            what reaches the host's sink. Any other is qualified: an "index
                      record", a "ring record", a "trigger record", a "module record"
    key               a name in a deployment's config. `config.md`
    setting           a value a host hands over, for a module's config component or a backend
    budget            a key that sizes something, stated before it is needed. `budgets.md`
    prior art         a project read for its reasoning: idTech3, flecs, Godot. Not "reference"
    citation          a document's name and one of its headings, as prose points at them.
                      `tools/xref.py` checks that each resolves

## Where to start

`design.md`, *Why*, is three paragraphs and explains the whole shape. Then `module.md` for what a
module is, and `data.md` for what it works on. Everything else answers a question those raise.
