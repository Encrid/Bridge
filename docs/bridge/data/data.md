# The data model

What a component is and when the world changes. How a module reaches one is `query.md` and
`reference.md`; what survives a reload is `lifetime.md`. Where the bytes live is `memory.md`; why
the ECS is flecs is `flecs.md`.

Bridge-ASM700 embeds **flecs**. Storage, archetypes, queries, entity ids and the conflict information the
scheduler needs are all flecs'. Bridge-ASM700 owns the seam: getting query results somewhere a wasm module
can read them with plain loads.

flecs is an **implementation detail, not the contract**. No module sees it, on either
backend. The ABI is Bridge-ASM700's own, which holds it to the eight entries in `abi.md`, *The ABI*,
rather than letting flecs' whole surface become what a module author has to know - half of which
cannot cross a wasm boundary anyway.

## Components

A component is a C type registered with flecs. Its size and alignment come from the type; which
types the schema can express, and how two modules are held to the same layout, is `schema.md`.

Its name is namespaced - `game:Position` - and who may name it is `grants.md`, *Whose component it
is*. No version in the name: the fingerprint in `schema.md` detects a layout change, so a version
would be a second answer to the same question.

`string` and `list` are not components. See `strings.md`, *Variable length*.

## Entities

A flecs id: an index and a generation packed into a `u64`. An id outlives the row behind it; a row
does not outlive a barrier.

## Access mode

Query terms carry it, as flecs' do: `in`, `out`, `inout`. It is a **scheduling input and nothing
else** - Bridge-ASM700 uses it to know which modules may run at once, and flecs' pipeline uses it for the
same purpose inside Bridge-ASM700.

It is not enforced, which is how every ECS works. A module that writes a term it declared `in` is
a bug in that module, no different from one writing nonsense into a term it owns. The second is
indistinguishable from corruption anyway, so policing the first would narrow the blast radius
without closing it. wasm has no read-only memory import in any case.

What is enforced is the boundary that matters: a module reaches only the memories it imports, so
it cannot touch the host, another module's memory, or anything it was not given.

## Structural change

Deferred, into flecs' command queue, applied at the merge point - which is our barrier. There is one
after every phase, and **no phase is an exception**: a phase is only a name, so no kind of phase
could be one - `schedule.md`, *A phase is a name and nothing else*. A span is a snapshot taken at the
last barrier, so a change applied sooner than the next one is a change nothing can address.

A component
is *exclusive* when one module declared it and nobody else did - `lifetime.md`, *Lifetime*.

What a module observes from that - when a spawn becomes visible, and why everyone agrees on one
world per phase - is `schedule.md`, *Phases*. This section is the mechanism only.

**Unload drops a module's exclusive components and keeps the shared ones.** Keeping everything
would leave orphaned rows accumulating with nothing to collect them. Dropping the shared ones would
destroy data that is not this module's to destroy. So the exclusivity line is the only one that
holds. What becomes of the entities that carried a dropped component is `lifetime.md`, *What unload
leaves behind*.

**Unload is therefore the verb that loses data and reload is the verb that keeps it**, which is why
reload needs no flag to choose - `lifetime.md`, *Lifetime*.

## Observers

A system runs every pass. An **observer** runs when its query's data changes - `flecs.h:453`, *"an
observer is a system that is invoked when an event matches its query"* - with `ON_ADD`, `ON_SET` and
`ON_REMOVE` as the triggers.

It runs **at the barrier where the change applied**, which is why it takes no phase. flecs arranges
that itself, which is checked rather than assumed. An observer on an *immediate* operation
does fire synchronously, on the caller's stack, mid-phase, against spans that are a snapshot from
the last barrier - the one thing `schedule.md`, *Phases*, does not allow. But **every structural
change here is deferred** - `abi.md`, *The ABI* - and under flecs' own defer not one invocation
arrives early. `tests/phase5.c` holds that on every build: an attach made in a phase reaches its
observer under that phase's barrier count, and a chain of three takes three barriers.

So Bridge-ASM700 uses flecs' observers as they are. Nothing to arrange, and no change detection to write:
an observer runs where the world is already consistent, and never inside somebody else's work.

**This is how an event crosses.** A native module with a fired signal spawns an entity carrying an event
component; the change applies at the barrier and the subscriber's observer runs there. Same latency
a dedicated ring would have given, since there is a barrier after every phase. And the entity
ceiling already budgets it, so there is no second size to pick and no overflow policy to invent.

**A module never asks another module for anything.** It writes, and whoever subscribed to that
shape acts. A system subscribes to a shape; an observer subscribes to a change.
