# Lifetime

Storage is flecs' and outlives the element that declared a component, so a reloaded element finds its
components still there - which the original Bridge does not do. What happens when the layout has
changed is *When the layout changed*, below.

**Reload has one mode and it keeps the components**, which is the whole point of it. There is no
drop-and-restart variant, because `bridge_unload` followed by `bridge_load` already is one and its
verb says what it costs - `lifecycle.md`, *The runtime lifecycle*.

That is also what idTech3 does. `VM_Restart` exists to *"reload the data, but leave everything else
in place... without changing memory allocation"* (`vm.c:349`) - the allocation survives, which is
this mode and not a reset. Where in-place is impossible it does not invent a third thing: *"DLL's
can't be restarted in place"* and then `VM_Free` followed by `VM_Create` (`:363`).

Bridge-ASM700 knows which components an element declared exclusively, because `declare` is idempotent and
records its callers - a set operation when it matters and nothing per frame. An element that corrupted
a *shared* component is not cleaned up by any of this.

## When the layout changed

The fingerprint catches it - `schema.md`, *Fingerprints* - and what happens next depends on who else
holds the component.

**Shared: refused, always.** If two elements declare `game:Position` and one is rebuilt with a
different `Position`, the other is still running with the old struct compiled in, and a column can
only have one layout. Dropping takes the other element's data; migrating leaves it reading new bytes
with old code. Nothing reconciles it, so this is forced rather than chosen.

**Exclusive: refused as well, and the refusal cannot be waived.** Dropping would harm only this
element, so it looks viable - but **Bridge-ASM700 never discards data on a call that means "keep
running"**, and a flag to switch that off would be a flag whose job is disabling a check. The
refusal names the component and both fingerprints, so the author knows what they changed.

An author who accepts the loss unloads and loads. Same outcome, through a verb that already means
the data goes - which is where the consent belongs, rather than in a boolean nobody can read at a
call site.

**Migration is not Bridge-ASM700's.** It cannot be: a fingerprint is a hash, so it detects difference and
cannot describe it, and a converter the element supplied would have to read a layout its new build
does not contain.

The ECS already offers migration in its own terms. **Declare a new name**: run a system that reads
`game:Position` and writes `game:Position2`, then stop declaring the old one. Two components, both
valid, no layout changing under anyone. So the rule is **change the bytes, change the name** - a
layout change to an existing name cannot work, and a new name is free.

Quake avoided this rather than answering it: `pr_edict_size` comes from the loaded progs
(`pr_edict.c:1021`), so reloading re-strides every edict - and progs is only loaded at
`SV_SpawnServer`, where edicts are fresh. Reload meant level restart.
