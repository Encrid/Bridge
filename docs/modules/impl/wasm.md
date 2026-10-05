# Modules on the wasm backend

On the wasm backend a module is a core wasm binary with a `bridge:manifest` section. What it can *do* is
`module.md`; this file is how wasm binds to it.

Read `module.md` first, then `data.md` and `memory.md`.

## Memories

A wasm module has a memory of its own and imports two of Bridge-ASM700's:

    own        defined by the module    stack, heap, whatever its language needs
    shared     `bridge.shared`          component columns, the entity index, the persistent arena
    private    `bridge.private`         its spans, its column_base, its scratch arena, its log ring

**They go by name, never by number.** Wasm numbers imported memories before a module's own, so in
the binary the shared and private memories are indices 0 and 1 and the module's own comes last -
the reverse of how one would count them. An import is matched by its name, and so is every mention
in these documents.

Three, and the split is by **who else reads it**. The shared memory is shared because sharing is the
point: two modules iterating `game:Position` address one column. The private memory is what
Bridge-ASM700 writes and only this module reads, so another module's bad arithmetic cannot land
there - wasm's bounds check does the work, which is the mechanism idTech3's `& dataMask` used.

That matters more than it looks, because the private memory holds the `base` and `count` that every
address in the shared memory is computed from. Private inputs, shared data.

Not one per component and not one per access mode, for two reasons. **wasm imports are static** -
they are in the binary, so they cannot be created by a call at init. And access mode rides on a
query term, where two systems of one module may declare different access on one component, so no
arrangement of memories could express it: `data.md`, *Access mode*. The private memory is neither: it is per
module, which a static import states perfectly well.

A module that touches no storage imports neither. One that does can address all of the shared memory, and
is trusted not to write what it declared read.

**Everything is imported from `bridge`.** Core wasm names an import by a module string and a field
string, and Bridge-ASM700 is the only thing that will ever provide these - the binary is freestanding, so
there is no WASI alongside and nothing to disambiguate from. The memories are fields of it too,
`shared` and `private`, because the memory layout and the function semantics are one contract:
change how a span is laid out and `declare`'s meaning changes with it.

`bridge-wasm.h` declares the imports, so declaration is static before it is dynamic: `declare` at
init discovers nothing, it binds names to what the binary already expects.

### A compiled module reaches them through accessors

**No C compiler can emit a load from a memory other than the module's own.** Wasm can express it and
C has the syntax for it, but LLVM's WebAssembly backend - which C, C++, Rust and zig all go through -
only carries a feature flag for multiple memories: a pointer always means the module's own.

So the accessors - `schema.md`, *What a module compiles against* - are a small module in wasm's
text format, `bridge-wasm.wat`, where a load names its memory directly. The module build merges it
into the compiled module and inlines it, with Binaryen's `wasm-merge` and `wasm-opt`. What comes out
has plain loads and stores on the shared memory in the loop and no call left: checked on a C loop
before this was written down.

**The build then strips them.** The merge leaves every accessor exported, so the build removes the
exports by their common prefix, `acc_`, and with them the bodies nothing calls any more:
`wasm-opt --remove-exports=acc_* --remove-unused-module-elements`.

Load does not check that it happened. Only a module can name one of its exports to the scheduler,
through `system`, and every accessor takes a parameter, so the `() -> i32` check refuses each of
them already - `signatures.md`, *Imports are declared, exports are checked*. Refused: refusing every
export `init` did not register, which would refuse the `memory` and `__heap_base` a compiler emits
unasked and make every toolchain strip to be loaded at all. The condition that brings it back is an
export becoming callable by something other than its own module.

The compiler sees those accessors as opaque calls until the merge, so it cannot vectorise across
them. That is the cost, and the measurement in `PLAN.md` phase 3 is where it shows if it matters.

Refused: a rewriter of our own doing the same substitution, which is Binaryen's job done twice. The
condition that brings it back is Binaryen being unable to inline or strip something this needs.
Refused: one memory per module with the shared and private blocks mapped into it, which makes a C
pointer enough and costs the TLB aliasing `costs.md`, *What we give up*, already declines. The
condition that brings it back is the opaque calls measurably costing the loop, or LLVM learning to
address a second memory, which retires the accessor module altogether.

## Exports

    init()      -> result   declares components, systems and observers. Once per load,
                            before any of its systems run.
    shutdown()  -> result   called once, before the instance goes away. Not optional:
                            a module with nothing to release answers 0. What it asks
                            for there is applied, and what it logs reaches the sink.
    reload()    -> result   optional. Absent means "cannot be hot-reloaded". Called in a
                            new instance instead of init: it declares and registers over
                            storage that is already there - `lifecycle.md`, *The runtime
                            lifecycle*.

    <system>()  -> result   one export per system and per observer, named by the module

**One export each, not one entry taking an index.** idTech3's `vmMain(command, ...)` dispatched on
an integer because a QVM has exactly one entry by construction; core wasm has an export section, so
that limitation is not ours to inherit. The two cost the same - Bridge-ASM700 resolves the export once at
init and keeps the handle, so a frame is one call either way - and a name is what four other things
already need. The deployment's `[order]` names a system: `"game:actors:movement"`, not `"...:3"`, which
would shift the moment a system is added ahead of it. The profile reports per system rather than per
module. A trap says which one. Names also survive reordering across a reload, where indices
assigned in declaration order do not.

`init`, `shutdown` and `reload` being reserved - `abi.md`, *The ABI* - is an export-name collision
here: resolving one would hand the scheduler an export that already means something else.

Two systems sharing one body is two exports that both call it. That is the whole cost of names.

**An absent `reload` is *cannot*, and `hot_reload = false` is *may not*.** Two parties can say a
module is not reloadable and they are not saying the same thing: the author by leaving the export
out, the deployment by turning the key off. So **`hot_reload` only ever forbids** - a deployment can
decline a capability the author provided and cannot conjure one, exactly as a config key demands a
capability of a backend and never synthesises it (`capabilities.md`, *Capabilities, so the swap is
honest*). `hot_reload = true` against a module with no `reload` export is config contradicting the
artifact, and it is refused at load, by name - `failure.md`, *When something fails*.

## Reaching a component

Through its system's query. Bridge-ASM700 publishes the matched spans into the private memory at
every barrier, and iterating them is two loops and no call. The layout is in `query.md`, *The
interface is a query*; following a single reference by id is in `reference.md`, *Reference
following*.

A fixed header at offset 0 of the private memory says where each of this module's regions begins.
Bridge-ASM700 owns that memory outright, so there is nothing to negotiate and no capacity for the
module to declare - see `memory.md`, *Where the seven regions live*, for what the header holds and
why no module could have sized it.

## The published layouts

A module reads Bridge-ASM700's structures with plain loads, so both sides have to lay them out
identically: the span, the private memory's header, the log record, the entity-index record,
`column_base`'s indexing, an observer's list, and the pair a field holds for text.

**One header, `bridge-abi.h`, compiled by Bridge-ASM700 and by every module, on either backend.** Nothing
re-derives anything, so there is nothing to disagree about - the same argument that made the schema
a header rather than a format. idTech3 did this with `g_public.h`, and its enforcement was a comment
in capitals: *"DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER EXPECTS THE FIELDS IN THAT ORDER!"*.
Ours is the ABI version in `bridge:manifest` - change a layout, bump the version, and a module
built against the old header is refused by name.

A module that is not C cannot include the header, so it is **normative rather than mandatory** -
bindings are derived from it, and the ABI version catches drift. The arrangement WASI has between
its interface definition and its hand-written bindings.

**The native backend's `bridge_api` is not in it.** A struct of C function pointers has nothing a
wasm module could derive, since it gets imports rather than pointers. It lives in
`bridge-native.h`, which only a native module and the native backend compile - `native.md`, *What
the backend provides*.

### It holds no content

This is where we part from idTech3. Its `g_public.h` declares `entityState_t` - origin, angles,
modelindex, solid - and asks the game to begin its own struct with that prefix, because idTech3 is a
game engine and knows what an entity is. **Bridge-ASM700 orchestrates and knows nothing.** `bridge-abi.h`
says how to find data and never what data there is: no component, no field, and no prefix a module
must start its struct with.

**Bridge-ASM700 declares no components at all**, not even about itself - see `no-builtins.md`, *No
builtins*. The line it does draw is the log record's: Bridge-ASM700 owns the record's shape, the module
owns the message. No field name of a component is in the header either, even though Bridge-ASM700 has
them - `schema.md`, *There is no schema format*.

## Identity

A `bridge:manifest` custom section. What it carries and what uses it is `module.md`, *Identity*.
What is specific to this backend is that **the ABI version in it is the only check on Bridge-ASM700's own
import set.** Core wasm matches an import by field name and numeric signature, so a change in what
`declare` *means* would link silently - the numbers still agree. A renamed field surfaces as
wasmtime's "unknown import"; a changed meaning surfaces only here, as Bridge-ASM700 saying the module
wants ABI 1 and it provides 2.

Custom sections are core wasm, so nothing about this depends on the component model.

## What a wasm module cannot do

**Reach memory it did not import** - the one thing this backend adds to `module.md`, *What no
module may do*. There is no such import, so it is the only refusal in the design that is a wall
rather than a contract, and it is the whole of what the sandbox guarantee means in practice.

The word is *cannot* here and *must not* in `native.md`, which is the difference the three
guarantees name.
