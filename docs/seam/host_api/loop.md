# The loop

Who advances Bridge, and who decides when the program ends. Both are the host's.

## The host owns the loop, and the order within it

`bridge_run_phase` runs one named phase and its barrier. **There is no `bridge_step`.** The host calls
the phases it wants, as often as it wants.

**No phase is Bridge's**, so every name here is one the deployment declared - `schedule.md`,
*Bridge ships no phase names*.

That is what makes a **fixed timestep** expressible: physics at 60 Hz with a variable framerate
means calling one phase more often than another, which a single advance-everything call cannot say.
Same for a host that owns its render loop and wants `setup` and `update`, then its own work, then
`present`.

It also follows from Bridge not looping. The host already owns the loop, so it owns what goes in
it.

**Two costs, both real.** `phases` has no `after` relation - nothing needs a dependency graph to
order phases the host is ordering itself. And **the host can forget a phase**: call `update` and
never `present`, and half the deployment quietly does nothing. Bridge cannot detect that, because
it has no notion of a complete pass.

What survives untouched is the refusal when two systems *within* one phase conflict unordered -
`schedule.md`, *Order within a phase is never implicit*. That was never about phase order.

## Bridge has no notion of being done

It runs the systems registered to the phase it was asked for, and at the barrier the observers of
whatever changed. If nothing is registered, it runs nothing. There is no idle status, no completion
tracking, and no way for a module to ask the process to stop - a module handles its own
lifecycle, and a system with nothing to do iterates zero spans.

**When the program ends is the host's question.** A host loops until a signal, or a window closes,
or because it was told to run once. A one-run program is then not a special case: it calls each
phase once and exits.

The one thing Bridge does track per module is failure - *"past the budget the module is
stopped"*, `failure.md`, *When something fails*. That is noticing you keep failing, not asking
whether you are finished, and it exists because a system failing every pass forever is what
idTech3's escalation guards against.
