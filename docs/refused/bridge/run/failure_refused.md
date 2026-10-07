# What was refused: When something fails

The record of `failure.md`: `readme.md`, *Three layers, three seams*.

## When something fails

Refused: answering a return code and nothing else, which would let a host that ignores it run a
deployment without the module its config called essential. No condition brings it back.

Refused: stopped until `bridge_destroy`, which is what this first said - the `fail` of
`legacy/rust-components` ends its process, which it owns; ours would have ended every module's
storage to mend one module, and made reload useless for exactly the modules a deployment calls
essential. No condition brings it back.

Refused: accepting the unload of any other module, which has no job once `bridge_destroy` shuts
every module down. The condition that brings it back is a host that must take several modules out
before it can run again.

Refused: accepting no unload at all, which is what this had: to go on without one module, or to put
one of another name in its place, cost every module its storage. No condition brings it back.

Refused: `on_error` as a key about loading only, which is what `legacy/rust-components` has and could
afford: it owns its process, and a crashed module there fails the calls made to it. The condition
that brings it back is a host that must keep presenting while an essential module is down.

Refused: a second key for being lost later, which answers the same question twice. The condition
that brings it back is a module that must load and need not stay.

## The budget

Of `failure.md`, *The budget*.

Refused: one count for a module, raised by a barrier in which anything of its failed and lowered by
one in which it ran clean, which is what this was. There is a barrier after every phase, so a module
with a system that always failed in one phase and a system that ran clean in the next went up one
and down one for ever, and was never stopped: the fault the next refusal names. `tests/host`, case
`module_config.max_system_failures-clean-system-forgives-no-other`, is that module. No condition
brings it back.

Refused: a count that one clean barrier resets, which is easier to say and never stops a module
failing every other pass. The condition that brings it back is deployments finding this one hard to
reason about.

Refused: a total since load, which stops a module for failures it recovered from. The condition that
brings it back is a deployment that must stop a module after so many failures, however far apart.

Refused: a count within a window, which is two keys. The condition that brings it back is a
deployment that needs a burst and a rate told apart.

## Out of memory

Of `failure.md`, *Out of memory*.

Refused: an allocation whose answer nobody looked at, which is what all but one of them were. A
null answer was written through, and the host's process ended with no record. No condition
brings it back.

Refused: one place that stops the process with a message. It is a few lines where this is a
check at every allocation; and it has the library end a process it does not own, which is the
one thing *When something fails* says it cannot do. No condition brings it back.

Refused: nothing allocated after create, every record a load needs taken from a pool that
`[budget]` sizes. Then a load cannot run out of memory at all; and every record of a module is
made again as a pool, sized for the most there may ever be, by keys nobody states today. The
condition that brings it back is a host that cannot bear a load failing for memory.
