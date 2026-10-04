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

**`unload`** - drop its systems and observers from the schedule; call `shutdown`; drop its exclusive
components, each by its `on_unload` policy - `lifetime.md`, *What unload leaves behind*; free its
module record; destroy the instance and its
loaded image, which on the native backend closes the library.

**Its persistent-arena space is not reclaimed** - `strings.md`, *It is never reclaimed*. That is what
keeps a surviving shared component's string readable, like the float beside it.

**Nothing can be holding anything.** No module ever had a reference to this one - it had a query,
and a query that now matches nothing iterates zero spans. That is what dropping calls bought: unload
has no dangling anything to chase.

**`reload`** takes an artifact too, because a rebuilt module may carry a changed manifest - and
that is also how a setting changes: hand over the new `cfg`. **Storage survives, which is the entire
point, and it is the one outcome nothing else here produces.** The fingerprint decides whether it
can: a changed struct body hashes differently and is refused by name, shared or exclusive alike -
`lifetime.md`, *When the layout changed*.

**There is no `reset` flag** - `lifetime.md`, *Lifetime*, for why. That makes `hot_reload` mean
something: it gates reload-in-place, the only thing with no alternative, rather than an outcome a
host could reach anyway.
