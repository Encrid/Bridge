# When something fails

`result` is an i32. Zero for success; anything else is failure, and which value it was is not
read. Any detail is written to the log - see `log.md`, *The log*.

A non-zero `result` and a trap are **the same failure**. They differ only in how the backend classified
it. Treating them alike keeps an `init` that returns an error and an `init` that reads out of bounds
from needing two policies.

The module says only *that* it failed, never how badly. Severity is a deployment decision - a mod
declaring its own failure fatal would take the game down, which is backwards from everything else
the deployment grants it.

What Bridge-ASM700 does is mostly not a choice, because the set of coherent outcomes differs by call site:

    init        the module never ran, so running without it is coherent:
                `on_error = fail | skip`, per module, and stated: there is no default
    a system    the invocation is abandoned, counted and logged. At its
    or an       budget the module is stopped - its systems and observers
    observer    stop running and its components stay - and `on_error`
                says what that costs the deployment
    shutdown    logged, and teardown continues. Nothing else is coherent
    reload      the old instance is kept if it still exists. otherwise the
                module is stopped, keeping what it declared

A mod ships `on_error = "skip"` and the base game `"fail"`, which is the whole of what "a mod must
not take the deployment down" needs.

**`fail` stops the bridge, since Bridge-ASM700 cannot stop the program.** The host owns the process,
so the most the library can do is refuse to go on: after a `fail` module fails, every `bridge_phase`
is refused, and every load, unload and reload but one. `skip` refuses that one load and carries on.
Refused: answering a return code and nothing else, which would let a host that ignores it run a
deployment without the module its config called essential.

**The one call a stopped bridge accepts is the one that brings that module back**: a load of it, if
it never loaded, or a reload of it. If that succeeds the bridge runs again, and says so. The reason
for stopping was running without the module, and with the module back there is nothing left of it.
Refused: stopped until `bridge_destroy`, which is what this first said - the original Bridge's
`fail` ends its process, which it owns; ours would have ended every module's storage to mend one
module, and made reload useless for exactly the modules a deployment calls essential. Refused:
accepting unloads as well, which has no job once `bridge_destroy` shuts every module down.

Storage running out does not stop a bridge. A barrier that cannot place an entity, for want of a
table or of columns, leaves it as it was and counts as a barrier the module that asked failed in,
beside whatever its systems did - `budgets.md`, *What running out does*.

**`fail` covers a stop as it covers a load.** A module stated `fail` and then stopped at its budget
stops the bridge too, because the reason does not change with the frame number: the deployment
called it essential, and nothing else here would notice it gone - no module calls another, so none
gets an error back. The key is `on_error`, and a reader who wrote `fail` expects it to mean every
error that takes the module away. `bridge_modules` says which module it was.

Refused: `on_error` as a key about loading only, which is what the original Bridge has and could
afford: it owns its process, and a crashed module there fails the calls made to it. The condition
that brings it back is a host that must keep presenting while an essential module is down. Refused:
a second key for being lost later, which answers the same question twice. The condition that brings
it back is a module that must load and need not stay.

## The budget

`max_system_failures` is a count that **a failed barrier raises by one and a clean barrier lowers
by one**, never below zero. A module that reaches the number is stopped. A barrier is failed for a
module when any of its systems or observers failed since the one before, or when an entity it
changed could not be placed - `budgets.md`, *What running out does*; it is clean when the module
ran and neither happened.

The unit is the barrier and not the invocation, so that a module is not punished for being made of
many systems. And it is not wall time, which is the one thing a pass cannot reproduce -
`schedule.md`, *What a pass depends on*.

**What forgives is sustained health, not one good pass.** The original Bridge forgives its retry
budget after a minute of running, and says why: the budget is there to catch a module that cannot
run, *"not one that has a bad day twice a year"*. One count that falls as slowly as it rises does
that with a single number, and stops any module that fails more often than not.

`-1`, `BRIDGE_NO_LIMIT`, means never stop it, as for every limit.

Refused: a count that one clean barrier resets, which is easier to say and never stops a module
failing every other pass. The condition that brings it back is deployments finding this one hard to
reason about. Refused: a total since load, which stops a module for failures it recovered from.
Refused: a count within a window, which is two keys. The condition that brings it back is a
deployment that needs a burst and a rate told apart.

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
there, more than three inside a hundred milliseconds became fatal. Here that is *The budget*, above.
And **a failure while handling a failure is fatal** (`:268`), because the alternative is unbounded
recursion in the one path that cannot afford it. Here handling a failure means taking a module out,
and what can fail then is its `shutdown`: that is logged, never retried and never counted, and the
teardown goes on. A `reload` that fails is likewise said once and not tried again.
