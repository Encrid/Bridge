# The data model

What was weighed here and refused is `data_refused.md`, *What was refused: The data model*.

What a component is and when the world changes. How a module reaches one is `query.md` and
`reference.md`; what survives a reload is `lifetime.md`. Where the bytes live is `memory.md`; who
keeps the rows, and how, is `storage.md`.

It is an ECS: components stored by type, entities as ids, systems declared by modules. Storage is
Bridge-ASM700's own, and it is **an implementation detail, not the contract**. No module calls it,
on either backend: a module reads what a barrier wrote for it and asks for change through the
eight entries of `abi.md`, *The ABI*.

## Components

A component is a C type, declared by its text. Its size and alignment come from the type; which
types the schema can express, and how two modules are held to the same component layout, is `schema.md`.

Its name is namespaced - `game:Position` - and who may name it is `grants.md`, *Whose component it
is*. No version in the name: the fingerprint in `schema.md` detects a component layout change, so a version
would be a second answer to the same question.

`string` and `list` are not components. See `strings.md`, *Variable length*.

## Entities

An id: an index in the low 32 bits and a generation in the high 32, in a `u64`. An id outlives the
row behind it; a row does not outlive a barrier. An index whose entity was destroyed is handed out
again under another generation, so the old id names nothing - `reference.md`, *Reference
following*.

## Access mode

Query terms carry it, and it is one of two: **read** or **write**. It is a **scheduling input and
nothing else** - Bridge-ASM700 uses it to know which modules may run at once, and storage is never
told.

It is not enforced, which is how every ECS works. A module that writes a term it declared read is
a bug in that module, no different from one writing nonsense into a term it owns. The second is
indistinguishable from corruption anyway, so policing the first would narrow the blast radius
without closing it. wasm has no read-only memory import in any case.

What is enforced is the boundary that matters: a module reaches only the memories it imports, so
it cannot touch the host, another module's memory, or anything it was not given.

## Structural change

Deferred, onto the queue of the module that asked, and applied at the barrier - `threads.md`,
*Threads*, for why the queue is the module's own. There is one
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

A system runs every pass. An **observer** runs when what its terms describe changes for some
entity. There are three events, `ON_ENTER`, `ON_LEAVE` and `ON_ATTACH`, and an observer asks for
some of them.

It runs **at the barrier where the change applied**, which is why it takes no phase. Every
structural change is deferred - `abi.md`, *The ABI* - so there is no change to hear of anywhere
else. `tests/stage5` holds that on every build: an attach made in a phase reaches its observer
under that phase's barrier count, and a chain of three takes three barriers.

**It is told what the barrier made of an entity, and not each thing that was asked of it.** A
barrier folds an entity's changes into what it will carry before it places it - `storage.md`, *A
barrier is worked an entity at a time* - and an observer is owed the difference between what the
entity carried before that barrier and what it carries after:

    it did not match, and now does      enter
    it matched, and no longer does,     leave
    or was destroyed
    it matches now, and an attach of    attach, with the enter if it has only now come to match
    one of its terms is still on it

**Enter and leave are about the observer's terms, and not about what was done to the entity.** An
observer of "A and not B" hears an entity that carries A enter when B is detached from it, and
leave when B is attached. **Attach is about the entry `attach`, and not about a value.** A system
that writes a component in place, through its column, raises nothing: storage is not told of a
write - *Access mode* - so there is nothing to raise it from. A module that wants a new value
heard attaches it.

So an observer of two terms hears an entity when it comes to carry both, by whichever attach
completed the pair. An entity made and destroyed inside one phase, or given a component and
stripped of it, is not heard at all: nothing of it is there to read. One that loses a component
and is given it back in the same phase is heard as an attach, since it carries a new value. One record
an entity at a barrier, with the events that hold. A detach of what is not there, and any change
to an id that is dead, is nothing. `tests/storage`, case `changes`, says each of these out loud.

**Every event is about something the observer can read**, which is what running after the
republish is for - `schedule.md`, *An observer runs at the barrier, one hop per barrier*.

**This is how a signal crosses.** A native module with something to tell spawns an entity that
carries a component for it; the change applies at the barrier and the subscriber's observer runs
there. Same latency a dedicated ring would have given, since there is a barrier after every phase.
And the entity ceiling already budgets it, so there is no second size to pick and no overflow policy
to invent.

**A module never asks another module for anything.** It writes, and whoever subscribed to that
shape acts. A system subscribes to a shape; an observer subscribes to a change.
