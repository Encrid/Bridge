# tools/reference.py

`reference.py` measures the storage of Bridge beside 2 other libraries of ECS storage, flecs and EnTT, and beside a
plain C array.

```sh
python tools/reference.py [--target <target>] [--times <count>]
```

The default target is `x86_64-windows-gnu`, and the default count of runs is 2.

## The steps

1. It stops if the build has no program `storage` for the target: `python project.py build` makes it.
2. `fetched` downloads the pinned sources into `build/reference`, unless they are there already.
3. `built` compiles the 3 programs of `tests/reference` for the target, with `-O2` and `-DNDEBUG`.
4. Each run writes `==== run <number>`. Then it writes the output of each of the 6 commands of the table, in order.

`storage` runs in its folder of the build. The 3 other programs run in the current folder.

| Command | Source | What it measures |
| --- | --- | --- |
| `storage measure` | `tests/storage/storage_test_measure.c` | Each operation of storage: its best round, its first round on new tables, and the cost of the call of a module. It also times ten thousand tables and sets that it makes and fills, a barrier with nothing to do, and a stress run. The stress run uses 4 threads, then 1 thread. |
| `storage weigh` | `tests/storage/storage_test_weigh.c` | An attach and a detach for each size of an entity, and a walk of the same rows in more and more tables. It also weighs a sparse set, a toggle, a direct-indexed component, markers, a singleton, and a toggle that a write switches. |
| `storage baseline` | `tests/storage/storage_test_baseline.c` | The reads of values: through a query, with a test of a field, and of one value on one entity. |
| `flecs_ref` | `tests/reference/flecs_ref.c` and the files `flecs_ref_*.c` | The same operations in flecs. |
| `entt_ref` | `tests/reference/entt_ref.cpp` | The same operations in EnTT. |
| `floor` | `tests/reference/floor.c` | A C array by entity, the least that a lookup by id can cost. |

Some rows give memory in KB, and not a time.

## The pinned sources

| Library | Version | Files |
| --- | --- | --- |
| flecs | 4.1.6 | `flecs.c` and `flecs.h`, of the folder `distr` |
| EnTT | 4.0.0 | `entt.hpp`, of the folder `single_include/entt` |

`PINNED` gives the address of each folder. `built` compiles `flecs.c` one time, into `build/reference/flecs.o`.

## zig and the target

`zig_path` takes the zig that `python project.py fetch` put into `build/zig`. Else it takes the zig of the path.
For a target of Windows, each program ends with `.exe`, and `flecs_ref` also links `ws2_32` and `dbghelp`.

## Messages

| Message | When |
| --- | --- |
| `reference.py: <path> is not built: run python project.py build` | The build has no program `storage` for the target. |
| `reference.py: zig is not there: run fetch` | No zig is in `build/zig` or on the path. |
| `reference.py: failed: <command>` | A command ended with a status that is not 0. Its output follows. |
| `fetching <address>` | `fetched` downloads a file. |

## Functions

| Function | Result |
| --- | --- |
| `run(command, **more)` | Runs a command, and gives its output. It stops the tool if the command fails. |
| `zig_path()` | Gives the path of zig. |
| `fetched(out)` | Downloads each pinned file that `out` does not hold. |
| `built(zig, target, out, exe)` | Compiles `flecs_ref`, `entt_ref` and `floor` into `out`. |
| `main()` | Reads the options, builds the programs, and writes the output of each run. |
