# What was refused: Threads

The record of `pinned.md`: `README.md`, *Three layers, three seams*.

## Threads

Refused: a thread of Bridge-ASM700's own for pinned modules, which would let a host call from
anywhere. It can never be the first thread of the process, and every call into a pinned module would
cross to it and wait. The condition that brings it back is no platform here demanding a particular
thread, and hosts found fighting the rule.

Refused: a thread for each pinned module, which gives up their running one at a time, the thing
modules driving one device need, and cannot be the first thread either. The condition that brings it
back is the same: no platform here demanding a particular thread.

Refused: a thread the host lends for each pinned module, through a callback that runs that module's
job there. It serves a module of either kind, and it is a second seam at the host for a case nothing
has, with every call into the module crossing to that thread and waiting. The condition that brings
it back is a module that must run on a thread of the host's other than the one that asks for phases.
