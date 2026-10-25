# .toolchain/tests/cases/build.toml

The group `build` checks `.toolchain/tools/build.py`, and the parts of `.toolchain/tools/shared.py` that the tools share.
Its cases run `build.py` or `test.py` on a project in `.toolchain/tests/data/build`, with the output in `{case}/out`.
The cases of the options, of the report and of the files of a project run a short Python program that calls `shared`.

| Project | Content |
| --- | --- |
| `built` | The library `greet`, the program `greeter`, the archive `greet_static` (with the flag `-DGREETING`), the program `greeter_static` that links it, and the artifact `stamped` of the part `extra`, with its variant `plain`. |
| `copy` | The artifact `copy.txt` of the kind `file`. The variables `tool`, `source` and `linked` give its tool, its source and its link. |
| `twice` | `[variables] language`, a key that `.toolchain/toolchain.toml` also gives. |
| `builtin` | `[variables] root`, the name of a variable that the tools give. |
| `unknown` | An artifact with the name `{named}`. The variable `named` is `{nothing}`, which has no value. |
| `listed` | An artifact with the name `{named}`. The variable `named` is `{language}`, a list in a text. |
| `targeted` | The artifact `near-{target}.txt`, and its variant `far-{target}.txt` with the target `nowhere`. The project adds `nowhere`, whose `runs_on` is empty. |
| `untargeted` | The same artifact and variant, but the variant has no `target`. |
| `module` | The modules `mod`, `two` and its variant `three`, over the defaults of `[module]`, and the program `pathcheck` that reads their `*_PATH` macros. |
| `config` | The artifact `thing`, stated in a file of `.toolchain/config` and not in `project.toml`. |
| `leaky` | The artifact `stamped`, whose section holds `{root}`: the folder of the project. |
| `archived` | The archive `one`. |

In `built`, `greet` gets its greeting from `defines`.
`greeter` includes `.toolchain/tests/data/build/built/include/greet.h`, links `greet`, and has `greet` beside it in its folder.
`greeter` also compiles `.toolchain/tests/data/build/built/src/<system>/system.c`, and writes `hello from a library, on <system>`.
The case `greeter` of `built` runs `greeter`. Its false variant runs a copy of `greeter` without the library.

The proof source includes `.toolchain/tests/data/build/built/include/greet.h` and `.toolchain/tests/data/build/built/include/sub/extra.h`.
So the proof reads a header from a sub-folder of `include`.
The proof also calls `outside`, which only `.toolchain/tests/data/build/built/tests/package/outside.c` gives.
The package does not hold that file, and `[package] link` names it through the variable `outside`.
`build --pack` builds no test artifact, thus the project has no archive outside the package.
zig compiles a source in the list of links, thus the file does the work of an archive of a different project.

The proof finds the declaration of `outside` in `.toolchain/tests/data/build/built/tests/package/include/outside.h`.
The package does not hold that header either, and `[package] include` names its folder through the variable `outside_include`.

The kind `stamped` copies `.toolchain/tests/data/build/built/src/note.txt` to a work file, renames the work file to the output, and adds the section `said`.
Then it checks that the output contains `{said}`. The default of `said` is `written`, a word of `note.txt`.
The section holds the items of `manifest` with commas between them: `first,second`.
The variant `plain` is unstamped.

The case of an archive of a release copies `archived` into `{case}/proj`, and runs `build.main` from a short Python program.
The program puts `[build] cache` in that folder, as in a project. A project in `.toolchain/tests/data/build` does not hold the cache.

| Case | Claim | False variant |
| --- | --- | --- |
| a program linked with a library | `test` on `built` passes the cases `greeter` and `greeter_static`: the archive and its flag work. | `test --break`. |
| a variant without its section | With `--set said=first,second`, `build` stops: `plain.txt does not hold 'first,second'`. | No `--set`. |
| a build for debugging | With `--debug`, `package.txt` is in `{case}/out/{target}-debug`. | No `--debug`. |
| a package in a zip | `build --pack` writes the path of the zip, and the names of the files in it. The proof builds with the nested header `include/sub/extra.h`. `greeter_static` has `test = true`, thus no name of the zip starts with `greeter_static/`. | No `--pack`. |
| the proof links the files of [package] link | `build --pack` writes `package proof: built ran`. The proof calls `outside`, and only the linked `outside.c` gives it. | `--set outside={case}/none.c`, a file that is not there. Thus `build` stops before the proof links. |
| the proof includes the folders of [package] include | `build --pack` writes `package proof: built ran`. The proof includes `outside.h`, and only the folder that `[package] include` names holds it. | `--set outside_include={case}/none`, a folder that is not there. Thus the proof does not compile. |
| a build of a part skips the artifacts of other parts | `build --part main` builds the untagged `greeter`, and not `stamped.txt` of the part `extra`. | No `--part`, thus `stamped.txt` too. |
| a module over the defaults, with a path | `build` on `module` builds `mod` from the default sources, `two` and its variant `three`, and `pathcheck` with their `*_PATH` macros. | `build` on `copy`. |
| the files of config merge | `build` on `config` makes `thing`, whose `[[artifact]]` is in a file of `.toolchain/config`, not `project.toml`. | `build` on `copy`. |
| a variable that [variables] does not have | `--set nowhere=1` stops `build`: `--set nowhere=1: [variables] has no nowhere`. | No `--set`. |
| a key in project.toml and in toolchain.toml | `build` on `twice` stops: `project.toml states variables.language, and toolchain.toml states it too`. | `build` on `copy`. |
| a built-in variable in [variables] | `build` on `builtin` stops: `[variables] names a built-in variable or a key of a target: root`. | `build` on `copy`. |
| a target that the toolchain does not have | `--target nowhere` stops `build`: `no target nowhere; the targets:`. | No `--target`. |
| a tool that is not there | `--set tool=nowhere` stops `build`: `the tool interpreter is not at nowhere`. | No `--set`. |
| sources that match no file | `--set source=none.txt` stops `build`: `no file matches the sources of copy.txt`. | No `--set`. |
| a link that is not there | `--set linked=missing.txt` stops `build`: `missing.txt is not there, and copy.txt is linked with it`. | No `--set`. |
| a variable that has no value | `build` on `unknown` stops: `no variable`. | `--set named=named.txt`. |
| a list variable in a text | `build` on `listed` stops: `is a list inside a text`. | `--set named=named.txt`. |
| an artifact for a different target | `build` on `targeted` lists `far-nowhere.txt` in `package.txt`. No machine runs that target. | `build` on `untargeted`. |
| an option with a value | `shared.arguments('--commits')` gets `--commits a..b`, and the program writes `a..b`. | No `--commits`, thus the program writes `None`. |
| a report with faults | `shared.report` gets two faults. It writes them and `faults: 2`, and the status is 1. | No fault. |
| a report with no fault | `shared.report` gets no fault, 12 and `commits`. It writes `no fault in 12 commits`, and the status is 0. | One fault. |
| the files of a project, walked | `shared.repository` writes `False` for `{case}/w`. `shared.ours` gives `{case}/w/a.txt` and `{case}/w/src/build/e.txt`. It leaves out `tests`, the folder `build` at the root and `__pycache__`. | `{case}/w/src/b.txt` is also there. |
| the files of a project, from git | The program makes `{case}/g` a repository of git, and `shared.repository` writes `True`. `shared.ours` gives `a.txt` and `.gitignore`, but not `tests` or a file that git ignores. | `{case}/g/src/b.txt` is also there. |
| a package that holds a path of its machine is refused | `build --pack` on `leaky` stops: `stamped.txt` holds the folder of the project. | No `--pack`. |
| the cache of zig is a path from the folder of the project | `build.prepared` on the template, which holds the cache, sets `ZIG_LOCAL_CACHE_DIR` to `build/zig-cache/local`. | `build.prepared` on `built`, which does not hold the cache: a whole path. |
| an archive of a release holds no path of its machine | `build --pack` on the copy of `archived` writes its zip, thus `one.a` holds no folder of the machine. On Windows, `one.a` holds one without `-fstrip`. | `--debug`: the debug information of `one.a` holds the folder of the project, thus `pack` stops. |

`.toolchain/docs/build.py.md` gives the limits of these cases.
