# Threads

**A module is the unit of parallelism.** Its systems are serialised, because they share its
instance and an instance cannot be used concurrently. What runs at once is *different modules*. A
deployment buys parallelism by splitting into more modules, not by declaring more systems - and
that cannot be worked around, since a module has one heap and two instances would be two modules.

**How many threads is stated**, as `threads`, and it counts the one that called `bridge_phase`:
that thread works too, and at `threads = 1` it does everything, in an order that never varies.
Bridge-ASM700 keeps the others for as long as it exists. Refused: asking the system how many cores
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

The barrier, where one thread has the world, applies the queues: module by module in the order
they loaded, and within one in the order it asked. Then it tops each block up. That order is the
same every run, so the ids a module is given and the rows its changes land in are too, on one
thread or on eight.

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
