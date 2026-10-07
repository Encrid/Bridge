# Lifetime

What was weighed here and refused is `lifetime_refused.md`, *What was refused: Lifetime*.

Storage is the deck's and outlives the module that declared a component, so a reloaded module finds its
components still there - which `legacy/rust-components` does not do. What happens when a component layout
has changed is *When a component layout changed*, below.

**Reload has one mode and it keeps the components**, which is the whole point of it. There is no
drop-and-restart variant, because `bridge_unload` followed by `bridge_load` already is one and its
verb says what it costs - `lifecycle.md`, *The runtime lifecycle*.

That is also what idTech3 does. `VM_Restart` exists to *"reload the data, but leave everything else
in place... without changing memory allocation"* (`vm.c:349`) - the allocation survives, which is
this mode and not a reset. Where in-place is impossible it does not invent a third thing: *"DLL's
can't be restarted in place"* and then `VM_Free` followed by `VM_Create` (`:363`).

Bridge knows which components a module declared exclusively, because `declare` is idempotent and
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
memories Bridge cannot trace, so it cannot be collected when unreachable either. What is left
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
a sign that budget is nearly spent as much as it is a fact about tables. `tests/storage`, case
`storage-stripfull`, reads it. Two tables then hold the same archetype until either empties;
a module sees a span for each, which is no more than it could ever rely on - `module.md`, *What
no module may do*. An entity given that set afterwards goes to the one that was there first, and
an entity already in either stays where it is for as long as its archetype does not change.

**The policy applied is the unloading module's own**, and only when it is the last to declare the
component. Two declarers need not agree, because the first to leave drops nothing.

`delete` takes other modules' components on those entities with it. That is no new power - any
module may already `destroy` an entity it only partly populates - but here it is stated up front.

A stored id to an entity that `sweep` or `delete` destroyed is refused as stale, by the generation
check - `reference.md`, *Reference following*. A module that keeps an entity as a bare handle, with
no component of its own on it, loses it to another module's `sweep`: an entity that must outlive
somebody else's unload carries a component of its holder's.

## When a component is moved

**A component is moved to another storage layout by a reload whose `declare` states another** -
`storage.md`, *Storage layouts*. It is the one module that declares it that may: every declarer
states the same storage layout, so while another module declares the component the reload is
refused, naming them, and the way through is the one a changed component layout has, below.

**It is all of the rows or none.** The reload makes a type for what the component becomes,
beside the one it is. When the new instance's `reload` has answered, every table and every row
the new type needs is had first, while every value is still where it was; then each entity is
placed once and its value written across; then the type it was is gone. If the room cannot be
had, nothing has moved, the reload is refused saying so, and what follows is what follows any
reload that fails - `lifecycle.md`, *The runtime lifecycle*. So for the length of a reload the
component is two types: it counts twice against `[budget] types`, and against `sparse_sets` or
`direct_indexed` if both are that, and its rows need room where they are going while they are
still where they were. Observers are told nothing, since no entity gains or loses a component.

    from, to                                 what happens to the values
    table, sparse set, direct-indexed,       every entity that carries it keeps its value. As a
    toggle: any to any                       toggle, each is on
    any, to singleton                        no entity carries it: the singleton has no value.
                                             One does: its value is the singleton's. More than
                                             one: refused, naming how many, since nothing says
                                             which
    singleton, to any                        refused: there is no entity to carry it
    toggle, to any other                     a row that is off is carried like the rest, with
                                             its value

**Leaving a toggle, that a row was off is lost, and it is a loss the reload has to accept.**
The reload is refused, naming the component and how many rows are off, unless the component's
name is among the losses it states - *When a component layout changed*, below, for how. The
value is kept, and the module's `reload`, which runs next, is where it detaches what it does
not want.

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

**Exclusive: its rows are moved to the new component layout, field by field, by name.**
Bridge keeps the fields of every type, and a `declare` hands it the new descriptor, so it has
both. It is done as a move to another storage layout is, and by the same code, in whichever
storage layout the component is kept - *When a component is moved*, above: a type is made for
what the component becomes, and it takes every row or none when the reload holds. A component
may change its component layout and its storage layout in one reload.

What each change does:

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
    an enum whose constants      refused if a constant now has another number, or is gone, or
    changed                      if the enum is an integer of another width: every stored value
                                 would silently mean something else, or nothing. A constant
                                 added is nothing
    a component whose own text   it is laid out again all the same, since its field is: what a
    is as it was, with a field   reload makes anew of a type, it makes of every type that has
    of a type that changed       a field of it
    a type another module's      refused, naming that component and that module: its rows
    component has a field of     hold the type as it is
    a struct declared as an      refused: a reload does not change what kind of type one is
    enum, or as a tag

**A reload that would drop values is refused until the loss is stated.** The first one is
refused, and the refusal names every field that would go and how many rows hold it. A reload
that states it accepts the loss then moves the rows, with the same record. A change that drops
nothing - a field added, fields in another order, an array made longer - needs no statement.
It is for the rename: an author who renamed a field meant to drop nothing, and is told before
anything is gone and not by a record after.

**The loss is stated by name, in the module's configuration**: `losses`, with `loss_count` of
them, or `BRIDGE_NONE` for none - `config.md`, *The keys*. A field that goes, or
an array made shorter, is its component's name and its own, `game:Position.z`; one level down,
`game:Body.at.z`; and the rows of a toggle that are off, for a component that leaves the toggle,
are the component's name alone. A loss that is not there is refused, naming it and how many
rows hold it. A name that is there and that this reload does not lose is refused too, so that a
list is not left where it is from one reload to the next, and a load, which loses nothing,
states none. It is a list and not a yes or no because the rule is for the rename: a yes would
let that through.

**A removal is the author's, and that is why it migrates.** Bridge never discards data
on a call that means "keep running", and that rule is for what Bridge would drop on its own
account. A field the new
struct no longer has was dropped by whoever edited the struct; refusing the reload does not keep
the field, it keeps the author from running the code they wrote. What they are owed is to be told
exactly what went, which the record does.

**A converter the module supplies is still not possible**: it would have to read a component
layout its new build does not contain. For a change no rule above covers there is the ECS's own
way. **Declare a new name**: run a system that reads `game:Position` and writes `game:Position2`,
then stop declaring the old one. Two components, both valid, no component layout changing under
anyone.

Quake avoided this rather than answering it: `pr_edict_size` comes from the loaded progs
(`pr_edict.c:1021`), so reloading re-strides every edict - and progs is only loaded at
`SV_SpawnServer`, where edicts are fresh. Reload meant level restart.
