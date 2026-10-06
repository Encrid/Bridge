# The runtime lifecycle

**Legal only between phases**, never while `bridge_phase` is executing. All three restructure
storage and the schedule, which is what a barrier makes safe. A native module holding the
pointer could break this; nothing contains native code, so it is a contract rather than a guarantee.

**`load`** takes an artifact rather than a path, because Bridge-ASM700 decides on `cfg` and never finds
or reads a module's file. In order: find the backend the artifact names, check the ABI version,
check `trusted` against that backend's `sandbox`, check the grants, take a per-module budget slot,
hand `bytes` and `path` to the backend, call `init` - which calls `declare`, `system` and
`observe` - recompute the conflict graph and **refuse-and-unwind if a new system or observer
conflicts unordered with an existing one**, then publish spans.

**Every module loads through a backend, and the steps are the same on each.** An artifact naming a
backend the deployment did not configure is refused by name. Refused: Bridge-ASM700 opening a native
library itself, which would make native code a case of its own in Bridge-ASM700 where a backend already
has the entry for it. The condition that brings it back is native code that must run with no backend
at all.

**Read, then decide, then load** survives the host owning the filesystem: the identity arrives as
data in `cfg`, so the decision is made with the artifact untouched.

The conflict refusal can only happen here. No system exists at create, so there is nothing to
compare, and the explicit call is what gives the refusal somebody to tell. Which of two conflicting
modules is the one refused depends on the order the host loaded them; the record names both either
way. Refused: a separate start call validating the whole schedule, which would need a notion of a
complete deployment that Bridge-ASM700 does not have. The condition that brings it back is a host
that needs all-or-nothing loading and cannot get it by destroying the bridge.

**`unload`** - drop its systems and observers from the schedule; call `shutdown`, under its
timeout; apply what it had asked for and not yet been given, `shutdown`'s requests included; drop
its exclusive components, each by its `on_unload` policy - `lifetime.md`, *What unload leaves
behind*; destroy the ids held ready for it and the row its settings were on; free its module
record; destroy the instance and its loaded image, which on the native backend closes the library.

**Its share of the persistent arena goes back, unless a row that is staying holds a string in
it** - `strings.md`, *It is given back when nothing points into it*. That is what keeps a
surviving shared component's string readable, like the float beside it.

**Nothing can be holding anything.** No module ever had a reference to this one - it had a query,
and a query that now matches nothing iterates zero spans. That is what dropping calls bought: unload
has no dangling anything to chase.

**`reload`** takes an artifact too, because a rebuilt module may carry a changed manifest - and
that is also how a setting changes: hand over the new `cfg`. **Storage survives, which is the entire
point, and it is the one outcome nothing else here produces.** The fingerprint decides whether it
can: a changed struct body hashes differently and is refused by name, shared or exclusive alike -
`lifetime.md`, *When a component layout changed*.

**A reload is the new instance's `reload`, and then the old one's `shutdown`.** In order: the
checks a load makes, and that the deployment allows it; a new image,
a new instance and a fresh private memory; the new instance's `reload`, which declares and
registers as `init` did and does nothing else; the schedule again, with the same refusal; then the
old instance's `shutdown`, and the swap. Until that last step the old instance has not been
touched, so a reload that fails anywhere before it leaves the module running as it was -
`failure.md`, *When something fails*.

**`reload` is not `init` run again.** `init` starts from nothing: it registers and it populates.
`reload` starts over what exists. Kept apart so that no `init` has to ask whether this is the first
time, which is Erlang's reason for giving new code an entry of its own. Refused: the original
Bridge's arrangement, where the old instance's `reload` consents and the new one runs `init` - right
there, where nothing survived a reload, and wrong here, where `init` would populate storage that is
already populated. Its veto has no job left either: a reload is between phases, so no module is in
the middle of anything. The condition that brings the veto back is a module that must be able to
refuse. Refused: one `init` told by a flag that storage survived, which is one path to keep in step
and a branch in every `init`. The condition that brings it back is authors unable to keep the two
registering the same things.

**Where two instances cannot exist at once, the old one goes first.** A process cannot hold two
copies of one library, so on a backend that says so - `capabilities.md`, *Capabilities, so the swap
is honest* - the order is the old `shutdown`, the library closed, the new one opened, its `reload`.
A failure then has no old instance to keep. **The module stays, stopped, with everything it
declared.** Its record, its components and their rows are as they were; it has no code, so it
shows as STOPPED and runs nothing; and `on_error` decides what that costs the deployment, as for
any stop. A reload with an artifact that works brings it back over the same rows, and an unload is
what ends it.

That is `lifetime.md`, *When a component layout changed*, applied here: Bridge-ASM700 never discards data on
a call that means "keep running", and `bridge_reload` is such a call. Refused: taking the module
out as an unload would, which is what this first did - whether a failed reload cost a module its
data then depended on which backend it ran on, a cleanup whose condition nobody calling it was
thinking of. The original Bridge marks a module whose reload failed as failed rather than stopped
by an operator, for the reason that holds here: what failed was the new artifact, and a better one
must be able to follow. Refused: no reload at all on such a backend, which gives up what
`capabilities.md` lists.

**What a reload may change is what is the instance's.** Fuel, limits, timeouts, the log level and
the three sizes of the private memory all arrive with the new `cfg`, as do `spawns` and the grants.
`persistent` too, upwards only: a reload that states more is given more, and one that states less
is refused by name, since what it allocated is in that share - `budgets.md`, *Budgets*. A
component the new instance
no longer declares is treated as an unload treats it. And **settings are not seeded again** -
`config.md`, *A module's own settings are a component*: after init the component is the state.

**There is no `reset` flag** - `lifetime.md`, *Lifetime*, for why. That makes `hot_reload` mean
something: it gates reload-in-place, the only thing with no alternative, rather than an outcome a
host could reach anyway.
