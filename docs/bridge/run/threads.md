# Threads

**An element is the unit of parallelism.** Its systems are serialised, because they share its
instance and an instance cannot be used concurrently. What runs at once is *different elements*. A
deployment buys parallelism by splitting into more elements, not by declaring more systems - and
that cannot be worked around, since an element has one heap and two instances would be two elements.

Across elements, two systems whose terms conflict never run at once. BridgeC knows which from the
declared terms, so the hot path is lock-free by construction rather than by care.

Those declarations give **parallelism, not order**. That two systems both write `game:Position` says
they cannot run together; it does not say which should go first, and two systems declaring `inout
Position` have no derivable order at all. Order comes from phases - see `schedule.md`, *Phases* - and
where a phase leaves it ambiguous, BridgeC refuses to start rather than pick.
