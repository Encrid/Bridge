# Module ABI

A module is a core wasm module with a `bridge:manifest` section. What it can *do* is `element.md`;
this file is how wasm binds to it.

Read `element.md` first, then `data.md` and `memory.md`.

## Memories

A module has its own private memory and imports two of BridgeC's:

    memory 0   its own     stack, heap, whatever its language needs
    memory 1   "shared"    component columns, the entity index, the persistent arena
    memory 2   "private"   its spans, its column_base, its scratch slice, its log ring

Three, and the split is by **who else reads it**. Memory 1 is shared because sharing is the point:
two modules iterating `game:Position` address one column. Memory 2 is what BridgeC writes and only
this module reads, so another module's bad arithmetic cannot land there - wasm's bounds check does
the work, which is the mechanism idTech3's `& dataMask` used.

That matters more than it looks, because memory 2 holds the `base` and `count` that every address in
memory 1 is computed from. Private inputs, shared data.

Not one per component and not one per access mode, for two reasons that still hold. **wasm imports
are static** - they are in the binary, so they cannot be created by a call at init. And access mode
rides on a query term, where two systems of one module may declare different access on one component,
so no arrangement of memories could express it: `data.md`, *Access mode*. Memory 2 is neither of
those things: it is per element, which a static import states perfectly well.

A module that touches no storage imports neither. One that does can address all of memory 1, and is
trusted not to write what it declared read.

**Everything is imported from `bridge`.** Core wasm names an import by a module string and a field
string, and BridgeC is the only thing that will ever provide these - modules are freestanding, so
there is no WASI alongside and nothing to disambiguate from. The memories are fields of it too,
`shared` and `private`, because the memory layout and the function semantics are one contract: change
how a span is laid out and `declare`'s meaning changes with it.

The generator emits the imports, so declaration is static before it is dynamic: `declare` at init
discovers nothing, it binds names to what the binary already expects.

## Exports

    init()      -> result   declares components, systems and observers. Once, before any
                            phase runs.
    shutdown()  -> result   called once, before the instance goes away.
    reload()    -> result   optional. Absent means "cannot be hot-reloaded".

    <system>()  -> result   one export per system and per observer, named by the module

**One export each, not one entry taking an index.** idTech3's `vmMain(command, ...)` dispatched
on an integer because a QVM has exactly one entry by construction; core wasm has an export section,
so that limitation is not ours to inherit. The two cost the same - BridgeC resolves the export once
at init and keeps the handle, so a frame is one call either way - and a name is what four other
things already need. The deployment's `[order]` names a system: `"mymod:movement"`, not `"mymod:3"`,
which would shift the moment a system is added ahead of it. The profile reports per system rather
than per module. A trap says which one. Names also survive reordering across a reload, where indices
assigned in declaration order do not.

`init`, `shutdown` and `reload` being reserved - `element.md`, *The ABI* - is an export-name
collision here: resolving one would hand the scheduler an export that already means something else.

Two systems sharing one body is two exports that both call it. That is the whole cost of names.

**An absent `reload` is *cannot*, and `hot_reload = false` is *may not*.** Two parties can say a
module is not reloadable and they are not saying the same thing: the author by leaving the export
out, the deployment by turning the key off. So **`hot_reload` only ever forbids** - a deployment can
decline a capability the author provided and cannot conjure one, exactly as a config key demands a
capability of a backend and never synthesises it (`vm-surface.md`, *Capabilities, so the swap is
honest*). `hot_reload = true` against a module with no `reload` export is config contradicting the
artifact, and it is refused at load, by name - `bridge.md`, *When something fails*.

## Reaching a component

Through its system's query. BridgeC publishes the matched spans into memory 2 at every barrier, and
iterating them is two loops and no call. The layout is in `data.md`, *The interface is a query*;
following a single reference by id is in *Reference following*.

A fixed header at offset 0 of memory 2 says where each of this module's regions begins. BridgeC owns
that memory outright, so there is nothing to negotiate and no capacity for the module to declare -
see `memory.md`, *Where the seven regions live*, for what the header holds and why no element could
have sized it.

## The published layouts

A module reads BridgeC's structures with plain loads, so both sides have to lay them out identically:
the span, memory 2's header, the log record, the entity-index record, and `column_base`'s indexing.

**One header, `bridge-abi.h`, compiled by BridgeC and by every module.** Nothing re-derives
anything, so there is nothing to disagree about - the same argument that made the schema a header
rather than a format. idTech3 did this with `g_public.h`, and its enforcement was a comment in
capitals: *"DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER EXPECTS THE FIELDS IN THAT ORDER!"*. Ours
is the ABI version in `bridge:manifest` - change a layout, bump the version, and a module built
against the old header is refused by name.

A module that is not C cannot include the header, so it is **normative rather than mandatory** -
bindings are derived from it, and the ABI version catches drift. The arrangement WASI has between its
interface definition and its hand-written bindings.

**The plugin vtable is not in it**, and that paragraph is the reason: a vtable of C function pointers
has nothing to derive, since a module gets imports rather than pointers. It lives in
`bridge-plugin.h`, which only a plugin and BridgeC compile.

### It holds no content

This is where we part from idTech3, and it is not a detail. Its `g_public.h` declares `entityState_t`
- origin, angles, modelindex, solid - and asks the game to begin its own struct with that prefix,
because idTech3 is a game engine and knows what an entity is. **BridgeC orchestrates and knows
nothing.** `bridge-abi.h` says how to find data and never what data there is: no component, no field,
and no prefix a module must start its struct with.

**BridgeC declares no components at all**, not even about itself - see `DESIGN.md`, *No builtins*. The
line it does draw is the log record's: BridgeC owns the record's shape, the element owns the message.
No field name of a component is in the header either, even though BridgeC has them - `schema.md`,
*There is no schema format*.

## Identity

A `bridge:manifest` custom section. What it carries and what uses it is `element.md`, *Identity*;
what is module-specific is that **the ABI version in it is the only check on BridgeC's own import
set.** Core wasm matches an import by field name and numeric signature, so a change in what `declare`
*means* would link silently - the numbers still agree. In the import string a renamed field surfaces
as wasmtime's "unknown import"; a changed meaning surfaces only here, as BridgeC saying the module
wants ABI 1 and this host provides 2.

Custom sections are core wasm, so nothing about this depended on the component model.

## What a module cannot do

**Reach memory it did not import** - the one thing this kind adds to `element.md`, *What no element
may do*. There is no such import, so it is the only refusal in the design that is a wall rather than
a contract, and it is the whole of what the sandbox guarantee means in practice.

The word is *cannot* here and *must not* in `plugin-abi.md`, which is the difference the three
guarantees name.
