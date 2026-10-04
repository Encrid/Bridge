# Phase 2: one module runs

**Draft.** Nothing here is settled until the done-when holds.

The first Bridge-ASM700 code. It produces **the backend seam and the wasm backend behind it**, the
part of the host seam needed to load one module through them, and the tests that show a module
running, failing and being refused. No storage, no systems, no phases: those are phase 3.

What is settled and not to be re-argued here: the sixteen entries and what each means -
`backend.md`, *What a backend is asked for*; signatures - `signatures.md`; capabilities -
`capabilities.md`; what `bridge_create` and `bridge_load` take and refuse - `host-api.md`,
`create.md`, `lifecycle.md`; what bounds a module - `limits.md`; the manifest - `module.md`, *The
manifest*.

## Decided for this phase

**wasmtime 48.0.5, vendored.** 48 is a long-term release, supported for 24 months where an ordinary
one gets two; the wasm backend exists for its sandbox, so security fixes outweigh matching the 47 the
original Bridge runs. Its C API has every call this phase needs, and the header lines `backend.md`
and `signatures.md` cite are the same in it. The official C API archive is unpacked under
`third_party/wasmtime/`: headers, the shared library and its import library, per platform.

**Test modules are wasm text.** Each case is a few instructions, a second memory needs nothing
extra, and `wasm-tools parse` assembles it, manifest section included. The compiled route - C, then
Binaryen - is phase 3's, with the generator: `wasm.md`, *A compiled module reaches them through
accessors*.

## What it produces

    include/bridge-backend.h      the sixteen entries, backend_sig, the capability flags
    include/bridge.h              begins: bridge_create, bridge_destroy, bridge_load,
                                  their structs, bridge_record and the sink
    include/bridge-abi.h          begins: the ABI version, the import names
    src/                          the library: create, load, records, opening a backend
    backends/wasm/                the wasm backend, a shared library over wasmtime
    third_party/wasmtime/         48.0.5 C API, with a README: version, hashes, licence
    tests/modules/*.wat           one module per case
    tests/phase2.c                a host: creates, loads, reads memory, collects records

`wasm-tools` joins Python as something the test build needs.

Every backend entry is implemented in the wasm backend now, since each is a thin call into wasmtime.
Bridge-ASM700 calls only what this phase's claim needs: `instance_set_fuel` exists and is first
called in phase 3, with the first system.

## Open questions

One, and it is the first code of the phase.

**Whether one block can back a memory in two wasmtime stores at once.** Bridge-ASM700 allocates the
shared block to the shape the backend states, and the wasm backend hands it to wasmtime through the
host memory creator - `backend.md`, *Memory is adopted, not created*, and `wasmtime.md`, *Memory*.
wasmtime's documentation describes creator memory as owned by the instance it was made for, and says
nothing about serving the same pages twice. Each module has its own store, and from phase 3 two
modules import one shared memory, so this must hold. Settled by `tests/adopt.c`: two stores on two
threads write disjoint ranges of one block, and a stray access traps with the process alive.

If it does not hold, the fallback is wasmtime's shared memory, which `wasmtime.md` refuses and
states the cost of. That is a decision for the method again, not a step of this phase.

Closed while planning, each in the document it belongs to: who runs the clock - `wasmtime.md`,
*Deadlines*; where a wrong export shape is caught - at `instance_func`, as `signatures.md` already
says; and what `bridge_create` needs before phases exist - nothing: the phase list and its refusal
arrive together in phase 3.

## Steps

1. **Vendor wasmtime**, hash-checked, with its README.
2. **`bridge-backend.h`**, and a wasm backend that creates and destroys.
3. **The open question**, as `tests/adopt.c` against the backend alone: adopt a block, instantiate
   a module that imports it, write from wasm, read from the host; then the same block in a second
   store on a second thread; then a stray access. Nothing else is written until this passes.
4. **The rest of the sixteen**: compile, imports, instance, limits, `define_func`, `instance_func`,
   `call`.
5. **`bridge.h`**: create opens the backend library and finds its entries by name; the sink and
   Bridge-ASM700's own records; load checks identity, ABI version and capabilities, then runs `init`.
6. **The cases**, one module each: writes shared memory; reads out of bounds; never returns from
   `init`; grows past its limit; exports `init` with a parameter; declares another ABI version.
7. **ctest**, with each refusal checked by the record it emits, not by a return code alone.

## Done when

`PLAN.md` states it. In practice: the host reads the bytes a module wrote; the three failures each
come back as an error with the process alive and a record naming the module; the four refusals each
emit a record naming the cause; and the two-store test passes or has changed the design.

## What would make us stop

If one block cannot back a memory in two stores without a copy or a bounds check per load, the
shared memory is not what `wasm.md` says it is, and that is a design question rather than a detail
of this phase. The options then are two the documents already refuse: one store for every module,
which `backend.md` declines and which costs parallelism, or blocks mapped into each module's own
memory, which `wasm.md` declines.
