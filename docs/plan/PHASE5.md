# Phase 5: scheduling

Done. The `linux` preset was first run after phase 5, and passed as written.

The phase replaced "registration order, one at a time" with the schedule `schedule.md` describes: a
conflict graph, `[order]`, observers at the barrier, threads, and `pinned`. `PLAN.md` phase 5 is
the claim. This document is what the phase built and what it found on the way.

## What was settled first

Four questions, each by the method of `PHASE0.md`, and each applied to the document it belongs to.

    what an observer reads            the ids that triggered it, and the events:
                                      `abi.md`, *The ABI*
    how `[order]` names a system      the module's identity without its version, then the system:
                                      `schedule.md`, *Order within a phase is never implicit*
    a module's own conflicting pair   the order it registered them in: the same section
    how many threads                  a stated key, `threads`, counting the caller's:
                                      `threads.md`, *Threads*

One more was forced and not chosen: a pinned module runs on the thread that calls `bridge_phase`,
which has to be the one that loaded it - `pinned.md`, *Threads*.

## What it produced

    include/bridge.h              [order], threads, pinned
    include/bridge-abi.h          the three events, the trigger record, the observer slot table
    include/bridge-wasm.h         registering an observer, and reading its list
    include/bridge-native.h       the same, through pointers
    src/schedule.c                conflicts, edges, the refusal, levels, and running a level
    src/pool.c                    the workers, and the caller working beside them
    src/abi.c                     observe; system and observe share one registration; the lock
    src/store.c                   an observer's list published; observers run at the barrier
    src/os.c                      a mutex, and the calling thread's id
    tests/phase5.c                a host with both backends, order, and threads
    tests/modules/                five wasm modules and three native ones

## The tests

Twenty-eight new ones, seventy-one in all, in a release build and a debug one. The three that depend on
timing were run six times over.

    order        the mover adds 1, then the halver halves, by one `[order]` pair: sum 125,250
    reorder      the pair reversed: 125,500
    ambiguous    no pair: the halver is refused, both systems and the component named, and the
                 deployment runs on without it
    ownorder     one module, doubling then adding as registered: 3 from 1, where the other way
                 is 4
    cycle        two pairs that contradict: create refuses
    observe      a native system attaches a value; a wasm observer reads that value through the
                 id, under the same barrier count as the attach
    chain        three observers in a chain: barriers 1, 2 and 3, each hop said once
    reserved     an observer named `init`: refused
    events       one observer of all three: an attach to an entity without the component
                 arrives as add and set, an attach to one with it as set, a detach as remove,
                 with an id that then leads nowhere
    obsconflict  two modules' observers of one change, one writing what the other reads: the
                 second load refused, both named, "at the barrier"
    obsorder     the same two ordered by `[order]`: the reader sees 77, the value attached
    obsreorder   the pair reversed: it sees 5, the value the other observer wrote
    parallel     two native modules each sleep 100 ms, two threads: 100 ms, two thread ids
    serial       the same on one thread: 200 ms, one thread id
    pinned       a pinned module among two others on four threads: 9 of its 9 calls on the
                 thread that loaded it, while the others used two
    wrongthread  a phase asked for from a thread that loaded nothing: refused by name
    nothreads    `threads` left at zero: create refuses, naming the key
    hop          two wasm modules on four threads for fifty frames: the last sum is exact

## What the phase found

**flecs' observers fire where `data.md` said.** A deferred attach reaches the observer at the flush
and not before, on the release pinned here and not only on the snapshot the spike measured. The
stop condition did not happen.

**A wasmtime store may be called from any thread, one at a time.** `hop` runs two wasm modules for
fifty frames with their jobs taken by whichever thread is free. Nothing in a store is tied to the
thread that made it, so `threads.md` holds and `pinned` stays the exception.

**The barrier has to go back to deferring before an observer runs, not after.** Otherwise what an
observer attaches applies at once, and a chain settles inside one barrier - which is the fixpoint
`schedule.md` refuses. The first arrangement had it at the end, where phase 3 left it.

**An observer and a system are the same registration.** Name, terms, the refusals, the export
check and the conflict graph are shared; an observer differs by having events instead of a phase
and a list instead of spans. `abi.md` had said so, and the code agrees in one function.

**Records from a worker would have reached the sink off the calling thread.** A system's failure
is noted where it happens and said after the join, so the sink keeps the property `create.md`
gives it.

## Closed after the phase

Everything earlier phases had built without testing, listed and not built, never run, or marked as
a shortcut, found by asking what "left" meant in a phase report.

**The set and remove events, and observers that conflict across modules**, had no test. They have
`events`, `obsconflict`, `obsorder` and `obsreorder` above.

**A field whose type is another declared type** was in `schema.md`'s type set from the start and
refused by phase 3's code: a nested struct, an enum, a bitmask. `declare` gained a kind, and a
declared type is named in flecs under its namespace - `schema.md`, *The type set*. Two tests:

    nested       a component of a struct, an enum, a bitmask and a float: all four read back
    nested2      the same without declaring the struct: refused, with the name flecs missed

Found on the way: flecs finishes an enum's layout in observers of its own, and what they queue is
only applied when a deferral ends. `declare` had suspended deferral, which is not ending it, so an
enum had no layout until the next barrier. It ends it now, for the length of the declaration.

**Linux.** Five phases had been met on Windows alone. Every source was cross-compiled and linked
for Linux with zig, clean under `-Wall -Wextra`; then `.github/workflows/linux.yml` ran the `linux`
preset on a real one, and all sixty-three tests passed on the first run. It runs on every push now -
`PLAN.md`, at its head.

**Every shortcut that was marked in the code**, since a shortcut with a note beside it is deferred
work, and deferred work and its documents drift apart:

    backend settings      typed, passed through, and a key the backend does not know refused
    the schedule          reachability as a row of bits per system
    the column allocator  exact sizes, gaps joined, growth in place - `flecs.md`, *It can be
                          confined*
    one bridge a process  as many as a host creates, each its own world, at once
    a load that stopped   answers that it did not succeed
    the bridge

`setting` and `two` are the tests the first and fourth gained. The allocator moved a number:
flecs' own tables fit in about 50 KB of columns where the first allocator needed 70.

**Figures that had come from another flecs** were taken again on the one pinned - `reference.md`,
*Reference following*, `data.md`, *Observers*, and `flecs.md`, *It can be confined*, where what
confinement keeps out of reach went from 87% and 38% to 93% and 77%.

**Reports that repeated the documents they fed** cite them now. A figure said twice is two figures
the day one of them moves, and one already had: the column floor.

**The lock around every ABI entry**, which was the last and took a decision: `threads.md`,
*Threads*. Looking for what replaces it found two things. flecs' stages, which the note beside the
lock named as its replacement, refuse to create an entity while they are in use. And the lock had
been hiding a fault: with more than one thread, the ids two modules were given in one level, and
the order their changes reached storage, depended on which thread took the lock first - against
`schedule.md`, *What a pass depends on*.

So entries reach nothing shared. A module spawns from ids set aside for it, allocates from its own
share of the arena, and queues its changes, and the barrier applies the queues in the order
modules loaded. Two keys per module pay for it, `spawns` and `persistent`. flecs' own deferral is
gone with it: nothing reaches flecs outside a barrier or a load.

    minted       two modules spawning in one level: the same 160 ids on one thread, on four,
                 and on four again
    strings      a string kept in a component, from the module's share of the arena
    noarena      a share too small for it: its `alloc` abandons `init`, by name
    bigarena     a share larger than what is left of the arena: refused at load
    nospawns     a module that stated it spawns nothing, and spawns: abandoned, by name
    undo         a load that fails after declaring: the same name with another layout then
                 loads, which a leftover of the first would refuse

`undo` is the other thing this closed. A failed load now takes back the ids it was given, its
share of the arena and every component nobody else declared, and what it queued goes with its
queue. That was `PLAN.md` phase 6's clause, and phase 6 now only has to keep it true for unload.

## Nothing left

No shortcut is marked in the code, and no report lists one.

## Open questions

None for the next phase.
