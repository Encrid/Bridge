# tools/reference.py

`reference.py` measures the storage of Bridge beside 2 other libraries of ECS storage, flecs and EnTT, and beside a
plain C array.

```sh
python tools/reference.py [--root <tree>] [--target <target>] [--times <count>]
```

`--root` names the tree whose program `storage` runs, for example a worktree of another commit.
Its default is the tree of the tool. The default count of runs is 2.

## The target and zig

`build.prepared` of the toolchain reads the configuration of the tree: `.toolchain/docs/build.py.md`, *build*.
It gives the target of this machine, or the target that `--target` names.
It stops for a target that is not there, or that does not run on this machine, since the programs run here.
It also gives zig, the folder of the build of the tree, and the keys `exe` and `system` of the target.

## The steps

1. It stops if the build of the tree has no program `storage` for the target: `python project.py build` makes it.
2. `fetched` makes sure that `build/reference` of this tree holds each pinned file.
3. `built` compiles the 3 programs of `tests/reference` of this tree, with `-O2` and `-DNDEBUG`.
4. Each run writes `==== run <number>`. Then it writes the output of each of the 6 commands of the table, in order.

The 3 programs come from the tree of the tool, so the runs of 2 trees use the same programs.
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
Each program ends with the `exe` of the target. For a target whose `system` is `windows`, `flecs_ref` also links
`ws2_32` and `dbghelp`.

## The pinned sources

| Library | Version | File | sha256 |
| --- | --- | --- | --- |
| flecs | 4.1.6 | `distr/flecs.c` | `6005392eb13c0f3c7abdecb2f85271e50934f63919afebf0bbe85f6dfc7320d6` |
| flecs | 4.1.6 | `distr/flecs.h` | `526036a5a41678e2a43a3cb835e9eaa70fd1993868b1978950c0d275752f69b1` |
| EnTT | 4.0.0 | `single_include/entt/entt.hpp` | `eedcb83fedffe640334ef69a0647625a186cf1daeb333b21b0a1445f01e89b6c` |

`PINNED` gives the address of each folder at its tag, and the sha256 of each file.
No maker publishes these digests. Each one is of a download of 2026-10-18.
`git hash-object` of each download gave the blob id that GitHub gives for the file at its tag.
`built` compiles `flecs.c` one time, into `build/reference/flecs.o`.

## The downloads

`fetched` keeps a file whose sha256 is the pinned one. It removes each other file, and downloads it again.
`downloaded` writes a download into the file with the added name `.part`, and checks its sha256.
Only then does the file get its name. A failed download, or a sha256 that is not the pinned one, removes the part.
`.toolchain/tools/fetch.py` cannot take these files, since its `fetched` unpacks an archive.

## Messages

| Message | When |
| --- | --- |
| `reference.py: <path> is not built: run python project.py build` | The build of the tree has no program `storage` for the target. |
| `reference.py: <address> did not download: <reason>` | A download failed. The status is 1. |
| `reference.py: <file> is not the pinned file: its sha256 is <digest>` | A download is not the pinned file. The status is 1. |
| `reference.py: failed: <command>` | A command ended with a status that is not 0. Its output follows. |
| `fetching <address>` | `downloaded` starts a download. |

## Limits

No case runs these branches.

- The download of the pinned files from their addresses. A case does not use the network.
- The run of the 6 commands, with its lines `====` and `--times`. The controls need the pinned files.

## Functions

| Function | Result |
| --- | --- |
| `run(command, **more)` | Runs a command, and gives its output. It stops the tool if the command fails. |
| `downloaded(url, path, digest)` | Downloads a file into `path`, as the section *The downloads* tells. |
| `fetched(out)` | Makes sure that `out` holds each pinned file, as the section *The downloads* tells. |
| `built(values, out)` | Compiles `flecs_ref`, `entt_ref` and `floor` into `out`, with the variables of `build.prepared`. |
| `main()` | Reads the options, builds the programs, and writes the output of each run. |
