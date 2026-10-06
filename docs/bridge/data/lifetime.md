# Lifetime

Storage is the bridge's and outlives the module that declared a component, so a reloaded module finds its
components still there - which the original Bridge does not do. What happens when a component layout
has changed is *When a component layout changed*, below.

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

**A strip never fails, and never costs an entity.** It is worked a table at a time, and what it
needs is never more than what is there: afterwards there are no more tables than before, and no
more bytes. For each table that carries the component:

    nothing else on its entities     they are left with nothing, in no table, and it is released
    no table of what is left         it becomes that table where it stands: the column goes,
                                     the ones after it close up, and no row moves
    a table of what is left,         its rows join that one, and it is released
    with room for them
    one without room, and the        it becomes a second table of that set, where it stands
    column region cannot give it

The last is the one case with a trace, **and it is said**: one record names the component
stripped, how many rows stay apart, and `[budget] columns`, which had no room to join them. It is
a sign that budget is nearly spent as much as it is a fact about tables. `tests/storage.c`, case
`stripfull`, reads it. Two tables then hold the same archetype until either empties;
a module sees a span for each, which is no more than it could ever rely on - `module.md`, *What
no module may do*. An entity given that set afterwards goes to the one that was there first, and
an entity already in either stays where it is for as long as its archetype does not change.

Refused: leaving a second table without saying so, which is what this did: a deployment saw
more tables than archetypes and no line told it why.

Refused: destroying what cannot be placed, which is what this first did. `strip` says the entity
stays, and a reload that stops declaring a component is a strip too, on a call that means "keep
running". Refused: never joining, and leaving every stripped table where it stands, which is one
path and no row moved: it doubles the tables and spans of every set that already had one, for as
long as their entities live. The condition that brings it back is the join costing more than the
second table does. Refused: moving every block of the column region so that a join always fits.
The condition that brings that back is second tables seen to pile up in a deployment.

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

## When a component layout changed

The fingerprint catches it - `schema.md`, *Fingerprints* - and what happens next depends on who else
holds the component.

**Shared: refused, and the refusal names who else declares it.** If two modules declare
`game:Position` and one is rebuilt with a different `Position`, the other is still running with
the old struct compiled in, and a column can only have one component layout. Dropping takes the
other module's data; migrating leaves it reading new bytes with old code.

**It is a choice, and a host has a way through it with the calls it has.** Unload every other
module that declares the type: the component stays, since one module still declares it. Reload
that one, which is now the only declarer, so its rows are moved, as the next paragraph has it. Then load
the others, built against the new struct. The component's rows are kept throughout; what is lost
is whatever else the unloaded modules held, as at any unload. The refusal lists the modules to
unload, so nobody has to find them.

Refused: one call that reloads several modules as one and moves the rows once all of them present
the new struct. It keeps every declarer's own state, at the price of a new call at the host seam
that swaps several modules together and puts them all back if one fails. The condition that
brings it back is a deployment that must change a shared struct and keep what each declarer
holds. This paragraph called the refusal "forced rather than chosen", and it was neither weighed
nor given a condition.

**Exclusive: its rows are moved to the new component layout, field by field, by name.**
Bridge-ASM700 keeps the fields of every type, and a `declare` hands it the new descriptor, so it has
both. That is decided and not yet built: it is `PLAN.md`, stage 11, since moving a type's rows to
another size is done once for every storage layout. **Until then a changed component layout is
refused**, naming the type and both fingerprints, and an author who accepts the loss unloads and
loads.

What each change will do:

    a field added                its bytes are zero, and the module's `reload`, which runs next,
                                 is where it is given a value
    fields in another order      each goes to its new offset
    an array made longer         what it held is kept, and the rest is zero
    a field removed              it is gone from every row, and a record names it and how many
                                 rows held it: once the loss is stated, below
    an array made shorter        the first of it is kept, and a record says how much went: once
                                 the loss is stated
    a field renamed              is a field removed and a field added, since two descriptors
                                 cannot say otherwise: the old values go and the new field is
                                 zero, and the record shows both: once the loss is stated
    a field of another type      the reload is refused, naming the field: bytes of one type are
                                 not a value of another
    a struct a field names,      the same rules, one level down
    changed
    an enum whose constants      refused if a constant now has another number, since every
    changed                      stored value would silently mean something else; a constant
                                 added is nothing

**A reload that would drop values is refused until the loss is stated.** The first one is
refused, and the refusal names every field that would go and how many rows hold it. A reload
that states it accepts the loss then moves the rows, with the same record. A change that drops
nothing - a field added, fields in another order, an array made longer - needs no statement.
It is for the rename: an author who renamed a field meant to drop nothing, and is told before
anything is gone and not by a record after. How the acceptance is stated is this stage's to
decide when it is built - `PLAN.md`, stage 11 - and it is a stated value, not a flag left off.

Refused: moving the rows at once and saying what went, which is what was first decided here. It
fails nobody who removed a field, and for a rename it loses every value of the field on a call
that means "keep running". The condition that brings it back is a rename that can be told from
a removal. Refused: refusing every such change, so that a struct which loses a field is had only
by unloading, which loses every row. The condition that brings it back is moving rows proving
unsound.

**A removal is the author's, and that is why it migrates.** This document said that
Bridge-ASM700 never discards data on a call that means "keep running", and refused every change
for it. The rule stands for what Bridge-ASM700 would drop on its own account. A field the new
struct no longer has was dropped by whoever edited the struct; refusing the reload does not keep
the field, it keeps the author from running the code they wrote. What they are owed is to be told
exactly what went, which the record does.

Refused: refusing every reload that would lose a byte. It keeps one promise literal and makes
tidying a struct, or renaming a field, cost the module's state. The condition that brings it back
is a field lost to a rename that nobody saw in time. Refused: a flag to waive the refusal, which
is a flag whose job is disabling a check. Refused: a way for a module to state "this field was
that one", which is one more thing in the ABI for a case two reloads or a new name already cover.
The condition is renames being common enough that losing their values is a daily cost.

**A converter the module supplies is still not possible**: it would have to read a component
layout its new build does not contain. For a change no rule above covers there is the ECS's own
way. **Declare a new name**: run a system that reads `game:Position` and writes `game:Position2`,
then stop declaring the old one. Two components, both valid, no component layout changing under
anyone.

Quake avoided this rather than answering it: `pr_edict_size` comes from the loaded progs
(`pr_edict.c:1021`), so reloading re-strides every edict - and progs is only loaded at
`SV_SpawnServer`, where edicts are fresh. Reload meant level restart.
