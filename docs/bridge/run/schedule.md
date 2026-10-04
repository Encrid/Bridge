# The schedule

What BridgeC does between an element's systems and observers: when they run, and what is visible
while they do. What bounds a run is `limits.md`; what happens when one fails is `failure.md`; which
systems run at once is `threads.md`. The data they run over is `data.md`; what an element can ask
for is `abi.md`.

BridgeC owns this because no host could do it differently - `split.md`, *The split*.

## Phases

A phase runs, then a barrier. The host asks for them one at a time and decides the order -
`loop.md`, *The host owns the loop, and the order within it*.

    a phase                its systems run
    a barrier              queued changes applied, storage restructured, spans
                           republished, then the observers of what changed

**One of each is a working deployment**, and that is the whole mechanical requirement. Nothing has
an immediate form that would need a phase of its own - `abi.md`, *The ABI* - so no second phase
is forced.

### BridgeC ships no phase names

Not one, and not a default set. **A phase name is a name somebody has to invent**, and inventing one
here would turn BridgeC's guess about the shape of a frame into something every element references.
That is the last clause of `no-builtins.md`, *No builtins*, and it fails that rule's universality test: a
one-run tool and a game do not want the same phases.

So a deployment declares its own, and `bridge_create` refuses a config with none, exactly as it
refuses one with a phase named twice. The reference host has no default to use either: `ideatek.md`,
*The pass is config's order*.

**flecs does ship eight, and where it ships them is the point.** `EcsOnLoad` through `EcsOnStore`
are bootstrapped by the **pipeline addon** (`src/addons/pipeline/pipeline.c:968`), not by the core,
and `ecs_pipeline_init` lets a world define its own instead. So they are an optional default made of
replaceable data, not names the library imposes. A default belongs there here too: beside the
library, as something a deployment chooses - the answer `no-builtins.md`, *No builtins*, gives for a
clock.

flecs' phases carry `DependsOn` relationships, and `:942` records the care that needs -
*"EcsOnUpdate doesn't have a direct DependsOn relationship on EcsPreUpdate, which ensures that when
the EcsPreUpdate phase is disabled, EcsOnUpdate still runs."* Ours carry no relation at all, because
the host orders them. A phase nobody calls does not run and takes nothing with it.

An illustration rather than a default, and the shape a game usually lands on:

    setup      spawning and loading, before anything reads
    update     the bulk of game logic
    present    pushing state outward

`present` is why this is an illustration. It is **frame-shaped** - a one-run program has nothing to
present to - and a set that is wrong for a whole class of deployment is a poor default.

### A phase is a name and nothing else

Not an order, because the host orders them. And **not an exclusivity flag**: nobody could name a
case for one.

flecs has the equivalent and says what it is for: `ecs_system_desc_t.immediate`, *"the system will
have access to the actual world. Cannot be true at the same time as multi_threaded"* -
`flecs.h:14371`. So running alone is the **consequence** of wanting undeclared access to the real
world, not the goal. **Here there is no way to ask for the actual world**: every read goes through a
declared term, and even `get` by id is indexed by the terms *this* element declared.

The three cases for it each live somewhere else. A system driving something that is not thread-safe
is `pinned`, and every pinned element is on one thread already - `plugin-threads.md`, *Threads*. A
system that wants the whole world to itself has one, for the whole of a phase - *What a phase shows
you*. A system that walks everything must declare everything, so it conflicts with every writer and
the conflict graph serialises it anyway - `threads.md`, *Threads*.

What is left is a flag somebody sets hoping to fix a race, where the cause is an undeclared write
that is wrong whether or not anything ran beside it. **The condition that brings it back** is an
element kind that can share an external resource without being pinnable; nothing like that exists
today.

Rows move when storage is compacted or migrated, so **nothing moves while any phase is in flight.**
That is the whole reason for the shape. Every phase defers its structural change, so BridgeC has one
safe moment to restructure, and it is the same moment it republishes the spans - which is what lets
a span be a snapshot. **Every phase, without exception**: `data.md`, *Structural change*.

**A barrier after every phase, not once per frame.** So a spawn in `update` is visible in `present`,
the same frame rather than the next one. It costs one republish per phase, each O(matched tables),
and it is what makes ordering worth having.

### An observer runs at the barrier, one hop per barrier

An observer subscribes to a change rather than to a phase - `data.md`, *Observers*. So it runs at
the barrier that applied the change, after the restructure and the republish, reading the world the
change made.

**What an observer queues applies at the next barrier, not this one.** One hop per barrier, never a
fixpoint. A chain - A adds a component, B reacts by adding another, C reacts to that - settles in
three barriers, which with a barrier after every phase is three phases. The alternative is
restructuring until nothing changes, and that loop has no bound a deployment could reason about: one
accidental cycle and the barrier never ends. Deferring the hop turns a cycle into a trickle of work
rather than a hang, and **the profile shows it**: both observers in a cycle keep a call count that
never falls to zero, `profile.md`, *The profile*. A cycle is legal, so nothing refuses it; it is
a thing to be able to see.

**The conflict graph covers them.** Two observers that conflict with nothing ordering them refuse at
load, exactly as two systems in one phase do, and for the same reason: the parties are different
elements and only the deployment knows both are there.

### Order within a phase is never implicit

If two systems in the same phase conflict - one writes what the other reads or writes - and nothing
orders them, **BridgeC refuses to start** and names both. Not declaration order, not load order,
nothing decided by which element happened to be found first. Conflicts are already computed for
parallelism, so detecting this costs nothing.

**The deployment resolves it, not the elements.** The only party that knows both are present is the
one that chose to deploy them. Either move one into a new phase, or state the pair:

    phases = ["setup", "physics", "present"]

    [order]
    before = [["mymod:movement", "othermod:collision"]]

The cost: a deployment combining two conflicting elements has work to do. That work was previously
done by luck, so making it explicit is the point rather than a regression.

## What a phase shows you

**A spawn is visible at the next barrier.** `spawn` answers a usable id at once, since the id is
BridgeC's to mint, but the row does not exist and no query counts it until then. Deletion mirrors
it: a destroyed entity stays readable for the rest of the phase that destroyed it.

So **every system sees one world for the whole of a phase**, which is what lets parallel systems
agree. It is the same fact as *nothing moves while any phase is in flight*, seen from the other
side.

## What a pass depends on

Storage, the phase the host asked for, and **what plugins wrote**. Nothing else crosses in:
everything a deployment learns from outside reaches a module as a column some plugin declared `out`,
applied at a barrier.

That is one place, because of three decisions taken elsewhere for their own reasons. **No element
can call another**, so no value ever arrives as a return - `abi.md`, *The ABI*. **A plugin's
code runs inside a phase** and a foreign-thread callback touches no storage at all -
`plugin-abi.md`, *The invariant*. And **two writers of one component are never in flight together**,
so the order the rest happen to run in cannot change the result - the conflict graph exists for
parallelism and hands this over for nothing.

So a pass is reproducible from its storage and that one column set. Entity ids and arena offsets are
BridgeC's to mint in order, iteration order follows from the same history, and fuel is deterministic
by period - `limits.md`, *What bounds a run*.

**The one thing that is not reproducible is a timeout firing.** The same `init` may finish on a
quiet machine and be killed on a busy one. That is the cost of wall time, and it is why fuel and a
timeout are not interchangeable.

Record and replay is what this is for, and it is not built - `PLAN.md` phase 8 is where it could
land. It is stated here because an input path that bypassed a column would quietly make a pass
depend on something nobody can see, and no later phase could recover that.
