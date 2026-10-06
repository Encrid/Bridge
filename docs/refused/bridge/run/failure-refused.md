# What was refused: When something fails

The record of `failure.md`: `README.md`, *Three layers, three seams*.

## When something fails

Refused: answering a return code and nothing else, which would let a host that ignores it run a
deployment without the module its config called essential. No condition brings it back.

Refused: stopped until `bridge_destroy`, which is what this first said - the original Bridge's
`fail` ends its process, which it owns; ours would have ended every module's storage to mend one
module, and made reload useless for exactly the modules a deployment calls essential. No condition
brings it back.

Refused: accepting the unload of any other module, which has no job once `bridge_destroy` shuts
every module down. The condition that brings it back is a host that must take several modules out
before it can run again.

Refused: accepting no unload at all, which is what this had: to go on without one module, or to put
one of another name in its place, cost every module its storage. No condition brings it back.

Refused: `on_error` as a key about loading only, which is what the original Bridge has and could
afford: it owns its process, and a crashed module there fails the calls made to it. The condition
that brings it back is a host that must keep presenting while an essential module is down.

Refused: a second key for being lost later, which answers the same question twice. The condition
that brings it back is a module that must load and need not stay.

## The budget

Of `failure.md`, *The budget*.

Refused: one count for a module, raised by a barrier in which anything of its failed and lowered by
one in which it ran clean, which is what this was. There is a barrier after every phase, so a module
with a system that always failed in one phase and a system that ran clean in the next went up one
and down one for ever, and was never stopped: the fault the next refusal names. `tests/stage6.c`,
case `notmasked`, is that module. No condition brings it back.

Refused: a count that one clean barrier resets, which is easier to say and never stops a module
failing every other pass. The condition that brings it back is deployments finding this one hard to
reason about.

Refused: a total since load, which stops a module for failures it recovered from. The condition that
brings it back is a deployment that must stop a module after so many failures, however far apart.

Refused: a count within a window, which is two keys. The condition that brings it back is a
deployment that needs a burst and a rate told apart.
