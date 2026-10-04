# When something fails

`result` is an i32. Zero for success; otherwise a code, with any detail written to the log - see
`log.md`, *The log*.

A non-zero `result` and a trap are **the same failure**. They differ only in how the backend classified
it. Treating them alike keeps an `init` that returns an error and an `init` that reads out of bounds
from needing two policies.

The module says only *that* it failed, never how badly. Severity is a deployment decision - a mod
declaring its own failure fatal would take the game down, which is backwards from everything else
the deployment grants it.

What Bridge-ASM700 does is mostly not a choice, because the set of coherent outcomes differs by call site:

    init        the module never ran, so running without it is coherent:
                `on_error = fail | skip`, per module, and stated: there is no default
    a system    the invocation is abandoned, counted and logged. Past the
    or an       budget the module is stopped - its systems and observers
    observer    stop running and its components stay
    shutdown    logged, and teardown continues. Nothing else is coherent
    reload      the old instance is kept if it still exists, otherwise
                this is init's case

A mod ships `on_error = "skip"` and the base game `"fail"`, which is the whole of what "a mod must
not take the deployment down" needs.

**`fail` stops the bridge, since Bridge-ASM700 cannot stop the program.** The host owns the process,
so the most the library can do is refuse to go on: after a `fail` module fails, every `bridge_phase`
and `bridge_load` is refused until `bridge_destroy`. `skip` refuses that one load and carries on.
Refused: answering a return code and nothing else, which would let a host that ignores it run a
deployment without the module its config called essential.

**A stopped module keeps its components.** Stopping is something Bridge-ASM700 does on its own
initiative, and unloading discards data, so the two are kept apart: Bridge-ASM700 never discards data
uninvited. Stale-but-readable is the less destructive failure.
A deployment that wants the data gone unloads the module - the verb that says so: `lifetime.md`,
*Lifetime*.

Bridge-ASM700's own refusals - a grant denied, a fingerprint mismatch, an undeclared phase, a reserved
system name, an ambiguous order, a capability the backend cannot honour - take init's path, because
they are failures to load.

An observer failing is a system failing: the same budget, the same count and the same stop, because
it is the same kind of code reached a different way. The only difference is that the work abandoned
was a reaction rather than a pass.

Two guards from idTech3's `Com_Error`, each covering a case nothing else does. **Repetition
escalates** (`common.c:257`): a recoverable failure arriving in a stream stops being recoverable -
there, more than three inside a hundred milliseconds became fatal. And **a failure while handling a
failure is fatal** (`:268`), because the alternative is unbounded recursion in the one path that
cannot afford it.
