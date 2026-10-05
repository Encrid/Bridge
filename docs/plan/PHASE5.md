# Phase 5: scheduling

**Planned, not built.** This document is the plan; when the phase ends it is rewritten as what was
built and what it found.

The phase replaces "registration order, one at a time" with the schedule `schedule.md` describes: a
conflict graph, `[order]`, observers at the barrier, threads, and `pinned`. `PLAN.md` phase 5 is
the claim.

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

## What it builds

**The host seam**: `[order]` and `threads` in `bridge_config`, `pinned` in a module's config.

**The module seam**: the three events, the trigger record, and an observer slot table in the
private memory's header.

**The schedule**, recomputed at every load. Two systems conflict when they share a component one
of them writes. The edges are `[order]`'s pairs and, inside one module, registration order. A
conflicting pair of two modules with no path between them refuses the load, naming both. Each phase
becomes levels, and a level is one job per module. Observers are scheduled the same way, as what
runs at a barrier.

**Observers.** `observe` makes a flecs observer whose callback only notes who and what. At the
barrier, after the publish, the lists are written into each module's span region, the observers that
have one run, and the lists are cleared. What they queue waits for the next barrier.

**Threads.** A pool of `threads - 1` workers kept for the bridge's life. A level's jobs are shared
between them and the caller, and a pinned module's job is the caller's alone. Failures are collected
and said after the join, so the sink stays on the calling thread.

**The entries under one lock**, since neither flecs' world nor the arena is safe from two threads.
Stated where it is written, with what replaces it.

## Steps

1. Documents: the four decisions. Committed before any code.
2. The seams: `bridge.h`, `bridge-abi.h`, and the two binding headers.
3. `[order]` at create, the conflict graph, the refusal at load, levels. Still on one thread.
4. Observers.
5. The pool, the lock, `pinned`.
6. The tests below.
7. Close: this document rewritten, `PLAN.md` marked, scan.

## The tests

    order        two modules ordered by `[order]` alone; the reversed pair gives the other sum
    ambiguous    the same two with no `[order]`: the second load refused, both named
    ownorder     one module, two conflicting systems: it loads, and they run as registered
    cycle        pairs that contradict each other: create refuses
    observe      a native module attaches; a wasm observer logs that id at the same barrier
    chain        three observers in a chain: three consecutive barriers
    reserved     an observer named `init`: refused
    parallel     two native modules each sleep 100 ms, on two threads: under 180 ms
    serial       the same on one thread: over 190 ms
    pinned       a pinned module's three systems, among others, on four threads: one thread id
    wrongthread  a phase asked for from another thread, with a pinned module loaded: refused
    nothreads    `threads` left at zero: create refuses, naming the key

## What would make us stop

flecs' observer not firing at the flush for a deferred attach, or firing before it: `data.md`,
*Observers*, rests on that, and the measurement behind it was taken on another flecs. A wasmtime
store misbehaving when its instance is called from different threads in different phases: then every
module is pinned in effect, and `threads.md` is wrong.
