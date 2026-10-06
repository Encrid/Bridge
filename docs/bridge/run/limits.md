# What bounds a run

Five numbers bound a module, in **three kinds**. What each one bounds, and over what *period*,
decides whether a number means anything:

    work_limit          a system's single invocation. re-set before every dispatch
    init_timeout        one lifecycle call each, in wall time
    reload_timeout
    shutdown_timeout
    memory_limit        the module's own memory, in bytes. a ceiling, never re-set

**The work limit is per invocation, and that follows from `failure.md`, *When something fails*** rather than
being chosen. That section says an abandoned invocation is counted and the module is stopped only past
its budget, so a single exhaustion has to be survivable. A per-lifetime budget would make the first
exhaustion terminal and `max_system_failures` meaningless for it.

It cannot be shared across a phase either, which would be cheaper. One greedy system would starve
its siblings and the trap would land on whichever ran last - destroying the attribution
`validation.md`, *What a bad write costs, and what finds it*, is built on.

**So Bridge-ASM700 re-sets the work limit before every dispatch.** It has no choice: what is left of
one only goes down, and a store left at zero traps immediately (`signatures.md`, *A call takes
nothing and answers a status*). One store write before a call is nothing against the call.

That is Quake's model: `pr_exec.c:382` sets `runaway = 100000` at the top of `PR_ExecuteProgram` and
`:399` raises *"runaway loop error"* when it reaches zero. They counted statements because they had
no way to interrupt.

**Counting is enough for a system, which is why there is no per-invocation deadline.** On the wasm
backend it counts only wasm instructions, so it would miss a module blocked inside a call it made
out - except that a module has no such call. Its only imports are Bridge-ASM700's eight ABI entries
(`wasm.md`, *Memories*), and none of them waits on anything. A wall-clock bound beside a work limit
would guard a case the design does not contain.

**A lifecycle call is the one that needs wall time**, and gets a timeout and no work limit: `init` may
legitimately do a great deal of work, so an instruction count is the wrong proxy for "took too long
once". Those three timeouts are what demands `timeout` of a backend - `capabilities.md`,
*Capabilities, so the swap is honest* - and `instance_set_timeout` is how they are enforced.

`memory_limit` is not a budget at all. Nothing resets a ceiling. It demands the capability of
the same name of a backend, as `work_limit` does.

All five are stated, and "no limit" is a value like any other: `-1`. `capabilities.md`,
*Capabilities, so the swap is honest*.

## What a bound costs

**A bound is paid for by the module that states it, and by no other.** A work limit is counted in the
compiled code, on the path this design exists to keep free, and so is the check that lets a deadline
interrupt. On the wasm backend, over the tightest loop a system can have - one float read, changed
and written back per row - that measured as:

    neither             0.29 ns a row
    a deadline          0.43            x1.5
    a work limit        0.64            x2.2
    both                0.65            x2.2

That is the worst case: the heavier a row's work, the smaller the share. A system dispatched by
`bridge_phase` over the same loop measures the same way, 0.39 ns a row without a work limit and 0.65
with one. It is still a factor and not a rounding error, so `-1` has to mean the cost is gone and
not merely that the budget is large. A module stating `work_limit = -1` is compiled without the
count, and one stating `-1` for all three timeouts without the interrupt check - `backend.md`, *What
a backend is asked for*, hands the backend what was demanded for that reason.

The original Bridge found the same thing the slow way: its fuel accounting was on for every module
while `work_limit = -1` everywhere was the common deployment, "and it was paying anyway".

What a module gives up with `work_limit = -1` is exactly what it says: a system of its that never returns
is never stopped, and the pass hangs with it, as it would for a native module.

Refused: bounding a system by a deadline instead, which is the cheaper of the two. A timeout firing
is the one thing a pass cannot reproduce - `schedule.md`, *What a pass depends on* - and it would
have to be armed on every dispatch and for several systems at once. The condition that brings it
back is record and replay being given up for good.

Refused: counting in the module's own code at loop back-edges, by a rewriter of ours. That is an
instrumenting compiler inside the library, for a gain nobody has measured. The condition that
brings it back is a deployment with work limits that cannot afford the factor above.
