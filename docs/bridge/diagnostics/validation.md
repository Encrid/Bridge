# What a bad write costs, and what finds it

What was weighed here and refused is `validation-refused.md`, *What was refused: What a bad write
costs, and what finds it*.

A diagnostic says what occurred. None of it is a second way to read the design, and state is
pulled: `introspection.md`, *Introspection*. The log is `log.md`; the counters are `profile.md`.

Two things get confused here. **A module writing a component another module reads is normal** - it
is the whole mechanism, and conflicting systems never run at once, so the scheduler already
serialises them. What is not normal is a module writing *outside the columns it declared*, and
the shared memory cannot prevent that.

Three things narrow it, none of them costing anything per frame:

**The suspects are the declared writers.** If a component is corrupt, the modules that declared it
written are the list and the rest are cleared - from data the scheduler keeps anyway.

**The culprit is the system that was running.** Conflicting systems never run at once, so at any
instant one system can be writing a given component, and Bridge-ASM700 dispatched it. One store per
dispatch, into the log - the `system` field of its record.

**The barrier is a validation point.** Nothing runs during one, so what is checked there is checked
against a world that is not moving. idTech3 validated at its protocol boundary, always on
and always paid for; what we check always and what we check when asked are two different lists.

## Always on

Nothing of Bridge-ASM700's is on this list any more: what follows is the wasm backend's SDK, and
it is stated where it had been always on.

### The row test

**The wasm SDK's accessors can test `row < count` before every load, and a module's author
states whether they do.** A module is compiled with `BRIDGE_ROWS_CHECKED` or with
`BRIDGE_ROWS_UNCHECKED`, and with neither, or both, it does not compile. It is not Bridge-ASM700's
to state: that code is in the module's own artifact, which Bridge-ASM700 only ever sees as bytes.
And it is not redundant with wasm's bounds check: wasm catches a load outside the *memory*, and
this catches one past the span's rows and still inside the shared memory, which wasm permits.

**What it protects is the author, from an index of their own that is wrong.** It is no part of
the sandbox: a wasm module reads and writes every byte of the shared memory with a plain load,
and access is not enforced - `data.md`, *Access mode*. The native SDK hands out real pointers and
tests nothing.

**It is a compare and branch per row.** The loop bound is the same `count`
and the test could be hoisted, but neither Binaryen nor wasmtime does. Over the tightest loop a
system can have, one float read, changed and written back per row of 200,000:

                         with the test    without
    no work limit        0.39 ns a row    0.33      x1.2
    a work limit         0.65             0.44      x1.5

Under a work limit it costs more than itself, because the branch it adds is a block and the wasm
backend counts work by the block - `limits.md`, *What a bound costs*. `tests/stage3.c`'s `bench`
case is the measurement: one module, built both ways.

**The entity index needs no check of its own.** It did while flecs kept the rows: flecs moved a
row and Bridge-ASM700 had to be told, by a patch whose failure would have been silent, so every
barrier compared row counts against table sizes. Storage now writes a record at the place in the
code where it places the row - `reference.md`, *Reference following* - and reads none of it back,
so there is no second account to compare. `tests/storage.c`, case `stress`, is where that is held:
four modules keep their own account of eight million changes and check every entity through the
index, and every component through its spans, at every pass.

## The checksum pass, when asked

One check, behind `[checks] validate`. Hash each column at every barrier and compare with the last.
**If a column changed and no system that ran declared it written, the log names the module** - which
is what the three narrowing facts above stop short of: they produce a suspect list, and this
produces a culprit.

That matters because the person reading the log is usually not the author of the module that broke
it, so narrowing is not enough. It is also **the only thing that finds a native module's stray write**:
it has no sandbox and need not use the accessor, so the `row < count` test does not reach
it.

O(data) per barrier, which is why it is off. A runtime key rather than a build flag costs one branch
per barrier. flecs compiles its expensive tier out (`FLECS_SANITIZE`, *"will severely slow down
code"*) because its checks are scattered through hot code where a branch each would be the cost;
ours is one pass at one site, so that force does not act on us.

**Columns only, of the shared memory's three regions.** The entity index is written where a row is
placed and never read back, so there is no second account of it to compare. The persistent arena is
deliberately left out: a corrupt string is visible to whoever reads it, where a corrupt float is
not, so hashing it would double the pass's cost to find the one kind of damage that announces
itself.

**What Bridge-ASM700 does not check is a component's own invariants.** It knows no content -
`wasm.md`, *It holds no content* - so a module that wants its invariants checked writes a
system that checks them. That is not a missing feature; it is the same rule that keeps
`bridge-abi.h` empty of components.
