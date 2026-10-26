# project.py

`project.py` runs the toolchain of the project.
Each command runs one tool of the folder `.toolchain/tools`.
The tools read their values from `.toolchain/toolchain.toml` and from `.toolchain/project.toml`.
`.toolchain/docs/project.toml.md` describes these values.
Do not change `project.py` or the files of `.toolchain`.

## Requirements

- Python 3.11 or a later version.
- zig at the version of `[zig] version` in `.toolchain/toolchain.toml`. The command `fetch` downloads this zig.

The tools use the zig in `build/zig` of the folder of `project.py` first.
If that zig is not there, the tools use the zig on the path.
The tools stop if the version of zig is not the pinned version.

## Commands

```sh
python project.py <command> [<option> ...]
```

| Command | Tool | Result |
| --- | --- | --- |
| `fetch` | `.toolchain/tools/fetch.py` | Downloads zig if zig is not there. Then fetches the sources and takes the dependencies. |
| `build` | `.toolchain/tools/build.py` | Takes the dependencies and builds all the artifacts. |
| `pack` | `.toolchain/tools/build.py --pack` | Builds all the artifacts. Then puts the package into a zip file. |
| `test` | `.toolchain/tools/test.py` | Builds all the artifacts. Then runs all the cases. |
| `check` | `.toolchain/tools/check.py` | Runs all the checks of the project. |
| `clean` | `.toolchain/tools/clean.py` | Removes what the build made, but keeps zig and the sources that `fetch` downloaded. |
| `distclean` | `.toolchain/tools/clean.py --all` | Removes the output folder, also zig and the sources that `fetch` downloaded. |
| `cla` | `.toolchain/tools/cla.py` | Checks that the author of a pull request signed `cla.md`. |

`project.py` gives its options to the tool without a change.
It ends with the exit status of the tool.
You can also run a tool alone, for example `python .toolchain/tools/build.py --debug`.
If the command is not in the table, `project.py` stops with the message
`project.py: name a command: fetch build pack test check clean distclean cla`.

## Options

| Option | Result |
| --- | --- |
| `--root <folder>` | Uses the project in this folder. The default is the folder of `project.py`. |
| `--target <target>` | Builds for this target. The default is the target of this machine. |
| `--debug` | Replaces each variable that `[debug]` names with the value of `[debug]`. |
| `--out <folder>` | Puts the output into this folder. The default is the folder `build` of the project. |
| `--set <name>=<value>` | Sets a key of `[variables]` to the value. You can use this option more than one time. |
| `--pack` | Puts the package into a zip file. The command `pack` gives this option to `.toolchain/tools/build.py`. |
| `--break` | Runs the false variant of each case. It is an option of `test` only. |
| `-k <part>` | Runs only the cases whose name contains `<part>`. It is an option of `test` only. |
| `--copies <number>` | Runs this number of copies of each case at the same time. It is an option of `test` only. |
| `--rounds <number>` | Runs all the cases this number of times. It is an option of `test` only. |
| `--toolchain` | Runs the cases of the toolchain, in place of the cases of the project. Only the repository `my_c23_template` has them. It is an option of `test` only. |
| `--branches` | Builds the project with probes, runs each case one time, and writes each branch that no case reaches. It is an option of `test` only. |
| `--leaks` | Builds the project with counts, runs each case one time, and writes each case that keeps a resource. It is an option of `test` only. |
| `--all` | Removes all of the output folder. The command `distclean` gives this option to `.toolchain/tools/clean.py`. |
| `--commits <range>` | Checks the messages of the commits of this range of git. It is an option of `check` only. |

The command `fetch` always uses the target of this machine.
The command `test` runs only on a machine of the target.
With `--root`, the tools of one project can work on a different project, for example a project in `tests`.
zig and its caches stay in the folder of `project.py`.

## Output

- The output folder is the variable `build`: `.toolchain/docs/shared.py.md`, *Variables*. Here, its name is `<build>`.
- The output folder of a target is `<build>/<target>`. With `--debug`, it is `<build>/<target>-debug`.
- An artifact is in the output folder of the target, or in its `folder` in that output folder.
- The steps of a kind put their temporary files into `<build>/<target>/obj`.
- `<build>/<target>/package.txt` lists the artifacts and the `beside` files, with the SHA-256 digest of each file.
  A different project can take these files as a dependency.
- `pack` writes one zip file, `<build>/<project>-<target>.zip`. `<project>` is the value of `[project] name`.
- Each case runs in its own empty folder, `<build>/<target>/cases/<group>/<number>`.
- `test` writes the output of the commands of all the cases into `<build>/<target>/said.txt`, but not with `--break`.
- `test --branches` puts the sources with the probes into `<build>/branches/tree`, and its build, its cases and the
  file of the hits `hits.txt` into `<build>/branches/<target>`.
- `test --leaks` puts the sources with the counts into `<build>/leaks/tree`, and its build and its cases into
  `<build>/leaks/<target>`. The lines of a case are in `<build>/leaks/<target>/cases/<group>/<number>.leaks`.
- The caches of zig are in `build/zig-cache` of the folder of `project.py`. Thus a build does not use a cache of the machine.

`build` writes the path of each artifact.
`pack` also writes what `packed` writes: `.toolchain/docs/build.py.md`, *The zip*.

### The zip

`pack` puts into the zip the files that `.toolchain/docs/build.py.md`, *The zip*, lists.

## Cases

`docs/contributing/tests.md` tells how to write the cases that `test` runs.

## The tests of the toolchain

The cases of the toolchain are only in the repository `my_c23_template`, in `.toolchain/tests/cases`. The files that they read are in `.toolchain/tests/data`.
`.toolchain/tests/docs` describes each group.
Run them in that repository after each change of the toolchain:

```sh
python project.py test --toolchain
python project.py test --toolchain --break
```

With `--toolchain`, `test` does not build the project, and does not check the messages of the code.
