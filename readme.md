# Bridge

Bridge is a C library that loads and runs modules over shared ECS storage.

## Background

Bridge is for a program that gains its features from modules at run time, such as a moddable or hostable application or game.
A host links Bridge and owns `main`, the loop and each file.
A host gives `bridge_create` a deployment, and gets a world: the storage, the modules loaded onto it, and the threads that run them.
The world stays until `bridge_destroy`. A process can have more than one world, and each world is separate.

The three layers are a host, Bridge and the modules. A module runs on a backend, for example native code or WebAssembly.
`docs/extern/lib/bridge.h.md` tells how a host uses Bridge.
`docs/extern/lib/bridge_abi.h.md` and `docs/extern/lib/bridge_schema.h.md` tell how a module is written.
`docs/extern/lib/bridge_backend.h.md` tells how a backend is written.
`architecture.md` diagrams the parts and their relations.

## Install

1. Install Python 3.11 or a later version.
2. Run `python project.py fetch`. It downloads zig, the compiler of the build.

## Usage

| Command | Result |
| --- | --- |
| `python project.py fetch` | Downloads what the build needs. |
| `python project.py build` | Builds the library. |
| `python project.py pack` | Packs the library and its headers for a release. |
| `python project.py test` | Builds the library, and the programs and modules of the tests. Then runs the tests. |
| `python project.py test --break` | Runs the tests with their claims made false. Each test must fail. |
| `python project.py test --branches` | Runs the tests on a build with probes. Each branch of the code must run. |
| `python project.py check` | Checks the project. |
| `python project.py clean` | Removes what the build made. Keeps what `fetch` downloaded. |
| `python project.py distclean` | Removes the folder `build`, also what `fetch` downloaded. |

`python tools/deps.py build/<target> --check` compares the imports of the built library with `docs/extern/os/os.h.md`, *The imports*.
`python tools/reference.py` measures the storage of Bridge beside flecs and EnTT.

## Layout

The tree of `src` and `include`, with the role of each folder.

```text
src/
  lib/          Bridge library: the orchestrator a host links
    bridge/     public API + world lifecycle (create, load, reload, run_phase, set_order)
    storage/    ECS storage: entities; tables, sparse sets, toggles, direct-indexed, singletons
    block/      the world's shared-memory block, budget, per-module views
    region/     arena allocator over a reserved region
    schedule/   system and observer scheduler: conflicts, order, dispatch
    pool/       worker thread pool
    module/     per-module record and lifecycle (check, start, call, discard)
    backend/    opens a backend and resolves its 18-function vtable
    reload/     hot-reload: re-check configs, move component rows
    abi/        the module seam: the 8 ABI entries, relayed to a backend
    schema/     reads and compares type descriptor text, fingerprints types
    config/     writes a module's config settings into its settings component
    say/        formats records and calls the host sink
  host/         bridge-host: the reference host that runs a deployment
    toml/       TOML parser (the deployment file and its .d/local.toml overlay)
    config/     parsed TOML to a deployment (world, order, backends, content)
    manifest/   reads a module's embedded .bridge manifest (elf / macho / wasm)
    state/      builds a module's config from its manifest and the overlay
    show/       the --show-config output
  backends/     the backends a module runs on
    native/     runs a module as a native shared library, in-process (trusted)
    wasm/       runs a module as sandboxed core wasm over wasmtime
  os/           the OS portability seam, behind one header (os.h)
    posix/      Linux and macOS
    windows/    Windows
include/
  lib/          the seams: bridge.h (host), bridge_backend.h (backend),
                bridge_abi.h + bridge_schema.h (module)
  os/           os.h: the portability contract
  backends/
    native/     the native module SDK
    wasm/       the wasm module SDK (with acc)
```

## The host

Bridge ships `bridge-host`, a host that runs modules from a deployment file.
After `python project.py build`, run `bridge-host <config.toml>`, or `bridge-host --show-config <config.toml>` to print the deployment without running it.
It reads the file and its `.d/local.toml` overlay, reads each module's manifest, loads the modules a backend holds, and runs the phases in rounds.

The deployment file states these keys at its top:

| Key | Value |
| --- | --- |
| `phases` | The names of the phases. A round runs them in this order. |
| `threads` | How many threads run a phase, from 1. |
| `rounds` | The most rounds of the run, from 0 to 1000000. 0 is no limit. |
| `live` | `false`: a quiet round ends the run. `true`: a quiet round does not end it. |

A round is quiet if the look finds no change in the content and no phase writes a record.
The run also ends at the limit of `rounds`, and when Bridge refuses a call.
A system or an observer that gives `BRIDGE_END` ends the run after its round, with each value of `live`.
`docs/extern/lib/bridge_abi.h.md`, *BRIDGE_END* tells the status.
After a run, the host exits with 0, or with 1 if Bridge refused a call or a record is an error.
`--show-config` prints each of these keys, with the file and the line that state it.

These are its messages.

With no file, or a bad one, it prints its usage:

```text
usage: bridge-host <config.toml>
       bridge-host --show-config <config.toml>
```

Reading the deployment:

- `cannot go into '%s'`
- `cannot read '%s'`
- `not stated: backend`
- `not stated: content.paths`
- `not stated: %s`
- `refused: %s:%u: %s`
- `refused: %s:%u: %s: %s`
- `refused: %s:%u: %s: no module found is it`
- `refused: %s:%u: presets: a list of files`
- `refused: %s names itself:`
- `refused: %s: %s in %s:%u, %s in %s:%u`
- `, over %s:%u`
- `, named by %s:%u`
- `%s.d/local.toml`

Holding one instance:

- `refused: %s.d is no folder`
- `refused: %s is being run`
- `%s.d/lock`
- `cannot lock '%s'`

Finding the modules:

- `cannot look in '%s'`
- `too many files: %d are looked at`
- `not a module: %s: %s`
- `states a %.60s that cannot be`
- `unloaded: %s: %s is gone`

Loading a module:

- `loaded: %s, %s, %s`
- `? trust %s, %s, %s`
- `not loaded: %s: %s is %s`
- `not loaded: %s: %s %s`
- `not loaded: %s: %s, %s, not trusted`
- `not loaded: %s: %s was built against another ABI`
- `not loaded: %s: %s asks for %s, which the host was not given`
- `not loaded: %s: %s, as content.load says`
- `not loaded: %s: %s was refused`
- `not loaded: %s: %s:`

Replacing a module:

- `replaced: %s by %s, %s`
- `not replaced: %s: it does not say it can be`
- `not replaced: %s: %s was refused`
- `not replaced: %s: %s%s%s`
- `not replaced: %s: %s:`

Running the rounds:

- `bridge_create refused`
- `record[%d] %s/%s: %.*s %u %u`
- `ended after %d rounds, as many as it was given`
- `ended at round %d: a module asked for the end`
- `still loaded:`
- `destroyed: %d`

Showing the deployment:

- `found:`
- `content.paths: %s`
- `content.ignore: %s`
- `order: %s before %s`
- `config = "%s"  its file`
- `settings = "%s"  its file`
- `pinned = %s  its file`
- `reloadable = %s  its file`
- `its file states %s: %s`
- `its file states no config`
- `its file states no pinned`
- `its file states no reloadable`
- `it takes no config.%s, stated in %s:%u`
- `it names no component, and settings`
- `phases.%s`

## Contributing

| Document | Content |
| --- | --- |
| `docs/contributing/coding_style.md` | How to write the code. |
| `docs/contributing/documentation_style.md` | How to write the documents. |
| `docs/contributing/tests.md` | How to write the tests. |
| `docs/contributing/timings.md` | How to time a refactor commit and a perf commit. |
| `docs/contributing/commits.md` | How to write a commit. |
| `docs/contributing/pull_request.md` | How to write a pull request. |
| `docs/contributing/agreement.md` | The agreement `cla.md`, which each contributor signs. |

## License

The GNU Affero General Public License, version 3 only. The file `license` holds the text.
Each source of third-party code has its notice in `third_party/<source>/license`.

A host links the library, and a module or a backend is built with its headers and loaded by it.
Thus what is built with Bridge is under the same licence, and its source goes with it.
For a work that stays closed, other terms are for sale: ask the account `Encrid` on the service that hosts this repository.
