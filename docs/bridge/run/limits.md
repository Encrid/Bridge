# What bounds a run

Five numbers meter a module, in **three kinds**. What each one bounds, and over what *period*,
decides whether a number means anything:

    fuel                a system's single invocation. re-set before every dispatch
    init_timeout        one lifecycle call each, in wall time
    reload_timeout
    shutdown_timeout
    memory_limit        the module's own memory, in bytes. a ceiling, never re-set

**Fuel is per invocation, and that follows from `failure.md`, *When something fails*** rather than
being chosen. That section says an abandoned invocation is counted and the module is stopped only past
its budget, so a single exhaustion has to be survivable. A per-lifetime budget would make the first
exhaustion terminal and `max_system_failures` meaningless for it.

It cannot be shared across a phase either, which would be cheaper. One greedy system would starve
its siblings and the trap would land on whichever ran last - destroying the attribution
`validation.md`, *What a bad write costs, and what finds it*, is built on.

**So Bridge-ASM700 re-sets fuel before every dispatch.** It has no choice: fuel only depletes, and a store
left at zero traps immediately (`signatures.md`, *A call takes nothing and answers a status*). One
store write before a call is nothing against the call.

That is Quake's model: `pr_exec.c:382` sets `runaway = 100000` at the top of `PR_ExecuteProgram` and
`:399` raises *"runaway loop error"* when it reaches zero. They counted statements because they had
no way to interrupt.

**Counting is enough for a system, which is why there is no per-invocation deadline.** Fuel counts
only wasm instructions, so it would miss a module blocked inside a host call - except that a module
has no such call. Its only imports are Bridge-ASM700's eight ABI entries (`wasm.md`, *Memories*),
and none of them waits on anything. A wall-clock bound beside fuel would guard a case the design
does not contain.

**A lifecycle call is the one that needs wall time**, and gets a timeout and no fuel: `init` may
legitimately do a great deal of work, so an instruction count is the wrong proxy for "took too long
once". Those three timeouts are what demands `deadline` of a backend - `capabilities.md`,
*Capabilities, so the swap is honest* - and `instance_set_deadline` is how they are enforced.

`memory_limit` is not a budget at all. Nothing resets a ceiling. It is what demands `limit` of a
backend, as `fuel` demands `meter`.

All five are stated, and "no limit" is a value like any other: `-1`. `capabilities.md`,
*Capabilities, so the swap is honest*.
