# What a bad write costs, and what finds it

A diagnostic says what occurred. None of it is a second way to read the design, and state is
pulled: `introspection.md`, *Introspection*. The log is `log.md`; the counters are `profile.md`.

Two things get confused here. **A module writing a component another module reads is normal** - it
is the whole mechanism, and conflicting systems never run at once, so the scheduler already
serialises them. What is not normal is a module writing *outside the columns it declared*, and
the shared memory cannot prevent that.

Three things narrow it, none of them costing anything per frame:

**The suspects are the declared writers.** If a component is corrupt, the modules that declared it
`out` or `inout` are the list and the rest are cleared - from data the scheduler keeps anyway.

**The culprit is the system that was running.** Conflicting systems never run at once, so at any
instant one system can be writing a given component, and Bridge-ASM700 dispatched it. One store per
dispatch, into the log - the `system` field of its record.

**The barrier is a validation point.** Nothing runs during one and Bridge-ASM700 is already walking the
matched tables, so checking there is natural. idTech3 validated at its protocol boundary, always on
and always paid for; what we check always and what we check when asked are two different lists.

## Always on

**The `row < count` test the accessors carry.** Not a policy: that code is compiled into the
module's own artifact, which Bridge-ASM700 only ever sees as bytes, so no Bridge-ASM700 setting
could remove it and the accessors offer none. An author who wants it gone writes their own
accessor, at which point it is their code. It is not redundant with wasm's bounds check either: wasm
catches a load outside the *memory*, and this catches one past the span's rows and still inside
the shared memory, which wasm permits.

**It is a compare and branch per row, and nothing removes it.** The loop bound is the same `count`
and the test could be hoisted, but neither Binaryen nor wasmtime does. Over the tightest loop a
system can have, one float read, changed and written back per row of 200,000:

                         with the test    without
    no fuel              0.39 ns a row    0.33      x1.2
    fuel                 0.66             0.44      x1.5

Under fuel it costs more than itself, because the branch it adds is a block and fuel is counted by
the block - `limits.md`, *What a bound costs*. `tests/phase3.c`'s `bench` case is the measurement,
and the build without the test exists only for it.

It stays, at that price: a sixth of a tight loop buys the one check that catches a module reading
rows that are not its span's. The condition that reopens it is a workload where that sixth is the
difference, and the answer then is an accessor that tests once per span rather than once per row.

**Row counts against table sizes.** O(matched tables), the same order as publishing, so it is not in
the expensive tier at all - a few comparisons on a walk Bridge-ASM700 is already doing.

**What it catches is our own patch failing**, not flecs. The patch in `flecs.md`, *flecs will not
tell you which*, exists so the mirror cannot drift. This check notices if the patch is wrong,
against the one mechanism whose failure is silent and sends modules to rows that are not theirs.
Nearly free, and aimed at the code we wrote rather than the library we vendored.

## The checksum pass, when asked

One check, behind `[checks] validate`. Hash each column at every barrier and compare with the last.
**If a column changed and no system that ran declared it `out`, the log names the module** - which
is what the three narrowing facts above stop short of: they produce a suspect list, and this
produces a culprit.

That matters because the person reading the log is usually not the author of the module that broke
it, so narrowing is not enough. It is also **the only thing that finds a native module's stray write**:
it has no sandbox and need not use the generated accessor, so the `row < count` test does not reach
it.

O(data) per barrier, which is why it is off. A runtime key rather than a build flag costs one branch
per barrier. flecs compiles its expensive tier out (`FLECS_SANITIZE`, *"will severely slow down
code"*) because its checks are scattered through hot code where a branch each would be the cost;
ours is one pass at one site, so that force does not act on us.

**Columns only, of the shared memory's three regions.** The entity index has its own check above,
cheaper and always on. The persistent arena is deliberately left out: a corrupt string is visible to
whoever reads it, where a corrupt float is not, so hashing it would double the pass's cost to find
the one kind of damage that announces itself.

**What Bridge-ASM700 does not check is a component's own invariants.** It knows no content -
`wasm.md`, *It holds no content* - so a module that wants its invariants checked writes a
system that checks them. That is not a missing feature; it is the same rule that keeps
`bridge-abi.h` empty of components.
