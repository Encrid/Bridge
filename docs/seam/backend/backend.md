# The backend surface

The surface a backend implements. Bridge-ASM700 defines it; a shared library provides it. Two exist: the
**wasm** backend, which wraps wasmtime, and the **native** backend, which opens a shared library and
calls it directly, for code that does not need sandboxing. A module names the one that runs it -
`host-api.md`, *The data*.

Bridge-ASM700 keeps everything that is not the engine's business - `split.md`, *The split* - so a backend
does not reimplement a scheduler to swap an interpreter.

What matters at this seam is the far side of that list: config, discovery and the formats are the
host's, so **a backend never has a file to find**. It is handed a module's bytes and its path,
uninspected, and the settings that are its own.

**Bridge-ASM700 opens the backend libraries its config names**, and finds the seventeen entries in each by
name - `create.md`, *Create*. That is the one kind of file Bridge-ASM700 opens.

## What a backend is asked for

    backend_create(settings)                   -> backend
    backend_destroy(backend)
    backend_capabilities(backend)              -> flags

    memory_shape(backend)                      -> reservation, guard
    memory_adopt(backend, ptr, len)            -> memory
    memory_release(memory)
    module_compile(backend, bytes, len, path, demands) -> module
    module_imports(module)                     -> names and kinds
    module_destroy(module)

    instance_create(backend, module, memories, count) -> instance
    instance_destroy(instance)
    instance_set_fuel(instance, units)
    instance_set_deadline(instance, nanos)
    instance_set_memory_limit(instance, bytes)

    define_func(instance, namespace, field, fn, sig)
    instance_func(instance, name, &arity, &results) -> func
    call(func, status)                         -> err

Seventeen entries. The original Bridge's `ModuleRuntime` trait had fourteen, four of which were
filesystem and timer work that has moved out.

**`module` at this seam is a module's loaded image**: compiled bytes on the wasm backend, an opened
library on the native one. `module_compile` takes
both the bytes and the path because Bridge-ASM700 inspects neither, and each backend uses the one it
needs.

**It also takes the capabilities this module's config demands**, as the flags of `capabilities.md`,
*Capabilities, so the swap is honest*. A guarantee can cost something on every instruction, and a
backend that knows a module declined one need not compile it in - `limits.md`, *What a bound
costs*. A backend is free to ignore them and enforce more than was asked; it is never free to
enforce less, which the load check has already settled before this is called.

`module_imports` is there for one decision: **which memories to hand `instance_create`.** Not every
module imports both - `wasm.md`, *Memories* - so Bridge-ASM700 reads the list rather than supplying
memories nobody asked for and letting the link fail.

## Memory is adopted, not created

`memory_adopt` takes pages Bridge-ASM700 already owns and asks the backend to make them addressable by a
module. It does not ask the backend to allocate. That is the whole of what "Bridge-ASM700 owns the
memory" means at this seam, and it is why the same region reaches a native module as a raw pointer
and a wasm module as an import.

**A backend states the shape it needs, and Bridge-ASM700 allocates to it.** An engine does not care
who allocated a block; it cares what surrounds it. One that enforces bounds by faulting needs
address space reserved after the block and left unmapped, so that a stray access lands on nothing.
`memory_shape` answers the reservation and the guard a backend needs; Bridge-ASM700 takes the largest
of every backend's and allocates each block once, to that. A backend with no requirement answers
zero for both.

That is what keeps the block one answer for any number of backends. Refused: one backend creating
the block and the others adopting it, which works for exactly one backend that insists on allocating
and makes "who owns the memory" depend on which backends a deployment has. The condition that brings
it back is a backend that cannot run over memory it did not allocate, whatever its shape.

There is no read-only variant and no verification pass, because access mode is not enforced - see
`data.md`, *Access mode*.

For the native backend `memory_adopt` is almost nothing - the pointer is already addressable - which
is the shape a good abstraction should have.

**`memory_release` takes the handle back**, and frees nothing of the block, which was never the
backend's. A module's private block is adopted when it loads and released when it unloads, so a
handle without a counterpart would leak once per unload. The order is Bridge-ASM700's to keep:
**a memory is released after the last instance it was handed to is destroyed**, never before, since
an instance may hold the handle for as long as it runs.

Refused: no handle at all, with `instance_create` taking a pointer and a length. On the two backends
that exist the handle is those two numbers and nothing more, so it would cost nothing today. It is
kept for the backend whose engine separates a block's backing from the object an instance sees, and
must build the first once per block. Refused: the handle dying with the instance that used it,
which is wrong for the shared memory - every instance uses it - and hides a count inside each
backend.

## The backend owns its clock

`instance_set_deadline` takes **nanoseconds**, and the backend converts to whatever its engine
counts in. Bridge-ASM700 asks in real time because that is the only unit every backend can mean the
same thing by.

It is the backend rather than the host, even though `split.md`, *The split*, sends a policy about
time the other way. The host has no handle on an engine's internals, and a host that forgot to drive
a clock would silently disable every deadline in the deployment. A backend that cannot interrupt
says so through `deadline` in `capabilities.md`, *Capabilities, so the swap is honest*, instead of
accepting a number it will ignore. How the wasm backend keeps time is `wasmtime.md`, *Deadlines*.

What is asked for is a **lifecycle call's** timeout; a system is bounded by fuel alone, for the
reason in `limits.md`, *What bounds a run*. And `instance_set_fuel` is called **before every
dispatch**, not once at load, because fuel only depletes.

## What the backend does not do

Decide when anything runs. Find a file. Know what a component, an entity or a barrier is. Own the
span layout. Choose a storage strategy. Parse a manifest.

**There is no store.** A store is wasmtime's unit of "one thread at a time" - its own header says a
store *"generally cannot be concurrently used"* while being *"cheap to create and cheap to
dispose"*, so one per instance is both idiomatic and what Bridge-ASM700 wants. The unit at this seam is
therefore the **instance**, and how a backend groups instances internally is its business: wasmtime
gives each a store, the native backend gives each nothing.

Fuel, a lifecycle call's timeout and the memory limit all move with it. They are per module in
config, and a limiter that knows which module is executing can hold co-tenants to their own
ceilings - so scoping them to one backend's concept would be wrong twice over.

**What this gives up: a backend cannot be told to group instances.** It may group them internally -
wasmtime could put several in one store - but Bridge-ASM700 has no way to ask and no reason to, since the
grouping policy would be a decision shaped by one engine. The condition that would change the answer
is a backend where instances **must** share state to be viable at all - one global heap for all of
them, say - at which point a grouping entry belongs here and the instance is the wrong unit.

The test each entry passes is that it means something to **every** backend. `module_compile` is
compile-or-dlopen; `instance_set_fuel` is set-fuel-or-say-you-cannot, which is what `meter` in
`capabilities.md`, *Capabilities, so the swap is honest*, exists to say.

So **meaningful-but-unavailable is not meaningless** - that is `instance_set_fuel`, kept.
Meaningless to one backend is a different thing, and that is `store_create`: not "cannot do it" but
"the word names nothing here", which no capability flag could rescue.

Bridge-ASM700 hands it bytes, memories and host functions, and asks it to run a function. That is the
entire relationship, and it is why the surface is seventeen entries rather than a second copy of
Bridge-ASM700.
