# Phase 6: failure and lifecycle

**Planned, not built.** This document is the plan; when the phase ends it is rewritten as what was
built and what it found.

A module can load, run, and fail one invocation at a time. This phase lets it be stopped, unloaded,
reloaded and configured, and lets a host ask what is running. `PLAN.md` phase 6 is the claim.

## What was settled first

Five questions, each by every step of `PHASE0.md`, and each applied to the document it belongs to.

    what a reload runs            the new instance's reload, then the old one's shutdown:
                                  `lifecycle.md`, *The runtime lifecycle*
    what the budget counts        raised by a failed barrier, lowered by a clean one:
                                  `failure.md`, *The budget*
    fail, on a stop               it covers one, as it covers a load:
                                  `failure.md`, *When something fails*
    which component is settings   the one the author names:
                                  `config.md`, *A module's own settings are a component*
    where a text setting goes     where its field's type says: the same section

Three of them were first put with steps missing, and were put again. Reading the original Bridge's
source for the second pass gave reload a fourth option and changed the recommendation for the
budget.

## What it builds

**The host seam**: `bridge_unload`, `bridge_reload`, `bridge_modules`; and in a module's config
`hot_reload`, `max_system_failures`, the reload and shutdown timeouts, `config` and its settings.

**The module seam**: `bridge_text`. **The backend seam**: a sixth capability, whether two instances
of one module can exist at once.

**Failure.** A mark per module of whether it failed since the last barrier; at each barrier the
count moves, a module at its budget is stopped, and one stated `fail` stops the bridge with it.

**Unload**, in the order `lifecycle.md` gives, with the three policies of `lifetime.md`, *What
unload leaves behind*. Table ids are given back when flecs says a table is gone.

**Reload**, in the order `lifecycle.md` gives, and in the other order where the backend cannot hold
two instances.

**Settings**, written after `init` by what each field is.

## Steps

1. Documents: the five decisions. Committed before any code.
2. The seams.
3. The budget and the stop, `bridge_modules`.
4. Unload, and table ids retired.
5. Reload, both orders.
6. Settings.
7. The tests: one per clause of the claim, and one per refusal the phase adds.
8. Close: this document rewritten, `PLAN.md` marked, scan, Linux.

## What would make us stop

flecs unable to delete a component that entities carried without leaving tables the mirror cannot
retire: then one hook is not enough, and `flecs.md` is wrong about it. Two wasm instances of one
module unable to coexist over the shared block: then a reload takes the native order everywhere,
and loses what keeps the old instance when the new one fails.

## Nothing is left behind it

The phase is not reported done with a list of what remains. No shortcut marked in the code, every
path built with a test, no document promising what the code does not do, and the `linux` preset
run. What cannot be finished is brought as a decision before the report, not after it.
