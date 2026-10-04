# Plan

The build order, and nothing else. What the design is and why is `DESIGN.md`; what the documents are
is `README.md`.

**Every phase ends in a claim that can be false.** A phase is not done because its code exists. It
is done when the stated thing is observably true. A phase whose done-when cannot fail will be
declared finished while broken.

## 0. Documents, and the spikes they need

The design documents `README.md` lists, `PHASE0.md` and `OPEN.md` aside. Cheap to change now,
expensive later.

Two questions could not be settled by argument, so a throwaway spike settled them inside this phase:
whether flecs can be confined to our block, and whether its events are enough to keep the entity
mirror in step. Both answered with numbers, and the second answered *no* - `flecs.md`, *It can be
confined, and the patch is thirteen sites*, and `flecs.md`, *flecs will not tell you which*.

*Done when OPEN.md is empty and every one of them is out of draft.*

## 1. Vendor and patch flecs

Vendored, pinned, and patched at fourteen sites in two files - `flecs.md`, *How it is carried*,
for the shape of all three, and `flecs.md`, *It can be confined, and the patch is thirteen sites* for the
thirteen plus the one `flecs.md`, *flecs will not tell you which*, adds.

**First, because everything else needs it.** Columns cannot be in memory 1 without it, and the
mirror cannot be correct without the fourteenth site.

*Done when the spike's own assertion holds as a build step rather than as a throwaway, and when the
patch script refuses a flecs version it was not written for.*

## 2. One module runs

A core module through wasmtime's C API, importing a BridgeC memory, which means `wasm_multi_memory`
on since the module has its own. `bridge-abi.h` begins here, because a module reading memory 1 with
plain loads needs the span layout. `define_func` with a real signature - `signatures.md`, *Imports
are declared, exports are checked* - `instance_func`'s `() -> i32` check, and `call`.

*Done when a module writes memory 1, the host reads the same bytes, a deliberate out-of-bounds
returns an error instead of killing the process, and an export whose shape is not `() -> i32` is
refused at load rather than on the first frame.*

## 3. Storage

flecs' columns inside the shared memory a module imports, plus each element's own memory 2 carrying
its spans and `column_base`. `declare`, the published query spans, and the entity index mirror
driven off the patched callback and `EcsOnTableCreate` / `EcsOnTableDelete`. Access mode and queries
are part of declaration from the start, not retrofitted. The schema generator: the `ECS_STRUCT` shim
and accessors carrying the `row < count` test.

*Done when two modules share a component array with no crossing per access, neither can reach the
other's spans, a mirror lookup survives a batch of swap-removes, and the accessor's test is measured
against a build without it* - `validation.md`, *Always on*, states that cost as reasoning, and this
is where it becomes a measurement.

## 4. Plugins

The three lifecycle symbols and `bridge-plugin.h`. A plugin's systems are function pointers the
scheduler cannot distinguish from a module's exports, and its storage access is `descriptors()` and
`region()` over the same spans.

*Done when a plugin iterates a component array through a raw pointer, and one of its systems and one
of a module's run in the same phase with neither able to tell.*

## 5. Scheduling

Phases from the host, a barrier after each, and the refusal when two conflicting systems in one
phase are left unordered. Observers at the barrier, one hop per barrier. Threads running
non-overlapping writers, which the declared sets give us.

*Done when a deployment orders two conflicting systems with `[order]` alone, an ambiguous pair is
named instead of guessed, a component a plugin sets runs a module's observer at that same barrier, a
three-deep observer chain settles in three barriers rather than hanging, and two elements provably
run in parallel.*

## 6. The host seam, and IdeaTek

`bridge.h`: `bridge_create`, `bridge_phase`, `load`/`unload`/`reload`, and the binary that uses
them. The seam is tested by existing - if IdeaTek links the library and nothing else, it is real.

*Done when `--once` runs a deployment, a host linking only the library needs no part of IdeaTek, and
`bridge_create` refuses a config with no phases.*

## 7. Config and discovery

The three layers resolved into structs by the host - `bridge_manifest_parse` beside the library, not
inside it. `[content]` walked with ignore patterns, `[element.defaults]`, and the per-element keys
with their periods.

*Done when a real `config.toml` drives a deployment, `--watch` reloads a changed module, a key
omitted by all three layers is an error naming the key, and a key set under a disabled table is
refused rather than ignored.*

## 8. Diagnostics

The log ring drained to the sink, with its text always in the scratch arena. `[checks] validate` and
`[checks] profile` - the barrier's checksum pass and the per-system counters. `bridge_elements` and
`bridge_profile`, both pulls. A journal is this machinery with a blocking sink instead of a dropping
one - `schedule.md`, *What a pass depends on*, for the capture point and why there is only one - and
lands here if a deployment wants it.

*Done when a deliberate stray write is reported with the element that made it, a system made slow on
purpose is the one the profile names, and an element stopped past its failure budget shows as
STOPPED rather than having to be inferred from log lines.*

## 9. Prove it

Port the Godot adapter. The probe asserts 191 checks today and two of them ask a module a question
synchronously - `costs.md`, *What we give up*, names both - so 189 is the number that can pass.

*Done when 189 of the probe's checks pass, the two that cannot are the two named, and the GDExample
frame is an order of magnitude under 42 us. If it is not, the architecture did not deliver and we
find out here.*

---

Throughout: the Rust Bridge is the reference implementation. Same modules, same observable
behaviour. A divergence is either a deliberate design change or a bug.
