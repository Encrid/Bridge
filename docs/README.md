# BridgeC

**Draft.** Open items are tracked in OPEN.md; nothing here is settled until that register is empty.

A rewrite of Bridge in C. The name is temporary: this replaces Bridge when it is ready.

**Two build targets, because they are two concerns.** BridgeC is a **library**. **IdeaTek** is the
standalone binary over it, and a host with its own loop links the library and never ships IdeaTek.

Three documents sit beside this one. `DESIGN.md` is why the shape is what it is and what it costs.
`PLAN.md` is the build order. `PHASE0.md` is how a decision gets made here at all, which is method
rather than design.

## Three layers, two seams

**The tree is the structure**, so the layers are directories rather than a diagram to keep in step
with. One concern per document, in reading order:

    README.md            this file
    DESIGN.md            why, what the design is, the split, what it costs
    PLAN.md              the ten phases, each with a done-when that can fail
    OPEN.md              the register

    plan/   PHASE0.md    how we decide things. method, not design
            phaseN.md    one per phase, written before that phase starts

    host/   ideatek.md   the binary: the loop, the flags, when to stop, its keys
            config.md    the three config layers, and who owns which table

    seam/   host-api.md  host to BridgeC: structs in, phases driven
            vm-surface.md  BridgeC to a VM backend

    bridge/ bridge.md    phases, barriers, the conflict graph, the ordering
                           refusal, what a pass depends on, failure policy
            data.md      components, entities, queries, observers, structural
                           change, reference following, lifetime
            memory.md    the seven regions, the two arenas, budgets, threads,
                           and where flecs allocates
            diagnostics.md  what a bad write costs, the log, the profile

    elements/ element.md  what an element is and can do: the ABI, grants, the
                            three guarantees, what decides which kind
            module-abi.md  wasm mechanics
            plugin-abi.md  native mechanics
            schema.md    how an element declares its data

A seam is its own directory because it belongs to neither side: each is a contract between two
layers, and filing it under one of them would suggest that one owns it.

A backend is below the orchestrator rather than being a fourth layer: it has no elements and no
deployment, it runs code. The one thing it is configured with is its own - a cache directory, a tick
period: `vm-surface.md`, *The backend owns its clock*.

**One header per seam**, so nothing compiles a declaration it cannot use:

    bridge.h          host seam: the functions and their structs
    bridge-abi.h      element seam: the layouts both sides read with plain loads -
                      `module-abi.md`, *The published layouts*
    bridge-plugin.h   the vtable. native only - a module gets imports, not pointers

**Prose cites a document by name, never by path** - `data.md`, not `bridge/data.md` - because a
reader wants the document and not its location, and `tools/xref.py` resolves the name.

## Where to start

`DESIGN.md`, *Why*, is three paragraphs and explains the whole shape. Then `element.md` for what an
element is, and `data.md` for what it works on. Everything else answers a question those raise.
