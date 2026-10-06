# The profile

What was weighed here and refused is `profile_refused.md`, *What was refused: The profile*.

It reports **per system rather than per module** - `wasm.md`, *Exports* - and an observer
cycle is visible in it, `schedule.md`, *An observer runs at the barrier, one hop per barrier*.

    int bridge_profile(bridge *b, bridge_profile_row *out, int max);

    typedef struct {
        const char *name;     /* a system's, an observer's, or a phase's for its barrier */
        uint64_t    calls;
        uint64_t    nanos;
        uint8_t     kind;     /* a system, an observer or a barrier: zero is none of them */
        uint8_t     pad[7];
    } bridge_profile_row;

It is not built: `plan.md`, stage 14. What is here is what that stage builds.

**A pull, not records**, for the reason `log.md`, *The log* gives against itself: that ring is *"Drop and
count, never block"*. A dropped log line is a missing line; a dropped profile record is a **biased
number with nothing marking it biased**. State must not arrive by a route that can drop, which is
the same argument `introspection.md`, *Introspection*, makes for `bridge_list_modules`.

**Two counters, and `calls` is earned by observers.** A system runs once per pass by construction, so
counting its calls says nothing; an observer runs once per barrier that triggered it, so a count
and a time are different facts there. flecs keeps **one** metric per system -
`ecs_system_stats_t.time_spent`, `flecs.h:14813` - against sixty-one at the world level, and the
discipline is the part worth copying.

**A row for each phase's barrier, and none for the phase.** What a barrier does is in no
system's row: it applies the changes that were queued, tops up ids, republishes spans, counts
failures and drains every module's log. An attach costs 30 to 40 ns there, so a phase that
queues a hundred thousand spends three or four milliseconds in a place the other rows do not
show. So each phase has one more row, for its barrier: how many ran, and how long they took. A
row says which of the three it is.

The phase itself has none, because a host can time the call it makes; the same rule keeps
`bridge_module_info_t` at two fields: do not report what the caller can derive. It cannot be had
by adding the rows up, though, which is what this said: systems of different modules run at
once, so four of a millisecond on four threads are a millisecond, and the barrier was in none
of them.

**And an observer cycle needs no counter of its own.** A cycle means A adds, B's observer fires, B
adds, A's fires - so both show a call count that never drops to zero, in the rows that already
exist.

**Opt-in, and nothing when off.** `no_builtins.md`, *No builtins*, uses a clock as its example of
something not universal to every deployment, and a profile needs one: a shipped game does not
measure itself. So `[checks] profile` beside `validate`, as the two things Bridge-ASM700 does only when
asked, with two clock reads per dispatch as the cost when on.

**How full each budget is, is asked for the same way, and is not built yet.** Every size a
deployment states is a guess until something says how near it came: `memory.md`, *Where the
regions live*, calls `spans` a key "a deployment is expected to correct", and nothing tells it
what to correct it to but the refusal when it runs out. So a second pull, beside this one: for
the bridge, what is in use and the most that ever was against each `[budget]` key; for each
module, the same against each of its own. They are numbers the library keeps already, with a
mark kept where it kept only the present value. `plan.md`, stage 14.

It is a pull for the reason the profile is. What to do with the numbers - print them, and when -
is the host's.