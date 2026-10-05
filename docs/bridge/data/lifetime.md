# Lifetime

Storage is the bridge's and outlives the module that declared a component, so a reloaded module finds its
components still there - which the original Bridge does not do. What happens when the layout has
changed is *When the layout changed*, below.

**Reload has one mode and it keeps the components**, which is the whole point of it. There is no
drop-and-restart variant, because `bridge_unload` followed by `bridge_load` already is one and its
verb says what it costs - `lifecycle.md`, *The runtime lifecycle*.

That is also what idTech3 does. `VM_Restart` exists to *"reload the data, but leave everything else
in place... without changing memory allocation"* (`vm.c:349`) - the allocation survives, which is
this mode and not a reset. Where in-place is impossible it does not invent a third thing: *"DLL's
can't be restarted in place"* and then `VM_Free` followed by `VM_Create` (`:363`).

Bridge-ASM700 knows which components a module declared exclusively, because `declare` is idempotent and
records its callers - a set operation when it matters and nothing per frame. A module that corrupted
a *shared* component is not cleaned up by any of this.

## What unload leaves behind

Unload drops a module's exclusive components and keeps the shared ones - `data.md`, *Structural
change*. What happens to the **entities** that carried a dropped component is the author's to say,
per component, at `declare`:

    strip     the component is removed. the entity stays, even with nothing left on it
    sweep     the component is removed. an entity left with no component is destroyed
    delete    every entity that has the component is destroyed, with whatever else is on it

**It is a lifetime question, and no single answer fits.** An entity has no owner - several modules
put components on one - so it cannot die with whoever spawned it. Its id sits as a plain `u64` in
memories Bridge-ASM700 cannot trace, so it cannot be collected when unreachable either. What is left
is that the author knows what the component *is*: an attribute of something that exists anyway, or
the reason the entity exists at all.

That is flecs' model, and its reason. Deleting a component there applies a cleanup policy,
`EcsRemove` by default or `EcsDelete`, because only the component's author knows which is meant.
`sweep` is ours: flecs, EnTT and Bevy all leave an emptied entity alive, and can afford to, since an
application can enumerate every entity and find its orphans. **No module here can**: a query needs
terms, so an entity with no component and no remembered id could never be destroyed by anyone, and
it would count against the entity ceiling for the life of the process.

**A strip moves each entity to the table of what it has left**, one entity at a time, and that
table may have to be made while the one it is leaving still stands. Where `[budget] tables` or
`[budget] columns` cannot give it, the entity is destroyed and a record says so: the component is
going, so the entity cannot stay as it was, which is what running out means anywhere else -
`budgets.md`, *What running out does*.

**The policy applied is the unloading module's own**, and only when it is the last to declare the
component. Two declarers need not agree, because the first to leave drops nothing.

`delete` takes other modules' components on those entities with it. That is no new power - any
module may already `destroy` an entity it only partly populates - but here it is stated up front.

A stored id to an entity that `sweep` or `delete` destroyed is refused as stale, by the generation
check - `reference.md`, *Reference following*. A module that keeps an entity as a bare handle, with
no component of its own on it, loses it to another module's `sweep`: an entity that must outlive
somebody else's unload carries a component of its holder's.

Refused: one rule for every component, which is right for an event and wrong for a handle, or the
reverse. Refused: destroying what a module spawned, which needs an owner per entity. Refused: a
fourth value that makes the unload itself fail while entities still carry the component - a new
failure path nothing has asked for. The condition that brings it back is data that must not be lost
by an unload somebody else requested.

## When the layout changed

The fingerprint catches it - `schema.md`, *Fingerprints* - and what happens next depends on who else
holds the component.

**Shared: refused, always.** If two modules declare `game:Position` and one is rebuilt with a
different `Position`, the other is still running with the old struct compiled in, and a column can
only have one layout. Dropping takes the other module's data; migrating leaves it reading new bytes
with old code. Nothing reconciles it, so this is forced rather than chosen.

**Exclusive: refused as well, and the refusal cannot be waived.** Dropping would harm only this
module, so it looks viable - but **Bridge-ASM700 never discards data on a call that means "keep
running"**, and a flag to switch that off would be a flag whose job is disabling a check. The
refusal names the component and both fingerprints, so the author knows what they changed.

An author who accepts the loss unloads and loads. Same outcome, through a verb that already means
the data goes - which is where the consent belongs, rather than in a boolean nobody can read at a
call site.

**Migration is not Bridge-ASM700's.** It cannot be: a fingerprint is a hash, so it detects difference and
cannot describe it, and a converter the module supplied would have to read a layout its new build
does not contain.

The ECS already offers migration in its own terms. **Declare a new name**: run a system that reads
`game:Position` and writes `game:Position2`, then stop declaring the old one. Two components, both
valid, no layout changing under anyone. So the rule is **change the bytes, change the name** - a
layout change to an existing name cannot work, and a new name is free.

Quake avoided this rather than answering it: `pr_edict_size` comes from the loaded progs
(`pr_edict.c:1021`), so reloading re-strides every edict - and progs is only loaded at
`SV_SpawnServer`, where edicts are fresh. Reload meant level restart.
