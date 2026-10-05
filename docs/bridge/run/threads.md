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

Across modules, two systems whose terms conflict never run at once. Bridge-ASM700 knows which from the
declared terms, so the hot path is lock-free by construction rather than by care.

Those declarations give **parallelism, not order**. Where order comes from, and what happens when
nothing states it: `schedule.md`, *Order within a phase is never implicit*.
