# What was refused: The runtime lifecycle

The record of `lifecycle.md`: `readme.md`, *Three layers, three seams*.

## The runtime lifecycle

Refused: Bridge-ASM700 opening a native library itself, which would make native code a case of its
own in Bridge-ASM700 where a backend already has the entry for it. The condition that brings it back
is native code that must run with no backend at all.

Refused: a separate start call validating the whole schedule, which would need a notion of a
complete deployment that Bridge-ASM700 does not have. The condition that brings it back is a host
that needs all-or-nothing loading and cannot get it by destroying the bridge.

Refused: the original Bridge's arrangement, where the old instance's `reload` consents and the new
one runs `init` - right there, where nothing survived a reload, and wrong here, where `init` would
populate storage that is already populated. Its veto has no job left either: a reload is between
phases, so no module is in the middle of anything. The condition that brings the veto back is a
module that must be able to refuse.

Refused: one `init` told by a flag that storage survived, which is one path to keep in step and a
branch in every `init`. The condition that brings it back is authors unable to keep the two
registering the same things.

Refused: taking the module out as an unload would, which is what this first did - whether a failed
reload cost a module its data then depended on which backend it ran on, a cleanup whose condition
nobody calling it was thinking of.

Refused: no reload at all on such a backend, which gives up what `capabilities.md` lists. The
condition that brings it back is a module left stopped by a failed reload shown to be worse than no
reload at all.