# The data model

**Draft.** Open items are tracked in OPEN.md; nothing here is settled until that register is empty.

What a component is, how an element reaches one, and when the world changes. Where the bytes live is
`memory.md`; how you find out what happened is `diagnostics.md`.

BridgeC embeds **flecs**. Storage, archetypes, queries, entity ids and the conflict information the
scheduler needs are all flecs'. What BridgeC owns is the seam: getting query results somewhere a wasm
module can read them with plain loads.

flecs is an **implementation detail, not the contract**. No element sees it - not a module, and not a
plugin. The ABI is BridgeC's own, which holds it to the eight entries in `element.md`, *The ABI*,
rather than letting flecs' whole surface become what a module author has to know - half of which
cannot cross a wasm boundary anyway.

Throughout these documents, **element** means a module or a plugin - the word for when something is
true of both.
## Why the VM is swappable and this is not

The VM surface abstracts a **behaviour**: BridgeC hands over bytes, memories and host functions and
asks for a function to be run, never looking inside. That is why a native backend implements the same
fifteen entries and nothing leaks.

An ECS surface would have to abstract a **memory layout**. BridgeC needs flecs' column addresses,
because a span's `base[]` *are* offsets into its columns; its entity index, to mirror; a row move it
does not publish at all, so the mirror cannot drift; and control of its allocator, so columns land
inside memory 1.
None of that is what an ECS does - it is how *this* ECS stores things, so the surface would be shaped
by flecs and leak its only implementation. And confinement settles it by force - *It can be
confined, and the patch is thirteen sites* - because a library you patch is a fork you ship rather
than a backend you swap.

Underneath is the trade the rewrite rests on. **Zero-copy direct access to the ECS's memory is
incompatible with treating the ECS as a black box.** A swappable ECS would mean asking it for data,
which is a copy or a call per access - the cost we are here to remove.


## Why flecs rather than our own

The model we reasoned out from scratch turned out to be flecs' model, term for term: components
stored by type, entities as ids carrying a generation, access modes declared for scheduling,
structural change deferred to a merge point, pipeline phases. Writing our own would have produced a
worse flecs.

The cost is honest: 116k lines of C we did not write, in a project whose pitch is minimal and
explicit. It is a deliberate trade. The ECS is where a decade of tuning matters most and where our
own attempt would have been weakest, and it is MIT and already vendored in Bridge-Ironwail.

## Components

A component is a C type registered with flecs. Its size and alignment come from the type; which
types the schema can express, and how two elements are held to the same layout, is `schema.md`.

Its name is namespaced - `game:Position` - and who may name it is `element.md`, *Whose component it
is*. No version in the name: the fingerprint in `schema.md` is what detects a layout change, so a
version would be a second answer to the same question.

`string` and `list` are not components. See *Variable length*.

## Entities

A flecs id: an index and a generation packed into a `u64`. An id outlives the row behind it; a row
does not outlive a barrier.

## The interface is a query

You do not get storage. You get your query's results, which is how an ECS is used:

    declare(...)   registers a component type
    system(...)    registers a query, a function and a phase together, and gets a slot

At each barrier BridgeC runs every registered query and writes the matched spans into the element's
private memory - memory 2, which nothing else reads; see `module-abi.md`, *Memories*. One span per
matched table - exactly what `ecs_query_next` yields inside BridgeC, materialised for a reader that
cannot call it.

    per query slot:
        u32 spans      how many tables matched
        u32 at         where this query's spans begin

    per span:
        u32 count      rows in this table
        u32 entities   where the entity-id array for these rows begins
        u32 base[n]    one offset per term, in declared order. zero = absent here
        u64 source[n]  entity a term came from. zero = per-row on the matched entity

**This is flecs' encoding, not an invention.** `ecs_iter_t` (`flecs/include/flecs.h:1192`) carries
`entities`, `ids` and `sources` per field. The three cases a reader must tell apart are the three
flecs' own JSON serialiser tells apart when it publishes an iterator to an external consumer, in
`src/addons/json/serialize_iter_result_query.c:177-186` - and it tells them apart with two null
checks, `!ptr` then `!it->sources[f]`, which is why nothing here carries a flag. **Offset 0 of memory
1 is reserved** so that zero is never a valid base, which is also what makes the same sentinel sound
in *Reference following*.

Iterating is two loops and no call, with the branch outside the row loop:

    for each span:
        for each term t:
            if   base[t] == 0     term absent in this table
            elif source[t] != 0   one value, at base[t], for every row
            else                  row r is at base[t] + r * sizeof(T)

`sizeof(T)` is a build-time constant from the schema, so stride is never published - the generator
already knows it.

The span is laid out at natural alignment, so `source[]` begins at the next multiple of eight after
`base[]`. And a query takes at most 32 terms, inherited from flecs' `FLECS_TERM_COUNT_MAX`.

`entities` is there because a system that wants to destroy the row it is on, attach a component to it,
or store a reference to it needs the id and has no other route to one. `source[t]` does double duty:
zero says the term is per-row on the matched entity, and non-zero both says it is shared and names the
entity it came from - which is what makes an inherited term one load per span rather than one per row.

### What a query may not ask for

**Or-terms and wildcards.** Their matched id varies per table, which is why flecs publishes `ids` per
field. Our generator emits a **typed** accessor at build time, and a term whose type changes per table
has no type to emit - so a wildcard is incompatible with the generator rather than merely awkward in
the span.

**Terms needing `field_at`** - flecs' `row_fields` - because a field that is not an array has no
base-and-stride form at all.

Both are refused at `system` and `observe`, by name - that is where terms are handed over, not
`declare`, which only registers a type. `EcsQueryCacheAll` makes flecs fail query creation unless
every term matches whole archetypes, which is what one-span-per-table assumes; wildcards pass that
test, so BridgeC refuses those by inspecting the terms it was handed; and `row_fields` catches the
rest after creation. So every refusal has a diagnostic and none costs a frame.

Many tables per query is the normal case rather than an awkward one: that is what archetypes mean.
And publishing costs **O(matched tables)**, not O(data), which is the property that makes this
scale. A query over ten million entities costs the same to publish as one over a thousand.

## Access mode

Query terms carry it, as flecs' do: `in`, `out`, `inout`. It is a **scheduling input and nothing
else** - BridgeC uses it to know which elements may run at once, and flecs' pipeline uses it for the
same purpose inside BridgeC.

It is not enforced, which is how every ECS works. An element that writes a term it declared `in` is
a bug in that element, no different from one writing nonsense into a term it owns - and the second
is indistinguishable from corruption anyway, so policing the first would narrow the blast radius
without closing it. wasm has no read-only memory import in any case.

What is enforced is the boundary that matters: an element reaches only the memories it imports, so
it cannot touch the host, another element's memory, or anything it was not given.

## Structural change

Deferred, into flecs' command queue, applied at the merge point - which is our barrier, and there is
one after every phase, and **no phase is an exception** - there is no kind of phase that could be
one, since a phase is only a name: `bridge.md`, *A phase is a name and nothing else*. A span is a
snapshot taken at the last barrier, so a change applied sooner than the next one is a change nothing
can address.

The word **exclusive** survives in these documents with one meaning only, and it is not about
phases: a component is *exclusive* when one element declared it and nobody else did - *Lifetime*.

What an element observes from that - when a spawn becomes visible, and why everyone agrees on one
world per phase - is `bridge.md`, *Phases*. This section is the mechanism only.

**Unload drops an element's exclusive components and keeps the shared ones.** Keeping everything
would leave orphaned rows accumulating with nothing to collect them; dropping the shared ones would
destroy data that is not this element's to destroy. So the exclusivity line is the only one that
holds.

**Unload is therefore the verb that loses data and reload is the verb that keeps it**, which is why
reload needs no flag to choose - *Lifetime*.

## Reference following

Iterating a query covers the hot path, but not everything: a projectile crediting its owner, a monster
reading its target, a trigger reaching its door. Following a reference means reaching *one* entity by
id, and that is a lookup rather than a walk.

It is arithmetic, not a call. BridgeC publishes two flat arrays and the whole of `get` is:

    record = entity_index[id & 0xffffffff]     // u32 table, u32 row, u32 generation
    if record.generation != id >> 32           -> stale id, refuse
    base   = column_base[record.table * terms + slot]
    if base == 0                               -> entity lacks this component
    ptr    = base + record.row * sizeof(T)

Two dependent loads and a multiply. No crossing, which is the point: a crossing per reference would
reintroduce a per-operation cost, and per-operation cost is what this whole design exists to remove.

`column_base` is needed because a component sits at a different offset in every table, so the table
alone is not enough. It is indexed by table and by the terms *this* element declared, so ten
components across a thousand tables is forty kilobytes.

**The stale-id check falls out of it.** The generation is in the record, so refusing an id whose
entity has died is one load and one compare - which nothing in an arithmetic path could otherwise do.

**What it costs.** The index changes only for entities created, destroyed, or moved between tables,
so it is patched at the merge point rather than rebuilt - **O(structural changes), unrelated to world
size**, measured at 1.4 record writes per operation where the swap-remove suggests two: 360
operations moved 503 records.

**Memory is twelve bytes times the highest entity id** rather than the live count - though flecs
recycles ids, so that tracks peak concurrent entities and not everything ever spawned. That is a
**budget declared at startup**, and the `spawn` that would exceed it fails, fatally and where it
happened: idTech3's discipline rather than an apology for a limitation, and growing it instead is
not available - `memory.md`, *Budgets*.

### flecs will not tell you which

The one thing a spike had to settle, because getting it wrong is silent. flecs' events name every
entity whose **components** changed and **no entity whose row merely moved** - of those 503 changes,
303 had no event naming them, every one a pure row move inside a single table, 0.8 per operation. An
event-driven mirror would point a module at the wrong row that often, and nothing would say so.

So the mirror takes **one more patched site** than the columns did - `flecs_table_delete`, in the
file *It can be confined, and the patch is thirteen sites* already forks, which makes it fourteen.
flecs computes the moved entity and its destination row there and updates its own record with them;
the patch is to say so out loud. That option is cheap only because the columns settled first: a
callback into a library we were not already forking would have been a different question.

**Table ids are BridgeC's own.** `ecs_record_t` is public and carries an `ecs_table_t *`, where a
span needs a dense `u32` to index `column_base` - so BridgeC keeps pointer-to-id itself, off
`EcsOnTableCreate` and `EcsOnTableDelete`, and never reads `ecs_table_t`'s layout.

**What `get` does not buy.** Retrieval is memory-bound, not lookup-bound: two dependent cache misses,
roughly ten nanoseconds warm and a hundred and sixty cold. A crossing would pay those same misses and
add the call, so this is cheaper by exactly the crossing - the irreducible cost is pointer-chasing.
Which is why `get` is for following a reference a few times, and a relationship you walk every frame
belongs in a query instead, where access is sequential and the prefetcher does the work.

## Observers

A system runs every pass. An **observer** runs when its query's data changes - `flecs.h:453`, *"an
observer is a system that is invoked when an event matches its query"* - with `ON_ADD`, `ON_SET` and
`ON_REMOVE` as the triggers.

It runs **at the barrier where the change applied**, which is why it takes no phase - and flecs
arranges that itself, which the spike checked rather than assumed. An observer on an *immediate*
operation does fire synchronously, on the caller's stack, mid-phase, against spans that are a
snapshot from the last barrier - the one thing `bridge.md`, *Phases*, does not allow. But **every
structural change here is deferred** - `element.md`, *The ABI* - and under flecs' own defer not one
invocation arrives early: 360 deferred operations produced 360 observer calls, all at the merge
point and none during the batch.

So BridgeC uses flecs' observers as they are. Nothing to arrange, and no change detection to write:
an observer runs where the world is already consistent, and never inside somebody else's work.

**This is how an event crosses.** A plugin with a fired signal spawns an entity carrying an event
component; the change applies at the barrier and the subscriber's observer runs there. Same latency a
dedicated ring would have given, since there is a barrier after every phase - and the entity ceiling
already budgets it, so there is no second size to pick and no overflow policy to invent.

It is also the answer to a question that used to need a mechanism: **an element never asks another
element for anything.** It writes, and whoever subscribed to that shape acts. A system subscribes to
a shape; an observer subscribes to a change.

## Lifetime

Storage is flecs' and outlives the element that declared a component, so a reloaded module finds its
components still there - the behaviour Bridge does not have today. What happens when the layout has
changed is *When the layout changed*, below.

**Reload has one mode and it keeps the components**, which is the whole point of it. There is no
drop-and-restart variant, because `bridge_unload` followed by `bridge_load` already is one and its
verb says what it costs - `host-api.md`, *The runtime lifecycle*.

That is also what idTech3 does. `VM_Restart` exists to *"reload the data, but leave everything else
in place... without changing memory allocation"* (`vm.c:349`) - the allocation survives, which is
this mode and not a reset. And where in-place is impossible it does not invent a third thing:
*"DLL's can't be restarted in place"* and then `VM_Free` followed by `VM_Create` (`:363`).

BridgeC knows which components an element declared exclusively, because `declare` is idempotent and
records its callers - a set operation when it matters and nothing per frame. A module that corrupted
a *shared* component is not cleaned up by any of this.

### When the layout changed

The fingerprint catches it - `schema.md`, *Fingerprints* - and what happens next depends on who else
holds the component.

**Shared: refused, always.** If two elements declare `game:Position` and one is rebuilt with a
different `Position`, the other is still running with the old struct compiled in, and a column can
only have one layout. Dropping takes the other element's data; migrating leaves it reading new bytes
with old code. Nothing reconciles it, so this is forced rather than chosen.

**Exclusive: refused as well, and the refusal cannot be waived.** Dropping would harm only this
element, so it looks viable - but **BridgeC never discards data on a call that means "keep running"**,
and a flag to switch that off would be a flag whose job is disabling a check. The refusal names the
component and both fingerprints, so the author knows what they changed.

An author who accepts the loss unloads and loads. That is the same outcome, through a verb that
already means the data goes - which is where the consent belongs, rather than in a boolean nobody
can read at a call site.

**Migration is not BridgeC's.** It cannot be: a fingerprint is a hash, so it detects difference and
cannot describe it, and a converter the element supplied would have to read a layout its new build
does not contain.

The ECS already offers migration in its own terms. **Declare a new name**: run a system that reads
`game:Position` and writes `game:Position2`, then stop declaring the old one. Two components, both
valid, no layout changing under anyone. So the rule is **change the bytes, change the name** - a
layout change to an existing name cannot work, and a new name is free.

Quake avoided this rather than answering it: `pr_edict_size` comes from the loaded progs
(`pr_edict.c:1021`), so reloading re-strides every edict - and progs is only loaded at
`SV_SpawnServer`, where edicts are fresh. Reload meant level restart.

