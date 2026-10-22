# tools/reference.py

`reference.py` measures the storage of Bridge beside 2 other libraries of ECS storage, flecs and EnTT, and beside a
plain C array.

```sh
python tools/reference.py [--root <tree>] [--target <target>] [--times <count>]
python tools/reference.py --compare <old> <new> [--compare <old> <new>]
python tools/reference.py --pair <old> <new> [--target <target>]
```

`--root` names the tree whose program `storage` runs, for example a worktree of another commit.
Its default is the tree of the tool. The default count of runs is 2.
`--compare` compares 2 files of runs, as the section *The comparison* tells.
`--pair` times 2 commits against each other, as the section *--pair* tells.

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
5. Before the output of a command, it writes `====` and the command, for example `==== storage measure`.

The 3 programs come from the tree of the tool, so the runs of 2 trees use the same programs.
`storage` runs in its folder of the build. The 3 other programs run in the current folder.

| Command | Source | What it measures |
| --- | --- | --- |
| `storage measure` | `tests/lib/storage/storage_test_measure.c` | Each operation of storage: its best round, its first round on new tables, and the cost of the call of a module. It also times ten thousand tables and sets that it makes and fills, a barrier with nothing to do, and a stress run. The stress run uses 4 threads, then 1 thread. |
| `storage weigh` | `tests/lib/storage/storage_test_weigh.c` | An attach and a detach for each size of an entity, and a walk of the same rows in more and more tables. It also weighs a sparse set, a toggle, a direct-indexed component, markers, a singleton, and a toggle that a write switches. |
| `storage baseline` | `tests/lib/storage/storage_test_baseline.c` | The reads of values: through a query, with a test of a field, and of one value on one entity. |
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

## The comparison

These words have one meaning in the rule:

- A run starts at a line `==== run <number>`, and a command at each other line that starts with `====`.
- A word is a group of characters between spaces.
- A number is a word of digits with at most one `.`. A `(` at the start of the word does not count.
  Each `)`, `,`, `;` or `:` at its end does not count either.
- The unit of a number is the next word, with no `,` or `;` at its end.
- A row that comes before the first command of its run does not count.
- The control rows are the rows of the commands `flecs_ref`, `entt_ref` and `floor`.

The rule:

1. A number is a time if its unit is `ns`, `us` or `ms`.
2. Else, it takes the nearest unit of time before it in its row. With none, it is no time.
3. A number in parentheses is no time: a `(` comes before it in its row, with no `)` between.
4. The key of a time has 4 parts: its command, its text, its place, and its count.
5. The text is the row, with each number as `#` and the words joined by 1 space.
6. The place counts the numbers of the row, from 1.
7. The count is the count of the same rows before it, in the same command of the same run.
8. For each key and each file, the tool takes the least and the spread: the most minus the least.
9. A time is slower when the new least passes the old least by more than the larger spread.
10. A key that is not in each run of both files is only in one tree.

The tool compares the numbers as decimals, so 0.5 minus 0.3 is exactly 0.2.
With more than 1 pair of files, a time counts only if it is slower in each pair.

## The report

```text
slower: <command>: <text>, row <row>, number <place>: <old least> to <old most>, then <new least> to <new most>
control: <command>: <text>, row <row>, number <place>: <old least> to <old most>, then <new least> to <new most>
only in one tree: <command>: <text>, row <row>
<n> of <m> times of Bridge are slower
<n> of <m> times of the control rows are slower
```

- A slower time of a control row has the line `control`, and each other slower time has the line `slower`.
- `<row>` is the count plus 1. A slower time gets a line for each pair.
- `<n>` counts the times that are slower in each pair, and `<m>` counts the keys of both trees.
- The report lists the times in the order in which the first file holds them.

Some times are one sample in each run: `first`, `ten thousand tables made`, `ten thousand more, by one attach
each`, and the 2 rows of the stress run.
The basics rows of flecs and EnTT are one sample too. Their noise is larger than that of a least of rounds.
The rows `a barrier with nothing to do` and `a barrier with nothing to do, then` are near one tick of the clock.
A slowdown that is smaller than the spread does not show. Thus 0 slower times does not prove that no time
changed.

## --pair

1. It prunes the worktrees of the repository, then makes `build/timing/old` and `build/timing/new`.
2. It checks out each commit in its worktree, with no branch.
3. It builds each tree with `.toolchain/tools/build.py --root` of this tree, so with the same zig.
4. It builds the 3 programs of `tests/reference` of this tree one time.
5. It makes one run of each tree that it does not count. A scan of the new files can end during it.
6. It makes 12 runs: old, new, old, new, old, new, then new, old, new, old, new, old.
7. The first 6 runs are the first pair, and the last 6 are the second pair.
8. It writes the 4 files of runs and the report into `build/timing/runs/<old>-<new>`, and writes the report.

The 2 worktrees have paths of the same length, since the length of a path can move code and data.
The output of the builds stays out of the report. A failed command stops the tool with its output.

## Messages

| Message | When |
| --- | --- |
| `reference.py: <path> is not built: run python project.py build` | The build of the tree has no program `storage` for the target. |
| `reference.py: <address> did not download: <reason>` | A download failed. The status is 1. |
| `reference.py: <file> is not the pinned file: its sha256 is <digest>` | A download is not the pinned file. The status is 1. |
| `reference.py: failed: <command>` | A command ended with a status that is not 0. Its output follows. |
| `fetching <address>` | `downloaded` starts a download. |
| `reference.py: <file> is not there` | `--compare` names a file that is not there. The status is 1. |
| `reference.py: <file> holds no run` | `--compare` names a file with no line `==== run <number>`. The status is 1. |

## Limits

No case runs these branches. The step of `docs/contributing/timings.md` runs them.

- The download of the pinned files from their addresses. A case does not use the network.
- The run of the 6 commands, with its lines `====` and `--times`. The controls need the pinned files.
- The option `--pair`. The controls need the pinned files, and its 14 runs take more than 120 s.

## Functions

| Function | Result |
| --- | --- |
| `run(command, **more)` | Runs a command, and gives its output. It stops the tool if the command fails. |
| `downloaded(url, path, digest)` | Downloads a file into `path`, as the section *The downloads* tells. |
| `fetched(out)` | Makes sure that `out` holds each pinned file, as the section *The downloads* tells. |
| `built(values, out)` | Compiles `flecs_ref`, `entt_ref` and `floor` into `out`, with the variables of `build.prepared`. |
| `measured(number, ours, out, exe)` | Gives the text of one run, with its lines `====`. |
| `timed(row)` | Gives the text of a row, and the place and the value of each of its times. |
| `runs(path)` | Gives the times of each run of a file, by key, as the section *The comparison* tells. |
| `compared(old, new)` | Gives the keys of both trees, the keys of one tree only, and the slower times. |
| `reported(pairs)` | Gives the lines of the report for 1 or 2 pairs of files. |
| `paired(old, new, target)` | Does the steps of the section *--pair*. |
| `main()` | Reads the options, and runs, compares, or times a pair. |
