# The wasm backend

The backend that runs a core wasm module in a sandbox, over wasmtime. What a backend is asked for is
`backend.md`; how a module binds to this one is `wasm.md`. This document is what is particular to
wasmtime and would be different under another engine.

## The pin

wasmtime **48.0.5**, its C API, vendored under `third_party/wasmtime/`. 48 is a long-term release,
supported for 24 months where an ordinary one gets two. This backend exists for its sandbox, so
getting security fixes outweighs matching the 47 the original Bridge runs.

## Memory

**Its shape.** wasmtime enforces a memory's bounds by faulting: it reserves address space for the
whole of a 32-bit memory and a guard region after it, and then emits loads with no check at all.
So `memory_shape` answers that reservation and that guard. A block without them would make wasmtime
check every load, on the path this design exists to keep free.

**Adoption** goes through wasmtime's host memory creator, the hook by which an embedder supplies the
pages behind a memory. The backend answers it with the block Bridge-ASM700 allocated, and one block
backs a memory in every store that imports it - each module has its own store, `backend.md`, *What
the backend does not do*.

wasmtime's own mechanism for one memory in many stores is a shared memory, and it is refused here:
it allocates its own pages, so it cannot adopt, and it brings the threads proposal with it, whose
wait instruction blocks a thread in a way neither fuel nor a deadline can interrupt - `limits.md`,
*What bounds a run*, rests on nothing a module can reach blocking. The condition that brings it back
is the creator proving unable to serve one block to several stores.

The creator is asked for every memory, a module's own included, and is not told which. The backend
knows because it asks: an adoption is in progress, or it is not.

## Deadlines

wasmtime interrupts by **epoch**: compiled code checks a counter the embedder increments, and traps
once it passes a store's deadline. A tick has no duration; whoever increments decides what time is.

So the backend keeps **one watchdog thread and no tick period**. `instance_set_deadline` arms it:
the store's deadline is one epoch ahead, the thread sleeps for the timeout, and increments once if
the call has not returned. Lifecycle calls never overlap - `lifecycle.md`, *The runtime lifecycle* -
so one armed deadline is all there ever is.

An increment reaches every store of the engine, and a store with no deadline set traps at once. So
every other call runs under a deadline too far away to be reached. That is this backend's business
and invisible at the surface: a system still has no deadline as far as Bridge-ASM700 is concerned.

Refused: a thread incrementing every tick period, which is wasmtime's usual pattern. It needs a
period somebody must choose, wakes forever with nothing armed, and rounds every timeout to a tick.
The condition that brings it back is several deadlines running at once.

## What it answers

    sandbox, meter, deadline, limit, reload        yes to all five

`capabilities.md`, *Capabilities, so the swap is honest*.
