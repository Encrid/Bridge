# No builtins

The original Bridge shipped `log`, `ctl`, `config` and `profile` as host-implemented interfaces -
the measured "built-in host call, 500 ns". Bridge-ASM700 has none. The rule is **no builtin
surface**: nothing a module imports, no component Bridge-ASM700 declares, and **no name
Bridge-ASM700 invents for a deployment's own structure**. It still takes config as structs, drains
log rings and keeps a clock for its own records; none of that is reachable from a module.

The last clause is about structure, not the calling convention. `init`, `shutdown` and `reload`
**are** three names Bridge-ASM700 chose, and they stay: something has to be called first, and no
deployment decision hides behind that - every loader has an entry name, from `main` to `vmMain`. A
phase name is different: a guess about the shape of a frame that every module would then have to
reference. `schedule.md`, *Bridge-ASM700 ships no phase names*.

Why: a builtin is a door that stays open. Each one is defensible alone and the set only grows, which
ends up contradicting the minimality the rest rests on. With none, the ABI is eight entries and has
no *reason* to grow - a new capability is a new module, not a new import.

**The test is whether a thing is universal to every deployment.** Config is: Bridge-ASM700 cannot start
without its phases and its budgets. Storage and the memories are. A **clock is not** - a one-run
program has no frame, and a component every deployment carried regardless would be content baked
into the orchestrator. So is an engine, a device, a filesystem.

The cost: nothing works out of the box, and two authors needing "time" no longer agree for free.

**Nothing ships a clock, and nothing ships a set of phases.** Not the library, by the test above;
and not a host, which runs what a deployment stated and invents nothing for it. A deployment that
needs time has a native module that declares a component and writes it, and that module is the
deployment's own.

What would let two authors agree is content that is neither the library's nor a host's: a
module with a name and a version - `bridge:clock@1`, say - that a deployment chooses to depend on,
can fork, and can do without. That is not a hole in the rule: a name baked into the orchestrator
would be unavoidable and unreplaceable, and this is neither. **It does not exist, and it is not
built on a guess.** `PLAN.md`, stage 17, builds the first real deployment, which needs both a clock
and phases; they are that deployment's own, and whether either becomes shared content is asked
there. The condition that makes it is a second deployment, by another author, needing the same.
