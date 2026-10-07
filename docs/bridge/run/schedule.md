# The schedule

What was weighed here and refused is `schedule_refused.md`, *What was refused: The schedule*.

What Bridge-ASM700 does between a module's systems and observers: when they run, and what is visible
while they do. What bounds a run is `limits.md`; what happens when one fails is `failure.md`; which
systems run at once is `threads.md`. The data they run over is `data.md`; what a module can ask
for is `abi.md`.

Bridge-ASM700 owns this because no host could do it differently - `split.md`, *The split*.

## Phases

A phase runs, then a barrier. The host asks for them one at a time and decides the order -
`loop.md`, *The host owns the loop, and the order within it*.

    a phase                its systems run
    a barrier              queued changes applied, storage restructured, spans
                           republished, then the observers of what changed

**One of each is a working deployment**, and that is the whole mechanical requirement. Nothing has
an immediate form that would need a phase of its own - `abi.md`, *The ABI* - so no second phase
is forced.

### Bridge-ASM700 ships no phase names

Not one, and not a default set. **A phase name is a name somebody has to invent**, and inventing one
here would turn Bridge-ASM700's guess about the shape of a frame into something every module references.
That is the last clause of `no_builtins.md`, *No builtins*, and it fails that rule's universality test: a
one-run tool and a game do not want the same phases.

So a deployment declares its own, and `bridge_create` refuses a config with none, exactly as it
refuses one with a phase named twice.

### A module's phase names are mapped by the deployment

**A module names its phases in its own words, and the deployment says which of its phases each
one is.** A module's code has a phase name in it, written by somebody who never saw the
deployment. With no names shipped, two authors share none: one writes `update` and another
`tick`. A deployment that declared both would have two phases, run one after the other with a
barrier between, which neither author meant. So a module's config carries a table, from a name
the module uses to a phase the deployment declared:

    [module."mods:physics".phases]
    tick = "update"

A name that is not in the table is looked for among the deployment's phases as it is written, so a
module that already says `update` needs no entry. A table that maps a name twice, or to a phase the
deployment did not declare, refuses the load, by the name. `tests/host`, cases
`module_config.phases-unmapped-refused`, `phase_name.from-mapped-loads` and
`phase_name.to-undeclared-refused`, are one module that says `tick`.

It is the deployment that resolves it, as it resolves an order between two systems and a grant
between two namespaces: the only party that has seen both sides.

**flecs does ship eight, and where it ships them is the point.** `EcsOnLoad` through `EcsOnStore`
are bootstrapped by the **pipeline addon** (`src/addons/pipeline/pipeline.c:968`), not by the core,
and `ecs_pipeline_init` lets a world define its own instead. So they are an optional default made of
replaceable data, not names the library imposes. Here nothing ships one at all: not the library,
and not a host, which hands over the phases its deployment stated. Whether a set of names ever
becomes something deployments share is the question `no_builtins.md`, *No builtins*, puts for a
clock, and it is asked where the first real deployment is built: `plan.md`, stage 19.

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
declared term, and even `get` by id is indexed by the types *this* module declared.

The three cases for it each live somewhere else. A system driving something that is not thread-safe
is `pinned`, and every pinned module is on one thread already - `pinned.md`, *Threads*. A
system that wants the whole world to itself has one, for the whole of a phase - *What a phase shows
you*. A system that walks everything must declare everything, so it conflicts with every writer and
the conflict graph serialises it anyway - `threads.md`, *Threads*.

What is left is a flag somebody sets hoping to fix a race, where the cause is an undeclared write
that is wrong whether or not anything ran beside it. **The condition that brings it back** is a
module that can share an external resource without being pinnable; nothing like that exists
today.

Rows move when an entity is placed in another table, or into the place of one that left, so
**nothing moves while any phase is in flight.**
That is the whole reason for the shape. Every phase defers its structural change, so Bridge-ASM700 has one
safe moment to restructure, and it is the same moment it republishes the spans - which is what lets
a span be a snapshot. **Every phase, without exception**: `data.md`, *Structural change*.

**A barrier after every phase, not once per frame.** So a spawn in `update` is visible in `present`,
the same frame rather than the next one. It costs one barrier a phase, and a barrier costs what
changed - `query.md`, *The interface is a query* - and, beside that, a visit to each module to
top up its ids, count its failures and drain its log: about a hundred nanoseconds in all with
one module and nothing to do, measured. `stage10.md`, *What it costs*. It is what makes
ordering worth having.

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
modules and only the deployment knows both are there.

### Order within a phase is never implicit

If two systems in the same phase conflict and nothing orders them, **Bridge-ASM700 refuses the
load that introduces the conflict** and names both. Never
declaration order or load order: nothing is decided by which module happened to be found first.

Two systems conflict in one of two ways:

    through a column    one writes a component the other reads or writes: their terms say so
    through a change    both attach or detach the same component: their changes say so,
                        the list each stated beside its terms - `abi.md`, *The ABI*

**A change is in the graph because a barrier applies changes in the order the systems ran.** Two
modules that attach one component to one entity in one phase leave the value of whichever ran last,
so which ran last has to be something the deployment stated. `tests/host`, cases
`order-changes-ordered-loaded-first` and `order-changes-ordered-loaded-last`, load the same two
modules in both orders and read the same value. A change does not conflict with a term: a change is
applied at the barrier, after every system of the phase has read what was there before it.

**Every attach and every detach is stated, with no exception.** So two modules that send the same
signal in one phase - each attaching the same component to entities only it holds - are refused
until the deployment orders them, though they could not have collided. That is the price of one
rule.

There is no moment called start to refuse at, since the host loads modules one at a time -
`lifecycle.md`, *The runtime lifecycle*. Conflicts are already computed for
parallelism, so detecting this costs nothing.

**The deployment resolves it, not the modules.** The only party that knows both are present is the
one that chose to deploy them. Either move one into a new phase, or state the pair:

    phases = ["setup", "physics", "present"]

    [order]
    before = [["game:actors:movement", "mods:physics:collision"]]

**A system is named by its module and then itself**: the module's identity without its version, and
the name it registered. Two modules may both have a `movement`, so the system's own name is not
enough; and the version is left out so that upgrading a module does not edit every deployment that
orders it. It follows that two modules differing only in version cannot be loaded together, since
nothing could tell their systems apart.

The cost: a deployment combining two conflicting modules has work to do. That work was previously
done by luck, so making it explicit is the point rather than a regression.

**A module orders its own systems by registering them.** The refusal above exists because two
parties who never met cannot have agreed on an order. Two conflicting systems of one module have one
author, who wrote both registrations in some order and meant it - so that order stands, with nothing
for a deployment to state. `[order]` may still say otherwise, since it can name any system.

An `[order]` pair is not required to name anything loaded: a deployment orders modules it may or
may not load. Pairs that contradict each other are refused at create, and a module whose own order
contradicts them is refused at load.

## What a phase shows you

**A spawn is visible at the next barrier.** `spawn` answers a usable id at once, since the id is
Bridge-ASM700's to mint, but the row does not exist and no query counts it until then. Deletion mirrors
it: a destroyed entity stays readable for the rest of the phase that destroyed it.

So **every system sees one world for the whole of a phase**, which is what lets parallel systems
agree. It is the same fact as *nothing moves while any phase is in flight*, seen from the other
side.

## What a pass depends on

Storage, the phase the host asked for, and **what native modules wrote**. Nothing else crosses in:
everything a deployment learns from outside reaches the rest as a column some native module declared
written, applied at a barrier.

That is one place, because of three decisions taken elsewhere for their own reasons. **No module
can call another**, so no value ever arrives as a return - `abi.md`, *The ABI*. **A native
module's code runs inside a phase** and a foreign-thread callback touches no storage at all -
`native.md`, *The invariant*. And **two writers of one component are never in flight together**,
so the order the rest happen to run in cannot change the result - the conflict graph exists for
parallelism and hands this over for nothing.

So a pass is reproducible from its storage and that one column set. Entity ids and arena offsets are
Bridge-ASM700's to mint in order, iteration order follows from the same history, and a work limit is
deterministic by period - `limits.md`, *What bounds a run*.

**The one thing that is not reproducible is a timeout firing.** The same `init` may finish on a
quiet machine and be killed on a busy one. That is the cost of wall time, and it is why a work limit and a
timeout are not interchangeable.

**That is a claim, and it has a case.** `tests/storage` runs four modules through eight million
changes on four threads and again on one, and what they say of their own account has to be the
same file, word for word, on every run. `[checks] validate` hashes every column at every barrier
- `validation.md`, *The checksum pass, when asked* - and with it the same is held barrier by
barrier: `plan.md`, stage 17.

It is stated here because an input path that bypassed a column would quietly make a pass depend
on something nobody can see, and nothing later could recover that.

**Recording a run and playing it back is what this is for, and it is a host's to build, all of
it.** Nothing is missing from Bridge-ASM700 for it. A host loads native modules of its own, and a
native module reads the columns of the components it declares through real pointers, and writes
them: so a module that copies what the input modules wrote, and another that writes it back in
their place, are two modules like any other - `no_builtins.md`, *No builtins*: a new capability
is a new module. The journal they feed is a file, and a file is the host's - `split.md`, *The
split*. It may not drop a record, which is why it is not the log's path - `log.md`, *The log*.

What is Bridge-ASM700's is what makes a recording mean anything: the rule above, the case that
holds it, and the checksums a played-back run is judged by.