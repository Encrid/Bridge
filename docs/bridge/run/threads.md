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

**What a module calls is serialised, and what it reads is not.** Every ABI entry runs under one
lock, because neither flecs' world nor the persistent arena is safe from two threads. That is the
right place for a lock to be: reading and writing components is the hot path and is no call, and an
entry is the rare thing a module does when the shape of the world changes. Two modules that both
spawn heavily in one phase do wait on each other. The condition that changes it is that wait
showing in a profile, and flecs' stages are what replaces the lock.

Across modules, two systems whose terms conflict never run at once. Bridge-ASM700 knows which from the
declared terms, so the hot path is lock-free by construction rather than by care.

Those declarations give **parallelism, not order**. Where order comes from, and what happens when
nothing states it: `schedule.md`, *Order within a phase is never implicit*.
