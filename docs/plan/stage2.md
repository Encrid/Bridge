# Stage 2: one module runs

Done. The `linux` preset and the POSIX half of `src/os.c` were first run after stage 5, and passed
as written.

The first Bridge-ASM700 code. It produced **the backend seam and the wasm backend behind it**, the
part of the host seam needed to load one module through them, and the tests that show a module
running, failing and being refused. No storage, no systems, no phases: those are stage 3.

What each piece is and why is in the documents it belongs to: `backend.md`, `wasmtime.md`,
`host_api.md`, `create.md`, `lifecycle.md`, `limits.md`. This document is what the stage built and
what it found on the way.

## What it produced

    include/bridge-backend.h      the sixteen entries, backend_sig, the capability flags
    include/bridge.h              begins: bridge_create, bridge_destroy, bridge_load,
                                  their structs, bridge_record and the sink
    include/bridge-abi.h          begins: the ABI version, the import names
    src/bridge.c                  create, load, records
    src/backend.c                 opening a backend library, finding its entries by name
    src/os.c                      address space, libraries, one thread and one event
    backends/wasm/wasm.c          the wasm backend, a shared library over wasmtime
    third_party/wasmtime/         48.0.5 C API, with a README: version, hashes, licence
    tests/modules/*.wat           one module per case, assembled by wasm-tools
    tests/adopt.c                 the backend alone: the memory question, and a host function
    tests/stage2.c                a host that links the library and nothing else

`wasm-tools` joined Python as something the test build needs.

## The tests

Fifteen new ones, beside stage 1's five. Every refusal is checked by the record it emits and by the
process still being there to print "alive" afterwards.

    adopt             one block: the host reads what a module wrote; two stores on two
                      threads fill it at once and each sees the other's half; a load past
                      it traps and the store still runs; a host function is called
    stage2-ok         a module loads
    stage2-nolimit    "no limit" is a value, and a module stating it loads
    stage2-oob        init reads out of bounds:      "trapped: out of bounds memory access"
    stage2-spin       init never returns:            "trapped: interrupt", after its timeout
    stage2-grow       init grows past its limit:     "its init answered 7"
    stage2-big        its own memory exceeds the limit: "cannot instantiate"
    stage2-shape      init takes an argument:        "its export 'init' is not () -> i32"
    stage2-noinit     no init:                       "no exported function 'init'"
    stage2-abi        another ABI version:           "it wants ABI 2 and this is ABI 1"
    stage2-nobackend  an unconfigured backend:       "no backend named 'native'"
    stage2-unset      a key left at zero:            "memory_limit is unset"
    stage2-stop       on_error = fail stops the bridge; skip does not
    stage2-nolib      a backend library that will not open: create refuses, with a record
    stage2-nosink     no sink: create refuses

## What the stage found

**One block does back a memory in two stores.** The question the stage opened with. Two stores on
two threads each wrote a page of one block two hundred times, byte by byte; both pages came out
whole, a third read found the other store's bytes, and a load one byte past the block trapped by
faulting on the reservation. So Bridge-ASM700 allocates, every backend adopts, and no shared memory
or threads proposal is needed - `backend.md`, *Memory is adopted, not created*.

**But not the way wasmtime's header says.** The host memory creator is not asked for the memory it
is documented for, and is asked for another: `wasmtime.md`, *Memory*, has which, and the holder
module that follows from it. Found by the test printing nothing where the creator should have
spoken, and confirmed in wasmtime's source.

**Storage is not handed to a module yet.** `bridge_load` gives a module no memories, because the
shared and private blocks arrive with `[budget]` in stage 3. A module that imports one fails to
instantiate, by name, which is the right refusal until then. The stage's claim that a module writes
the shared memory and the host reads it is `tests/adopt.c`, against the backend alone.

**A refusal this stage cannot test.** "A capability the backend lacks" needs a backend that lacks
one. The check is written and has never run false; stage 4's native backend is where it does.

**Two things about wasmtime's C API worth knowing.** `wasmtime_error_delete` crashes on NULL, though
every call that can fail returns NULL on success. And a trap's message leads with a backtrace and
ends with the reason, so the backend reports the part after "wasm trap: ".

## Open questions

None for this stage. Three were raised for the next and wait in `stage3.md`.

## What would make us stop

It did not happen: the block backs two stores with no copy and no check on a load. It stays the
condition for the next wasmtime: adoption rests on how 48.0.5 treats a memory an instance defines,
which its documentation does not promise. `tests/adopt.c` is what says whether an upgrade kept it.
