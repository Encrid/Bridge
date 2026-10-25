# Bridge

Bridge is a C library that loads and runs modules over shared ECS storage.

## Background

Bridge is for a program that gains its features from modules at run time, such as a moddable or hostable application or game.
A host links Bridge and owns `main`, the loop and each file.
A host gives `bridge_create` a deployment, and gets a world: the storage, the modules loaded onto it, and the threads that run them.
The world stays until `bridge_destroy`. A process can have more than one world, and each world is separate.

The three layers are a host, Bridge and the modules. A module runs on a backend, for example native code or WebAssembly.
The host Bridge-Host and the backends BB-Native and BB-Wasm are repositories of their own.

`docs/extern/bridge.h.md` tells how a host uses Bridge.
`docs/extern/bridge_abi.h.md` and `docs/extern/bridge_schema.h.md` tell how a module is written.
`docs/extern/bridge_backend.h.md` tells how a backend is written.
`architecture.md` diagrams the parts and their relations.

## Install

1. Install Python 3.11 or a later version.
2. Run `python project.py fetch`. It downloads zig, the compiler of the build, and the os layer of Bridge-OS.

## Usage

| Command | Result |
| --- | --- |
| `python project.py fetch` | Downloads what the build needs. |
| `python project.py build` | Builds the library. |
| `python project.py pack` | Packs the library and its headers for a release. |
| `python project.py test` | Builds the library, and the programs and modules of the tests. Then runs the tests. |
| `python project.py test --break` | Runs the tests with their claims made false. Each test must fail. |
| `python project.py test --branches` | Runs the tests on a build with probes. Each branch of the code must run. |
| `python project.py test --leaks` | Runs the tests on a build with counts. Each test must give back what the code takes. |
| `python project.py check` | Checks the project. |
| `python project.py clean` | Removes what the build made. Keeps what `fetch` downloaded. |
| `python project.py distclean` | Removes the folder `build`, also what `fetch` downloaded. |

`python tools/deps.py build/<target> --check` compares the imports of the built library with `docs/extern/bridge.h.md`, *The imports*.
`python tools/reference.py` measures the storage of Bridge beside flecs and EnTT.

## Layout

The tree of `src` and `include`, with the role of each folder.

```text
src/            Bridge library: the orchestrator a host links
  bridge/       public API + world lifecycle (create, load, reload, run_phase, set_order)
  storage/      ECS storage: entities; tables, sparse sets, toggles, direct-indexed, singletons
  block/        the world's shared-memory block, budget, per-module views
  region/       arena allocator over a reserved region
  schedule/     system and observer scheduler: conflicts, order, dispatch
  pool/         worker thread pool
  module/       per-module record and lifecycle (check, start, call, discard)
  backend/      opens a backend and resolves its 18-function vtable
  reload/       hot-reload: re-check configs, move component rows
  abi/          the module seam: the 8 ABI entries, relayed to a backend
  schema/       reads and compares type descriptor text, fingerprints types
  config/       writes a module's config settings into its settings component
  say/          formats records and calls the host sink
include/        the seams: bridge.h (host), bridge_backend.h (backend),
                bridge_abi.h + bridge_schema.h (module)
```

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
