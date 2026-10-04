# Diagnostics

How you find out what happened: what a bad write costs and what finds it, the log an element writes,
and the counters BridgeC keeps when asked. None of it is a second way to read the design - a
diagnostic says what occurred, and state is pulled: `host-api.md`, *Introspection*.

## What a bad write costs, and what finds it

Two things get confused here. **A module writing a component another module reads is normal** - it is
the whole mechanism, and conflicting systems never run at once, so the scheduler already serialises
them. What is not normal is a module writing *outside the columns it declared*, and memory 1 cannot
prevent that.

Three things narrow it, none of them costing anything per frame:

**The suspects are the declared writers.** If a component is corrupt, the modules that declared it
`out` or `inout` are the list and the rest are cleared - from data the scheduler keeps anyway.

**The culprit is the system that was running.** Conflicting systems never run at once, so at any
instant one system can be writing a given component, and BridgeC dispatched it. One store per
dispatch, into the log - the `system` field of its record.

**The barrier is a validation point.** Nothing runs during one and BridgeC is already walking the
matched tables, so checking there is natural. idTech3 validated at its protocol boundary, always on
and always paid for; what we check always and what we check when asked are two different lists.

### Always on

**The `row < count` test the generator emits.** Not a policy: that code is compiled into the
element's own artifact, which BridgeC only ever sees as bytes, so there is no BridgeC setting that
could remove it and the generator offers none. An author who wants it gone writes their own accessor,
at which point it is their code. It is not redundant with wasm's bounds check either - wasm catches a
load outside the *memory*, and this catches one past the span's rows and still inside memory 1, which
wasm permits.

A compare and branch per access, which a compiler can usually remove because the loop bound is the
same `count` load. That is reasoning, not measurement - `PLAN.md` phase 2 is where it gets measured,
and its done-when says so.

**Row counts against table sizes.** O(matched tables), the same order as publishing, so it is not in
the expensive tier at all - a few comparisons on a walk BridgeC is already doing.

**What it catches is our own patch failing**, not flecs. The patch in *flecs will not tell you which*
exists so the mirror cannot drift; this check is what notices if the patch is wrong, against the one
mechanism whose failure is silent and sends modules to rows that are not theirs. Nearly free, and
aimed at the code we wrote rather than the library we vendored.

### The checksum pass, when asked

One check, behind `[checks] validate`. Hash each column at every barrier and compare with the last.
**If a column changed and no system that ran declared it `out`, the log names the element** - which
is what the three narrowing facts above stop short of, since they produce a suspect list and this
produces a culprit.

That matters because the person reading the log is usually not the author of the element that broke
it, so narrowing is not enough. It is also **the only thing that finds a plugin's stray write**: a
plugin has no sandbox and no generated accessor, so neither always-on check applies to it.

O(data) per barrier, which is why it is off. A runtime key rather than a build flag costs one branch
per barrier - flecs compiles its expensive tier out (`FLECS_SANITIZE`, *"will severely slow down
code"*) because its checks are scattered through hot code where a branch each would be the cost;
ours is one pass at one site, so that force does not act on us.

**Columns only, of memory 1's three regions.** The entity index has its own check above, cheaper and
always on. The persistent arena is deliberately left out: a corrupt string is visible to whoever
reads it, where a corrupt float is not, so hashing it would double the pass's cost to find the one
kind of damage that announces itself.

**What BridgeC does not check is a component's own invariants.** It knows no content -
`module-abi.md`, *It holds no content* - so an element that wants its invariants checked writes a
system that checks them. That is not a missing feature; it is the same rule that keeps
`bridge-abi.h` empty of components.

## The log

The log is BridgeC's, not an element's, and it passes the test in `DESIGN.md`, *No builtins*: anything
can fail, so every deployment needs it, and a failure nobody can report is worse than a missing
feature.

An element says things by appending to **one ring in its memory 2**, with a single writer by
construction: an element's systems are serialised - `memory.md`, *Threads* - so no line needs an
atomic. No call either, so a line costs stores rather than the 500ns a host call costs today, and
BridgeC drains every ring at each barrier.

    u32 when      barrier count, and offset within the phase
    u8  -         reserved, to keep the stride at 24
    i8  level     negative is worse, flecs' convention
    u16 system    which system was running
    u32 text      offset into this element's scratch arena, always
    u32 len
    u32 arg[2]    two inline scalars, for "something is N"

**This record never leaves BridgeC.** What the host sees is resolved - `host-api.md`, *Nothing here
is a BridgeC layout* - so the shape above is the element seam's alone and its stride is free to
change.

Fixed stride, so the ring is an array: head and tail are counters, record `i` is `base + i*24`, and the
number dropped is a subtraction. Variable-length records would need scanning, which is why idTech3's
journal is a sequential file and not a ring.

**The text is in the scratch arena, and that is not a default but the whole rule.** An offset means
nothing without a memory, and we have three - so one of them is fixed by the binding rather than
carried in the datum. That is the component model's own answer to the same problem: its canonical ABI
takes a `memory` **option**, an index decided where the function is declared, so no value has to say
which address space it came from. idTech3 and flecs never faced it, because `VM_ArgPtr` resolves
against a single `dataBase` (`vm.c:607`) and `ecs_os_api.log_` takes a real pointer.

**So a literal is copied in, and costs a `memcpy` of its length.** Writing it instead as an offset
into memory 0 would be free, and would need a sixteenth seam entry to read a memory BridgeC does not
own - paying surface for zero copies on a path whose own claim is only that a line costs stores.
Godot's alternative is available too and refused for a reason already here: it copies at the boundary
and hands back a handle - `string_new_with_utf8_chars_and_len2` - which is a call with a payload, the
crossing this ring replaced.

**That is also why the scratch arena exists**, which was never written down: it is the only memory an
element can write that **BridgeC can read**. Memory 0 is the module's own and never adopted; memory 1
is columns.

Two rules follow, and both were already here. **Drain precedes the scratch-arena reset**, because the
message lives in that arena and the barrier does both. And **BridgeC bounds-checks `text + len`** -
now against a determinate block, which is what makes the check writable at all. One compare per
record is the price.

Levels are flecs' signed convention, so flecs' own output needs no mapping - `ecs_os_api.log_` takes
`(level, file, line, msg)`, warn at -2 and err at -3. Filtering is at the writer: the generator emits
`if (level > threshold) return` against a threshold in the memory-2 header, so a line that is switched
off costs a load and a branch.

**Drop and count, never block.** A log may lose lines, and that is exactly what separates it from an
event: an event is an entity, so the entity ceiling governs it and running out is a failure where it
happens rather than a silent drop - *Observers*, and *Budgets*.

Where drained records go - a file, a console, a plugin - is BridgeC's business and config's, and
nothing about it crosses to an element.

**Nothing but log lines go through here.** Profile counters used to share this record, on the
argument that one shape beats two nearly-alike ones - and they are not a record at all now, so no
second shape appears: *The profile*, below.

## The profile

Four documents claimed something about it before any section said what it was. Two of those claims
survive and they are the specification: it reports **per system rather than per module** -
`module-abi.md`, *Exports* - and an observer cycle is visible in it, *Observers*.

    int bridge_profile(bridge *b, bridge_profile_row *out, int max);

    typedef struct {
        const char *name;     /* a system or an observer */
        uint64_t    calls;
        uint64_t    nanos;
    } bridge_profile_row;

**A pull, not records**, for the reason *The log* gives against itself: that ring is *"Drop and
count, never block"*. A dropped log line is a missing line; a dropped profile record is a **biased
number with nothing marking it biased**. State must not arrive by a route that can drop, which is
the same argument `host-api.md`, *Introspection*, makes for `bridge_elements` - this mirrors it.

**Two fields, and `calls` is earned by observers.** A system runs once per pass by construction, so
counting its calls says nothing; an observer runs once per matched span, so a count and a time are
different facts there. flecs keeps **one** metric per system - `ecs_system_stats_t.time_spent`,
`flecs.h:14813` - against sixty-one at the world level, and the discipline is the part worth
copying.

**No per-phase row**, because a phase is the sum of its systems and the host can add. That is the
rule that cut `bridge_element_info` to two fields: do not report what the caller can derive.

**And an observer cycle needs no counter of its own.** A cycle means A adds, B's observer fires, B
adds, A's fires - so both show a call count that never drops to zero, in the rows that already
exist.

**Opt-in, and nothing when off.** `DESIGN.md`, *No builtins*, uses a clock as its example of something
not universal to every deployment, and a profile needs one: a shipped game does not measure itself.
So `[checks] profile` beside `validate`, as the two things BridgeC does only when asked, with two
clock reads per dispatch as the cost when on.

**Not recorded: memory high-water per element.** Bridge's profile had it and nothing here claims it.
The condition that would add it is a deployment wanting to watch a limit be approached rather than
hit.

