# Threads

**The execution model is task parallelism between modules, and sequential within one.** Systems
of different modules that do not conflict run at the same instant, each on a thread of its own.
The systems of one module run one after another, each from its start to its return: never two at
once, and never one paused for another. What decides the order is worked out a system at a time,
by the conflict graph; what runs at once is a module at a time.

**A module's systems cannot overlap because they share what only one of them may use.** On the
wasm backend a module is one instance in one store, and a store is entered by one thread. On both
backends it has one private memory: Bridge-ASM700 writes there which system is running and when,
before each call, and the module moves one scratch pointer and one log ring head there. On the
native backend it has its own globals, which nothing guards.

**What that gives up is one heavy system on more than one core.** A deployment buys parallelism
by having more modules whose systems do not conflict, not by declaring more systems; and one loop
over one component cannot be split across two modules either, since both would write it, conflict,
and be ordered.

Refused: data parallelism, one system run on several threads with each over a part of its spans.
It is the one model that would speed up a single loop, and it needs an instance of the module for
each thread, so that spans, the queue of changes and the count of failures all become an
instance's, and a system that keeps anything between rows breaks. Nothing measured asks for it.
The condition that brings it back is a deployment, measured, in which one system takes most of a
phase while other threads have nothing to run: `PLAN.md`, stage 17, measures that. Refused: task
parallelism inside a module, its systems that do not conflict run at once. Each would need an
instance of its own for the same reason, and it helps a module of many small systems and not the
heavy one. The same condition. Refused: concurrency inside a module, a system paused part-way
for another. It gains no time, since nothing a system calls ever waits - `limits.md`, *What
bounds a run* - and a module that wants work spread over several phases keeps where it is in a
component. This section said the model "cannot be worked around", which was not so: it can, at
the price above.

**How many threads is stated**, as `threads`, and it counts the one that called `bridge_phase`:
that thread works too, and at `threads = 1` it does everything, in an order that never varies.
Bridge-ASM700 keeps the others for as long as it exists. Refused: asking the operating system how many cores
it has, which is a value nobody stated and the wrong one inside a host with a thread pool of its
own. Refused: borrowing the host's threads through a callback, a new seam nothing asks for yet. The
condition that brings it back is a host whose job system Bridge-ASM700 has to run on.

**No entry takes a lock, and none reaches storage.** Modules run at once, and what one is given must
not depend on who ran beside it - `schedule.md`, *What a pass depends on*. So everything an entry
touches while a phase runs is the calling module's alone, and a module's systems are serialised, so
each has one writer:

    spawn                    an id from a block set aside for the module at the last barrier
    alloc                    space from the module's own share of the persistent arena
    destroy, attach, detach  onto the module's own queue, with the value attach carries

The barrier, where one thread has the world, applies the queues **in the order the calls that
filled them ran**: the observers of the barrier before, then any lifecycle call made since, then
the phase's systems, a level at a time. Within one call, in the order it asked. Two systems of
one level are in no order, and need none: had they changed the same component they would have
been ordered when they loaded - `schedule.md`, *Order within a phase is never implicit*. Then the
barrier tops each block up. That order is the same every run, so the ids a module is given and
the rows its changes land in are too, on one thread or on eight.

A module's queue is already in that order, so each keeps one mark for every call that queued
something, and the barrier takes the earliest mark any module has next. A mark costs a compare
for each change queued, and the room for them is found when a module gains a system and before
a lifecycle call, never in a phase.

Refused: one lock around every entry, which is what this replaced. It serialised modules that had no
reason to wait for each other, and it made the ids two modules were given in one level a matter of
which thread arrived first. Refused: a queue inside storage for each thread, which is what an ECS
offers for this: whose change is applied first would then follow which thread ran whom. Refused:
declaring that a system mints, as a write is declared, so that two such never share a level - no key
to state, and two modules could then never spawn at once. The condition that brings that one back is
the two keys below proving harder to state than the parallelism is worth.

What it costs is `spawns` and `persistent`, two more values a module states: `budgets.md`,
*Budgets*.

Across modules, two systems whose terms conflict never run at once. Bridge-ASM700 knows which from the
declared terms, so the hot path is lock-free by construction rather than by care.

Those declarations give **parallelism, not order**. Where order comes from, and what happens when
nothing states it: `schedule.md`, *Order within a phase is never implicit*.
