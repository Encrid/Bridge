# The wasm backend

What was weighed here and refused is `wasmtime_refused.md`, *What was refused: The wasm backend*.

The backend that runs a core wasm module in a sandbox, over wasmtime. What a backend is asked for is
`backend.md`; how a module is written for this one is `wasm.md`. This document is what is particular to
wasmtime and would be different under another engine.

## The pin

wasmtime **48.0.5**, its C API, vendored under `backends/wasm/wasmtime/`. 48 is a long-term release,
supported for 24 months where an ordinary one gets two. This backend exists for its sandbox, so
getting security fixes for that long is what decides the pin.

## Memory

**Its shape.** wasmtime enforces a memory's bounds by faulting: it reserves address space for the
whole of a 32-bit memory and a guard region after it, and then emits loads with no check at all.
So `memory_get_shape` answers that reservation and that guard. A block without them would make wasmtime
check every load, on the path this design exists to keep free.

**Adoption** goes through wasmtime's host memory creator, the hook by which an embedder supplies the
pages behind a memory. The backend answers it with the block Bridge allocated, and one block
backs a memory in every store that imports it - each module has its own store, `backend.md`, *What
the backend does not do*. `tests/adopt` is the proof: two stores on two threads write one block
at once, each sees the other's bytes, and a load past it traps.

**Through a holder module, because the creator is asked less than its header says.** wasmtime's C
API documents the creator as used "when creating host Memory objects". In 48.0.5 it is not: a memory
the host creates is allocated by wasmtime's own default. The creator is asked only for a memory an
*instance defines*. So the backend builds, per adopted block, a module of a few bytes that defines
one memory of that size and exports it, instantiates it in the store, and hands the export to the
module that imports it. Nothing in it runs.

**That module is built once for a block, and kept in the block's handle.** `memory_adopt`
answers the handle, the first instance to import the block under an engine compiles the holder
module, and every later one instantiates what is there; `memory_release` deletes it. It was
compiled again for every instance: 14.5 us each, measured, twice a load. A module is compiled by
an engine, and this backend has up to four - *One engine per demand* - so the handle keeps one for
each that has used the block.

That rests on behaviour wasmtime does not document, so it is what an upgrade must re-prove, and the
test above is how.

**The block is named under the backend's lock.** wasmtime asks its creator for every linear
memory and is not told which, so the backend says which block the next one is, makes the holder's
instance, and unsays it. Two threads may each be making an instance of one backend, and one that
unsaid it between the other's saying and wasmtime's asking would leave that other a memory of
wasmtime's own, whose writes reach nobody. So the lock is held from the holder being found to its
memory being made. `tests/adopt` makes two stores on two threads at once, and with six copies of
it running one in some tens lost that race when the lock was not held.

wasmtime's own mechanism for one memory in many stores is a shared memory, and it is refused here:
it allocates its own pages, so it cannot adopt, and it brings the threads proposal with it, whose
wait instruction blocks a thread in a way neither fuel nor a deadline can interrupt - `limits.md`,
*What bounds a run*, rests on nothing a module can reach blocking. The condition that brings it back
is the creator no longer being asked for a memory an instance defines.

**Copy-on-write initialisation is off.** Where the system offers it wasmtime lays a module's data
segments down as a mapped image rather than copying them, and asserts that the memory under the
image is one it mapped itself. No memory here is: all of them come from the creator. Left on, a
module whose data happens to fit an image aborts the process at instantiation - on Linux and not on
Windows, which has no such images, so the first Linux run over such a module is
what found it. `tests/modules/wasm/image.wat` is a module that would.

The creator is asked for every memory, a module's own included, and is not told which. The backend
knows because it asks: an adoption is in progress, or it is not.

## Deadlines

wasmtime interrupts by **epoch**: compiled code checks a counter the embedder increments, and traps
once it passes a store's deadline. A tick has no duration; whoever increments decides what time is.

So the backend keeps **one watchdog thread and no tick period**. `instance_set_timeout` arms it:
the store's deadline is one epoch ahead, the thread sleeps for the timeout, and increments once if
the call has not returned. Lifecycle calls never overlap - `lifecycle.md`, *The runtime lifecycle* -
so one armed deadline is all there ever is.

**The watchdog looks; it does not count.** What it acts on is state under a lock: which timed call
is running, and the moment it is out of time. An event only tells it to look again. The first
version had two events, one for a call starting and one for it ending, and waited for each in
turn - which is counting them, and an event set twice before it is waited for once is one event.
Two timed calls back to back, a `reload` and then a `shutdown`, lost one: the thread sat waiting
for an end that had already been said, and the backend could not be destroyed. It takes two
such calls in a row, and one run in some hundreds, to lose that race. `tests/adopt` makes twenty
thousand.

An increment reaches every store of the engine, and a store with no deadline set traps at once. So
every other call on that engine runs under a deadline too far away to be reached. That is this
backend's business and invisible at the surface: a system still has no deadline as far as
Bridge is concerned.

## One engine per demand

Fuel and epoch interruption are settings of a wasmtime **engine**, fixed before anything is
compiled, and each puts code in every loop - `limits.md`, *What a bound costs*, has the numbers. So
the backend keeps up to four engines, one per combination of `work_limit` and `timeout`, each made the
first time a module demanding that combination is compiled. A deployment where every module agrees
has one.

A module compiled without epoch checks cannot be interrupted, so its store sets no deadline and the
watchdog increments only the engine of the call it was armed for.

## What it answers

    untrusted, work_limit, timeout, memory_limit, reload    yes
    side by side                                   yes: two instances of a module at once
    unload                                         no: its library stays in the process

`capabilities.md`, *Capabilities, so the swap is honest*.
