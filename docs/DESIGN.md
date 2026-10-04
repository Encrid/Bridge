# Design

Why Bridge-ASM700 has this shape, which side owns what, and what the shape costs. What the documents are
is `README.md`; the build order is `PLAN.md`.

## Why

The original Bridge's costs were measured, not guessed. One machine, release build:

    empty tick                      300 ns
    built-in host call              500 ns
    plugin call                     900 ns
    module to module, same store   2200 ns
    module to module, across stores 6300 ns

A plain C call is 1 to 2 ns. Nothing closes a 200x gap per crossing, and the gap is wasmtime's, not
the original Bridge's. Removing every heap allocation from the crossing path bought 10%. So a frame
costs the *number* of crossings, and the way to cut it is to stop sending data through calls at all.

One example: the original's Godot adapter's per-frame work is 18 crossings, 42 us. The same work over
shared storage is one crossing.

## What the design is

**Data is declared once and addressed directly.** At registration a module declares the components
it needs and gets a slot, and Bridge-ASM700 republishes where they are at every barrier. Reading and
writing is then address arithmetic. No handle, no lookup, no crossing.

**Bridge-ASM700 owns the memory.** It is the orchestrator, so it owns the region, the schema and the
allocation policy.

**Core wasm, not components, and not WIT.** The component model exists to describe and copy values
across a language boundary. Under declare-at-init nothing is copied, so the canonical ABI has
nothing to do - and it cannot express a memory import or export, which core modules can. WIT went
with it: Bridge-ASM700 has no type system, only sizes, alignments and fingerprints, and the schema is a C
header both sides compile. See `schema.md`.

**No call carries data at all.** Every function Bridge-ASM700 calls - `init`, `shutdown`, `reload`, a
system, an observer - takes no arguments and returns an i32 status. Data has nowhere to travel
through a call, which is stronger than restricting what may travel: a design cannot drift back into
passing things. What a module calls *into* still carries scalars, and only two kinds:
`signatures.md`, *Imports are declared, exports are checked*.

**No general allocation on the per-frame path.** Anything per-frame comes from a bump region or a
fixed ring, reset or drained at a barrier. Budgets are declared at startup, and exhaustion is fatal
where it happens rather than degrading quietly later. That is idTech3's discipline: "data does not
cross" applied to memory instead of calls.

**ECS shape, on flecs.** Components are data stored per type, entities are ids, systems are declared
by modules. The native backend is for what cannot be reached from inside wasm; the wasm backend is for the
three guarantees it buys. Either way it is the same module - `module.md`.

Bridge-ASM700 embeds flecs rather than writing an ECS, and no module ever sees it: the ABI is
Bridge-ASM700's own. Why that trade, why flecs is embedded rather than swappable like a backend, and
what it costs: `flecs.md`, *Why flecs rather than our own*.

**One ABI, eight entries, and every module gets all of it.**

    provides   init, shutdown, reload, and its systems and observers
    gets       declare, system, observe
               spawn, destroy, attach, detach, alloc

**And nothing else - no module can call another.** Nothing asks; everything subscribes, which is
what an ECS already is - `abi.md`, *The ABI*. So there is no call machinery: no `expose`,
`find_func` or `call`, no service registry, no signature fingerprint, no affinity propagating along
a call graph.

How a module binds differs by backend, and nothing differs in capability: `wasm.md` and
`native.md`. `declare` carries a fingerprint - `schema.md`, *Fingerprints*, for why a hash is
enough.

**A deployment says what a module may touch.** Component names are namespaced, so it declares
freely in its own namespace and needs a grant to reach another's. That makes a game's mod API
explicit instead of defaulting to every component present, and it is one check at init.

**A backend is a shared library** behind a surface Bridge-ASM700 defines. One wraps wasmtime; the other
is native, for code that does not need sandboxing. A module names the one that runs it.

**Parallelism is derived; order is declared.** Declared read and write sets say which systems may
run at once - two writers of the same component may not - which makes the hot path lock-free by
construction. They do not say which of two conflicting systems goes *first*. Ordering comes from
phases, and where a phase leaves two conflicting systems unordered Bridge-ASM700 refuses the load
that brought the second one rather than fall back on load order.
