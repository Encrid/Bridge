# The orchestrator

What BridgeC does between an element's systems and observers: when they run, what is visible while
they do, and what happens when one fails. The data they run over is `data.md`; what an element can
ask for is `element.md`.

BridgeC owns this because no host could do it differently - `DESIGN.md`, *The split*.

## Phases

A phase runs, then a barrier. The host asks for them one at a time and decides the order -
`host-api.md`, *The host owns the loop, and the order within it*.

    a phase                its systems run
    a barrier              queued changes applied, storage restructured, spans
                           republished, then the observers of what changed

**One of each is a working deployment**, and that is the whole mechanical requirement. There is no
immediate form of anything to need a phase of its own - `element.md`, *The ABI* - so no second phase
is forced.

### BridgeC ships no phase names

Not one, and not a default set. **A phase name is a name somebody has to invent**, and inventing one
here would turn BridgeC's guess about the shape of a frame into something every element references.
That is the last clause of `DESIGN.md`, *No builtins*, and it fails that rule's universality test
plainly: a one-run tool and a game do not want the same phases.

So a deployment declares its own and `bridge_create` refuses a config with none, exactly as it
refuses one with a phase named twice. `ideatek.md`, *The pass is config's order*, was already
written on that assumption, so there was never a default for the reference host to use.

**flecs does ship eight, and where it ships them is the point.** `EcsOnLoad` through `EcsOnStore`
are bootstrapped by the **pipeline addon** (`src/addons/pipeline/pipeline.c:968`), not by the core,
and `ecs_pipeline_init` lets a world define its own instead - so they are an optional default made
of replaceable data, not names the library imposes. That is where a default belongs here too: beside
the library, as something a deployment chooses, which is the same answer `DESIGN.md`, *No builtins*,
gives for a clock.

Worth noting what flecs had to do that we do not. Its phases carry `DependsOn` relationships, and
`:942` records the care that needs - *"EcsOnUpdate doesn't have a direct DependsOn relationship on
EcsPreUpdate, which ensures that when the EcsPreUpdate phase is disabled, EcsOnUpdate still runs."*
Ours carry no relation at all, because the host orders them, so a phase nobody calls simply does not
run and takes nothing with it.

An illustration rather than a default, and the shape a game usually lands on:

    setup      spawning and loading, before anything reads
    update     the bulk of game logic
    present    pushing state outward

`present` is why this is an illustration. It is **frame-shaped** - a one-run program has nothing to
present to - and a set that is wrong for a whole class of deployment is a poor thing to default to.

### A phase is a name and nothing else

Not an order, because the host orders them. And **not an exclusivity flag**, which this design
carried until nobody could name a case for it.

flecs has the equivalent and says what it is for: `ecs_system_desc_t.immediate`, *"the system will
have access to the actual world. Cannot be true at the same time as multi_threaded"* -
`flecs.h:14371`. So running alone is the **consequence** of wanting undeclared access to the real
world, not the goal. **Here there is no way to ask for the actual world**: every read goes through a
declared term, and even `get` by id is indexed by the terms *this* element declared.

The three cases it was written for had each moved somewhere else. A system driving something that is
not thread-safe is `pinned`, and every pinned element is on one thread already - `plugin-abi.md`,
*Threads*. A system that wants the whole world to itself has one, for the whole of a phase -
*What a phase shows you*. And a system that walks everything must declare everything, so it conflicts
with every writer and the conflict graph serialises it anyway - `plugin-abi.md`, *Threads*.

What is left is a flag somebody sets hoping to fix a race, where the cause is an undeclared write
that is wrong whether or not anything ran beside it. **The condition that brings it back** is an
element kind that can share an external resource without being pinnable; nothing like that exists
today.

Rows move when storage is compacted or migrated, so **nothing moves while any phase is in flight.**
That is the whole reason for the shape: every phase defers its structural change, so BridgeC has one
safe moment to restructure, and it is the same moment it republishes the spans - which is what lets
a span be a snapshot at all. **Every phase, without exception**: `data.md`, *Structural change*.

**A barrier after every phase, not once per frame.** So a spawn in `update` is visible in `present`,
the same frame rather than the next one. It costs one republish per phase, each O(matched tables),
and it is what makes ordering worth having.

### An observer runs at the barrier, one hop per barrier

An observer subscribes to a change rather than to a phase - `data.md`, *Observers* - so it runs at
the barrier that applied the change, after the restructure and the republish, reading the world the
change made.

**What an observer queues applies at the next barrier, not this one.** One hop per barrier, never a
fixpoint. A chain - A adds a component, B reacts by adding another, C reacts to that - settles in
three barriers, which with a barrier after every phase is three phases. The alternative is
restructuring until nothing changes, and that loop has no bound a deployment could reason about: one
accidental cycle and the barrier never ends. Deferring the hop turns a cycle into a trickle of work
rather than a hang - and it is **the profile that shows it**: both observers in a cycle keep a call
count that never falls to zero, `diagnostics.md`, *The profile*. A cycle is legal, so nothing refuses it;
it is a thing to be able to see.

**The conflict graph covers them.** Two observers that conflict with nothing ordering them refuse at
load, exactly as two systems in one phase do, and for the same reason: the parties are different
elements and only the deployment knows both are there.

### Order within a phase is never implicit

If two systems in the same phase conflict - one writes what the other reads or writes - and nothing
orders them, **BridgeC refuses to start** and names both. Not declaration order, not load order,
nothing decided by which element happened to be found first. Conflicts are already computed for
parallelism, so detecting this costs nothing.

When it fires, **the deployment resolves it, not the elements.** The only party that knows both are
present is the one that chose to deploy them, so the tie-break lives there: either move one into a
new phase, or state the pair.

    phases = ["setup", "physics", "present"]

    [order]
    before = [["mymod:movement", "othermod:collision"]]

The cost is that a deployment combining two conflicting elements has work to do. That work was
previously done by luck, so making it explicit is the point rather than a regression.

## What a phase shows you

**A spawn is visible at the next barrier.** `spawn` answers a usable id at once, since the id is
BridgeC's to mint, but the row does not exist and no query counts it until then. Deletion mirrors it:
a destroyed entity stays readable for the rest of the phase that destroyed it.

So **every system sees one world for the whole of a phase**, which is what lets parallel systems
agree, and it is the same fact from the other side as *nothing moves while any phase is in flight*.

## What a pass depends on

Storage, the phase the host asked for, and **what plugins wrote**. Nothing else crosses in:
everything a deployment learns from outside reaches a module as a column some plugin declared `out`,
applied at a barrier.

That is one place, and it is one place because of three decisions taken elsewhere for their own
reasons. **No element can call another**, so no value ever arrives as a return - `element.md`, *The
ABI*. **A plugin's code runs inside a phase** and a foreign-thread callback touches no storage at all
- `plugin-abi.md`, *The invariant*. And **two writers of one component are never in flight
together**, so the order the rest happen to run in cannot change the result - the conflict graph
exists for parallelism and hands this over for nothing.

So a pass is reproducible from its storage and that one column set. Entity ids and arena offsets are
BridgeC's to mint in order, iteration order follows from the same history, and fuel is deterministic
by period - *What bounds a run*.

**The one thing that is reproducible from nothing is a timeout firing.** The same `init` may finish on
a quiet machine and be killed on a busy one. That is the cost of wall time and it is why fuel and a
timeout are not interchangeable.

Record and replay is what this is for, and it is not built - `PLAN.md` phase 6 is where it could
land. It is stated here because an input path that bypassed a column would quietly make a pass depend
on something nobody can see, and that is not a thing a later phase could recover.

## What bounds a run

Five numbers meter an element, in **three kinds**. What each one bounds, and over what *period*, is
what decides whether a number means anything at all:

    fuel                a system's single invocation. re-set before every dispatch
    init_timeout        one lifecycle call each, in wall time
    reload_timeout
    shutdown_timeout
    memory_limit        the element's own memory, in bytes. a ceiling, never re-set

**Fuel is per invocation, and that follows from *When something fails* below** rather than being
chosen. A single exhaustion has to be survivable and counted, because that section says an abandoned
invocation is counted and the element is stopped only past its budget. A per-lifetime budget would
make the first exhaustion terminal and `max_system_failures` meaningless for it.

It also cannot be shared across a phase, which would be cheaper. One greedy system would starve its
siblings and the trap would land on whichever ran last - which would destroy the attribution
`diagnostics.md`, *What a bad write costs, and what finds it*, is built on.

**So BridgeC re-sets fuel before every dispatch.** It has no choice - fuel only depletes, and a store
left at zero traps immediately (`vm-surface.md`, *A call takes nothing and answers a status*). One
store write before a call is nothing against the call.

That is Quake's model, and the shape is the same twenty-five years later: `pr_exec.c:382` sets
`runaway = 100000` at the top of `PR_ExecuteProgram` and `:399` raises *"runaway loop error"* when it
reaches zero. They counted statements because they had no way to interrupt.

**And counting is enough for a system, which is why there is no per-invocation deadline.** Fuel
counts only wasm instructions, so it would miss a module blocked inside a host call - except that a
module has no such call to be blocked in. Its only imports are BridgeC's eight ABI entries
(`module-abi.md`, *Memories*), and none of them waits on anything. Nothing a system can reach blocks,
so a wall-clock bound beside fuel would guard a case the design does not contain.

**A lifecycle call is the one that needs wall time**, and gets a timeout and no fuel: `init` may
legitimately do a great deal of work, so an instruction count is the wrong proxy for "took too long
once". Those three timeouts are what demands `deadline` of a backend - `vm-surface.md`,
*Capabilities, so the swap is honest* - and `instance_set_deadline` is how they are enforced.

`memory_limit` is not a budget at all. Nothing resets a ceiling.

## When something fails

`result` is an i32. Zero for success; otherwise a code, with any detail written to the log - see
`diagnostics.md`, *The log*.

A non-zero `result` and a trap are **the same failure**. They differ only in how the VM classified
it, and treating them alike is what keeps an `init` that returns an error and an `init` that reads
out of bounds from needing two policies.

The element says only *that* it failed, never how badly. Severity is a deployment decision - a mod
declaring its own failure fatal would take the game down, which is backwards from everything else the
deployment grants it.

What BridgeC does is mostly not a choice, because the set of coherent outcomes differs by call site:

    init        the element never ran, so running without it is coherent:
                `on_error = fail | skip`, per element, default fail
    a system    the invocation is abandoned, counted and logged. Past the
    or an       budget the element is stopped - its systems and observers
    observer    stop running and its components stay
    shutdown    logged, and teardown continues. Nothing else is coherent
    reload      the old instance is kept if it still exists, otherwise
                this is init's case

A mod ships `on_error = "skip"` and the base game `"fail"`, which is the whole of what "a mod must
not take the deployment down" needs.

**A stopped element keeps its components.** Unloading would destroy entities, and that takes other
elements' components on those entities with it. Stale-but-readable is the less destructive failure,
and a deployment that wants the data gone unloads the element - the verb that says so:
`data.md`, *Lifetime*.

BridgeC's own refusals - a grant denied, a fingerprint mismatch, an undeclared phase, a reserved
system name, an ambiguous order, a capability the backend cannot honour - take init's path, because
they are failures to start.

An observer failing is a system failing. It is the same budget, the same count and the same stop,
because it is the same kind of code reached a different way - the only difference is that the work
abandoned was a reaction rather than a pass.

Two guards from idTech3's `Com_Error`, each covering a case nothing else does. **Repetition
escalates** (`common.c:257`): a recoverable failure arriving in a stream stops being recoverable -
there, more than three inside a hundred milliseconds became fatal. And **a failure while handling a
failure is fatal** (`:268`), because the alternative is unbounded recursion in the one path that
cannot afford it.
