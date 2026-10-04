# Threads

**A module is the unit of parallelism.** Its systems are serialised, because they share its
instance and an instance cannot be used concurrently. What runs at once is *different modules*. A
deployment buys parallelism by splitting into more modules, not by declaring more systems - and
that cannot be worked around, since a module has one heap and two instances would be two modules.

Across modules, two systems whose terms conflict never run at once. Bridge-ASM700 knows which from the
declared terms, so the hot path is lock-free by construction rather than by care.

Those declarations give **parallelism, not order**. Where order comes from, and what happens when
nothing states it: `schedule.md`, *Order within a phase is never implicit*.
