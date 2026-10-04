# The runtime lifecycle

**Legal only between phases**, never while `bridge_phase` is executing. All three restructure
storage and the schedule, which is what a barrier makes safe. A plugin holding the pointer could
break this; nothing contains a plugin, so it is a contract rather than a guarantee.

**`load`** takes an artifact rather than a path, because BridgeC decides on `cfg` and never opens
anything. In order: check the ABI version, check `trusted` against the backend's `sandbox`, check
the grants, take a per-element budget slot, hand `bytes` or `path` to the backend, call `init` -
which calls `declare`, `system` and `observe` - recompute the conflict graph and
**refuse-and-unwind if a new system or observer conflicts unordered with an existing one**, then
publish spans.

**Read, then decide, then load** survives the host owning the filesystem: the identity arrives as
data in `cfg`, so the decision is made with the artifact untouched.

That refusal is the startup rule generalised. At create it refuses to start; here it refuses the
load, and the explicit call is what gives it somebody to tell.

**`unload`** - drop its systems and observers from the schedule; call `shutdown`; drop its exclusive
components, `data.md`, *Structural change*; free its element record; destroy the instance or
`dlclose`.

**Its persistent-arena space is not reclaimed** - `strings.md`, *It is never reclaimed*. That is what
keeps a surviving shared component's string readable, like the float beside it.

**Nothing can be holding anything.** No element ever had a reference to this one - it had a query,
and a query that now matches nothing iterates zero spans. That is what dropping calls bought: unload
has no dangling anything to chase.

**`reload`** takes an artifact too, because a rebuilt element may carry a changed manifest - and
that is also how a setting changes: hand over the new `cfg`. **Storage survives, which is the entire
point, and it is the one outcome nothing else here produces.** The fingerprint decides whether it
can: a changed struct body hashes differently and is refused by name, shared or exclusive alike -
`lifetime.md`, *When the layout changed*.

**There is no `reset` flag.** Dropping this element's data and starting it over is `unload` then
`load`, which is the same sequence under a verb that says so. It also makes `hot_reload` mean
something: it gates reload-in-place, the only thing with no alternative, rather than an outcome a
host could reach anyway.
