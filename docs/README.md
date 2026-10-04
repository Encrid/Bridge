# BridgeC

A rewrite of Bridge in C. The name is temporary: this replaces Bridge when it is ready.

**Two build targets, because they are two concerns.** BridgeC is a **library**. **IdeaTek** is the
standalone binary over it, and a host with its own loop links the library and never ships IdeaTek.

Three documents sit beside this one. `DESIGN.md` is why the shape is what it is and what it costs.
`PLAN.md` is the build order. `PHASE0.md` is how a decision gets made here at all, which is method
rather than design.

## Three layers, three seams

**The tree is the structure**, so the layers are directories rather than a diagram to keep in step
with. One concern per document, in reading order:

    README.md            this file
    DESIGN.md            why, and what the design is
    PLAN.md              the ten phases, each with a done-when that can fail

    design/
        no-builtins.md       the rule, and its universality test
        split.md             which side owns what
        costs.md             what we give up

    plan/
        PHASE0.md            how we decide things. method, not design
        phaseN.md            one per phase, written before that phase starts

    host/
        ideatek.md           the binary: the loop, the flags, when to stop, its keys
        config.md            the three config layers, and who owns which table
        config-changes.md    Bridge's keys: what changed, and what was killed

    seam/host-api/           host to BridgeC
        host-api.md          the surface and its structs
        create.md            bridge_create, and the sink
        loop.md              who advances, and who decides when it ends
        lifecycle.md         load, unload, reload
        introspection.md     the one pull about elements

    seam/vm-surface/         BridgeC to a VM backend
        vm-surface.md        the fifteen entries, memory, the clock
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
        log.md               the ring an element writes
        profile.md           per-system counters

    elements/
        element.md           what an element is: identity, the rules, the guarantees
        abi.md               the eight entries
        grants.md            whose component it is
        module-abi.md        wasm mechanics
        plugin-abi.md        native mechanics
        plugin-threads.md    pinned
        engine-callbacks.md  answering an engine from storage
        schema.md            how an element declares its data

The seams are host to BridgeC, BridgeC to a VM backend, and BridgeC to elements. The first two have
a directory of their own under `seam/`: a separate build target sits on the far side, and filing the
contract under either side would suggest that one owns it. The element seam is documented in
`elements/`, beside what an element is.

A backend is below the orchestrator rather than being a fourth layer: it has no elements and no
deployment, it runs code. The one thing it is configured with is its own - a cache directory, a tick
period: `vm-surface.md`, *The backend owns its clock*.

**One header per seam**, so nothing compiles a declaration it cannot use:

    bridge.h          host seam: the functions and their structs
    bridge-abi.h      element seam: the layouts both sides read with plain loads -
                      `module-abi.md`, *The published layouts*
    bridge-plugin.h   element seam, native only: the vtable - a module gets imports,
                      not pointers
    bridge-vm.h       VM seam: the fifteen entries in `vm-surface.md`

**Prose cites a document by name, never by path** - `data.md`, not `bridge/data/data.md` - because a
reader wants the document and not its location, and `tools/xref.py` resolves the name.

## Where to start

`DESIGN.md`, *Why*, is three paragraphs and explains the whole shape. Then `element.md` for what an
element is, and `data.md` for what it works on. Everything else answers a question those raise.
