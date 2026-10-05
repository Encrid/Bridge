# Phase 5: scheduling

Done, on Windows. The `linux` preset is written and has not been run.

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

Eighteen new ones, sixty-one in all, in a release build and a debug one. The three that depend on
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

## Shortcuts taken

Each works, is marked where it is written, and becomes work only if a measurement says so.

**Every ABI entry runs under one lock** - `threads.md`, *Threads*, with what replaces it.

**The schedule is recomputed whole at every load**, with a dense matrix per phase. Stated where it
is written, with what replaces it.

## Open questions

None for the next phase.
