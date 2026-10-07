# Create

What was weighed here and refused is `create_refused.md`, *What was refused: Create*.

`bridge_create` is `Com_Init` minus the filesystem: take the config, open the backends it names,
size the shared block, and build the phase table. It loads no module - the host does that.

**Opening a backend** is opening its library, finding the eighteen entries of `backend.md`, *What a
backend is asked for*, by name, and calling `backend_create` with that backend's settings.

**A library named by a path is looked for there, and so is what it needs.** A backend brings its
runtime beside itself, and a host's executable is somewhere else. Windows searches beside the
executable unless told otherwise, so it is told. A bare name, with no directory in it, is left to
the operating system's own search on every platform. The same holds for a native module's library.

It returns null only when the **deployment** cannot start: a host of another seam's number; a
key of the deployment left unset; **no backend at all**, which leaves nothing to run a module, or
**no phases at all**; a backend or a phase with no name, or named twice; an `[order]` pair that
is malformed or contradicts another; a budget that cannot be satisfied; a backend whose library
cannot be opened, lacks an entry, or will not start; or a null `sink`.

**Which of those, the host learns from the sink** - it was handed over as an argument, so Bridge-ASM700
emits the record naming the phase or the pair before it returns. The two failures no record can
explain are a null `sink` and a null deployment, and a host that passed null already knows.

## Destroy

`bridge_destroy` calls every loaded module's `shutdown`, each under its own timeout, and then takes
everything down. A `shutdown` that fails is logged and the rest go on - `failure.md`, *When
something fails*. They run in the order the modules loaded: nothing here depends on anything else,
so no other order would mean more, and the original Bridge's reverse order served modules that
called each other.

**It answers a status, because one thing can refuse it.** A pinned module's `shutdown` is that
module's code, and runs on the thread that loaded it or not at all - `pinned.md`, *Threads*. Called
from another thread with a pinned module loaded, `bridge_destroy` answers non-zero with a record
naming the module, and has torn nothing down: the bridge is whole, to be destroyed from the thread
that can. It is the refusal `bridge_run_phase` makes, made before anything is touched.

**A backend's library is closed only if the backend says it can be** - `capabilities.md`,
*Capabilities, so the swap is honest*. The wasm backend does not: it brought wasmtime into the
process, which keeps threads of its own and a handler registered with the operating system, and
offers no call that stops either. Taken out from under them and loaded again by the next
`bridge_create`, it crashed about one run in sixty of a test that makes three bridges in a row,
and only on a busy machine. So its library stays for the life of the process, and the next bridge
finds it there. A native module's library is another matter: it is closed at unload and at
reload, which is what reload is.

## The sink

`sink` is why an embedded host needs more than idTech3's three functions. Bridge-ASM700 emits records and
**never writes a file**; a host writes them, and one with its own logging takes them as
records.

**It is mandatory**, which is the log's own argument - `log.md`, *The log* - applied one
level out: a library that can fail unreportably is worse than one that refuses to start. Refusing
costs nothing here, since the decision is made before anything has been loaded.

Four properties, all of which fall out of where the drain happens rather than being choices:

    one call per record    the call is native to native, so batching saves no
                           crossing and an array would need a lifetime rule
    on the calling thread  the drain is at a barrier, inside bridge_run_phase, after
                           parallel systems have joined. a sink needs no lock
    text for this call     the scratch arena resets right after the drain, so a
                           formatted message is gone when the call returns. a host
                           that keeps a line copies it
    identity may be null   Bridge-ASM700 emits its own records too - a denied grant, a
                           refused load - and those belong to no module

**And there is no `bridge_error`.** A second route to the same answer is a second thing to keep
true, and it would hand back a string where `introspection.md`, *Introspection*, refuses to let a
host learn state from diagnostics. A failed call answers with its return code; what is running answers with
`bridge_list_modules`; why is a record.
