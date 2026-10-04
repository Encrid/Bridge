# The profile

It reports **per system rather than per module** - `module-abi.md`, *Exports* - and an observer
cycle is visible in it, `data.md`, *Observers*.

    int bridge_profile(bridge *b, bridge_profile_row *out, int max);

    typedef struct {
        const char *name;     /* a system or an observer */
        uint64_t    calls;
        uint64_t    nanos;
    } bridge_profile_row;

**A pull, not records**, for the reason `log.md`, *The log* gives against itself: that ring is *"Drop and
count, never block"*. A dropped log line is a missing line; a dropped profile record is a **biased
number with nothing marking it biased**. State must not arrive by a route that can drop, which is
the same argument `introspection.md`, *Introspection*, makes for `bridge_elements`.

**Two fields, and `calls` is earned by observers.** A system runs once per pass by construction, so
counting its calls says nothing; an observer runs once per matched span, so a count and a time are
different facts there. flecs keeps **one** metric per system - `ecs_system_stats_t.time_spent`,
`flecs.h:14813` - against sixty-one at the world level, and the discipline is the part worth
copying.

**No per-phase row**, because a phase is the sum of its systems and the host can add. The same rule
keeps `bridge_element_info` at two fields: do not report what the caller can derive.

**And an observer cycle needs no counter of its own.** A cycle means A adds, B's observer fires, B
adds, A's fires - so both show a call count that never drops to zero, in the rows that already
exist.

**Opt-in, and nothing when off.** `no-builtins.md`, *No builtins*, uses a clock as its example of
something not universal to every deployment, and a profile needs one: a shipped game does not
measure itself. So `[checks] profile` beside `validate`, as the two things BridgeC does only when
asked, with two clock reads per dispatch as the cost when on.

**Not recorded: memory high-water per element.** Bridge's profile had it and nothing here claims it.
The condition that would add it is a deployment wanting to watch a limit be approached rather than
hit.
