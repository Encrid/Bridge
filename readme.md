# Bridge

An orchestrator, written in C, that runs modules over shared ECS storage.

## Background

Bridge is a library. A host links it, and owns `main`, the loop and each file.
A host gives `bridge_create` a deployment, and gets a world: the storage, the modules loaded onto it, and the threads that run them.
The world stays until `bridge_destroy`. A process can have more than one world, and each world is separate.

The three layers are a host, Bridge and the modules. A module runs on a backend, for example native code or WebAssembly.
`docs/extern/bridge.h.md` tells how a host uses Bridge.
`docs/extern/bridge_abi.h.md` and `docs/extern/bridge_schema.h.md` tell how a module is written.
`docs/extern/bridge_backend.h.md` tells how a backend is written.

## Install

1. Install Python 3.11 or a later version.
2. Run `python project.py fetch`. It downloads zig, the compiler of the build.

## Usage

| Command | Result |
| --- | --- |
| `python project.py fetch` | Downloads what the build needs. |
| `python project.py build` | Builds the library, and the programs and modules of the tests. |
| `python project.py pack` | Packs the library and its headers for a release. |
| `python project.py test` | Runs the tests. |
| `python project.py test --break` | Runs the tests with their claims made false. Each test must fail. |
| `python project.py test --branches` | Runs the tests on a build with probes. Each branch of the code must run. |
| `python project.py check` | Checks the project. |
| `python project.py clean` | Removes what the build made. Keeps what `fetch` downloaded. |
| `python project.py distclean` | Removes the folder `build`, also what `fetch` downloaded. |

`python tools/deps.py build/<target> --check` compares the imports of the built library with `docs/intern/os/os.h.md`, *The imports*.
`python tools/reference.py` measures the storage of Bridge beside flecs and EnTT.

## The host

Bridge ships `bridge-host`, a host that runs modules from a deployment file.
After `python project.py build`, run `bridge-host <config.toml>`, or `bridge-host --show-config <config.toml>` to print the deployment without running it.
It reads the file and its `.d/local.toml` overlay, reads each module's manifest, loads the modules a backend holds, and runs the phases in rounds.

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

- `loaded: %s, %s, %s, %s`
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
