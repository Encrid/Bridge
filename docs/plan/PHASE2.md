# Phase 2: one module runs

**Draft.** Nothing here is settled until the done-when holds.

The first Bridge-ASM700 code. It produces **the backend seam and the wasm backend behind it**, the
part of the host seam needed to load one module through them, and the tests that show a module
running, failing and being refused. No storage, no systems, no phases: those are phase 3.

What is settled and not to be re-argued here: the fifteen entries and what each means -
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

    include/bridge-backend.h      the fifteen entries, backend_sig, the capability flags
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

Settled by trying, in this order. The first is the one that could send the phase back to the method.

**1. How a block Bridge-ASM700 owns becomes a memory a module imports.** wasmtime offers a host
memory creator, `wasmtime_config_host_memory_creator_set`: the engine asks a callback for every
linear memory and uses the pages it is given. Three things are unknown until tried:

- whether the same block can back a memory in **two stores at once**. `backend.md` gives each
  instance its own store, and from phase 3 two modules import one shared memory. wasmtime's own
  answer to sharing across stores is a `shared` memory, which brings the threads proposal with it.
- what wasmtime requires of the pages. It asks for a reservation and a guard region; pages without
  them make it emit an explicit bounds check on every load, on the path this design exists to keep
  free. The alternative is Bridge-ASM700 reserving address space around its blocks.
- how the callback tells an adopted memory from a module's own, since it is asked for both and is
  told neither which.

A two-store test is part of this phase for that reason, though two modules are phase 3's claim:
`memory_adopt` is this phase's entry, and its shape depends on the answer.

**2. Who runs the clock.** `instance_set_deadline` takes nanoseconds and wasmtime counts epochs, so
the backend runs a timer that increments the epoch - `backend.md`, *The backend owns its clock*.
Whether that is one thread per backend, and how it stops, is not written anywhere.

**3. Where a refusal is detected.** An export of the wrong shape is refused "at load"; whether
`instance_func` can see a shape before instantiation or only after is wasmtime's to say.

**4. What `bridge_create` needs before there are phases.** `create.md` refuses a config with no
phases, and phases arrive with `bridge_phase` in phase 3. This phase either carries the phase list
unused or adds that refusal with the thing it protects.

## Steps

1. **Vendor wasmtime**, hash-checked, with its README.
2. **`bridge-backend.h`**, and a wasm backend that creates and destroys.
3. **Open question 1**, as a test against the backend alone: adopt a block, instantiate a module
   that imports it, write from wasm, read from the host; then the same block in a second store.
4. **The rest of the fifteen**: compile, imports, instance, limits, `define_func`, `instance_func`,
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
