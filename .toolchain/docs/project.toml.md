# .toolchain/project.toml

`.toolchain/project.toml` holds `[project]` and `[test]`.
The other values are in the files of `.toolchain/config`, one file for each concern: `artifacts.toml`, `modules.toml`, `targets.toml`, `kinds.toml`, `sources.toml`, `variables.toml`, `package.toml`, `checks.toml` and `leaks.toml`.
zig, the compiler flags and the limits of the style are in `.toolchain/toolchain.toml`.
The tools read every file as one configuration.

- A table of a file adds its keys to the table of another file that has the same name.
- If two files give the same key, the tools stop: `project.toml states <key>, and toolchain.toml states it too`.
- A file of a project cannot change a value of `.toolchain/toolchain.toml`. Do not change `.toolchain/toolchain.toml`.

Each section of this document gives the keys of one table.
For example, these lines make a program from the sources of `src`:

```toml
[[artifact]]
name = "hello"
kind = "program"
sources = ["src/*.c"]
```

## Variables

A text of the configuration can contain variables.
A variable is a name in braces, for example `{root}`.
The tools replace each variable with its value before they use the text.
A value can also contain variables.

The tools give these variables:

| Variable | Value |
| --- | --- |
| `{root}` | The folder of the project. |
| `{build}` | The output folder, as `.toolchain/docs/shared.py.md`, *Variables*, gives it. |
| `{dist}` | The deliverable folder of the target, `<build>/<target>`. |
| `{out}` | The scratch folder of the target, `<dist>/scratch`. |
| `{target}` | The name of the target. |
| `{home}` | The name of the target of this machine. |
| `{python}` | The Python program that runs the tool. |
| `{zig}` | The zig program. |
| `{paths}` | The `*_PATH` macros of the test libraries, as `.toolchain/docs/build.py.md`, *Modules*, tells. |

A case has more variables: `docs/contributing/tests.md`, *Variables* gives them.
Each key of `[targets.<target>]` is also a variable, for example `{exe}`, `{library}` and `{system}`.
Each key of `[variables]` is a variable.
Each source, each dependency and each tool is a variable too.
A path in a variable uses `/` as the separator, on all systems.

A variable can have a list as its value.
If a list variable is a full item of a list, the items of the variable replace it.
If a list variable is a part of a text, give a separator after a colon.
For example, in the TOML text `'{manifest:\0}'`, the separator is the two characters `\0`.
In the TOML text `"{manifest:\u0000}"`, the separator is the character with the code 0.

The tools stop if a variable has no value: `no variable {<name>} for <text>`.
They also stop if a list variable is a part of a text, and has no separator.

## [project]

| Key | Value |
| --- | --- |
| `name` | The name of the project. The title of `readme.md`, `package.txt` and the zip of `pack` take it. |

If `name` is empty, the tools that need it stop: `.toolchain/project.toml: write the name of the project in [project] name`.

## [variables]

Each key of `[variables]` is a variable.
A key must not have the name of a variable that the tools give, or of a key of a target.
`.toolchain/toolchain.toml` gives the keys `language`, `optimize` and `float`.
With `--debug`, each key of `[debug]` in `.toolchain/toolchain.toml` replaces the variable with the same name.
`--set <name>=<value>` replaces the value of a key of `[variables]`.

## [tools]

Each key of `[tools]` is the name of a program, and its value is the path of the program.
The path does not include the extension of the program. The tools find the extension.
The tools stop if a program is not there.

## [[source]]

A source is an archive of files that a different maker publishes, for example the source code of a program.
Put the patches of a source and its licence into `third_party/<source>`.

| Key | Value |
| --- | --- |
| `name` | The name of the source. The variable `{<name>}` is its `top` folder, in its `into` folder. |
| `url` | The address of the archive. A text with no `://` names a file of the project. |
| `sha256` | The SHA-256 digest of the archive. |
| `into` | The folder that gets the files, for example `{build}/third_party/<source>`. |
| `top` | Optional. The folder of the archive that holds the files. |
| `patches` | Optional. The patches, as files of the project, in the order that `fetch` applies them. |

For example:

```toml
[[source]]
name = "sample"
url = "https://example.org/sample-1.0.tar.gz"
sha256 = "<digest>"
into = "{build}/third_party/sample"
top = "sample-1.0"
patches = ["third_party/sample/fix.patch"]
```

The command `fetch` does these steps for each source:

1. It removes the files of the `into` folder, and downloads the archive into the folder.
2. It stops if the archive does not have the digest of the source.
3. It unpacks the archive, and deletes it.
4. It applies each patch to the files of the `top` folder, in the order of the list.
5. It writes `fetched.txt` into the `into` folder. The file holds the digest of the archive and of each patch.

A patch applies only if each line of its context is in the file, at the line that the patch gives.
A patch can end its lines as Windows does or as Linux does.
`fetch` stops if a patch does not apply.
If `fetched.txt` holds the digests of the configuration, `fetch` does not download the archive again.
The commands `build`, `pack` and `test` do not download a source.
They stop if `fetched.txt` does not hold the digests of the configuration.

## [[dependency]]

A dependency is a folder of files that a different project makes.
Each dependency has a list of its files. A line of the list is `<key> <value>`, or `<digest>  <file>`.
The `package.txt` of a different project is such a list.

| Key | Value |
| --- | --- |
| `name` | The name of the dependency. The variable `{<name>}` is its `into` folder. |
| `from` | The folder that has the files. |
| `list` | The list of the files, in `from`. |
| `into` | The folder that gets the copies. |
| `make` | Optional. A command that runs before the tools read the list. |
| `optional` | Optional. If it is `true` and the list is not there, the tools do not take the dependency, and continue. |

For example, this dependency takes the package of a project beside this project:

```toml
[[dependency]]
name = "sdk"
from = "../sdk/build/{home}"
list = "package.txt"
into = "{build}/taken/sdk"
```

The tools copy the files of each dependency into its `into` folder.
They copy a file only if the copy is not there, or if the copy is different.
They stop if a file does not have the digest that the list gives.
They also stop if two dependencies do not agree, as each `[[agree]]` tells.
In these two cases, the tools do not change a file.

## [[agree]]

| Key | Value |
| --- | --- |
| `keys` | The keys that must have the same value, as `<dependency>.<key>`. The tools ignore a dependency that they did not take. |
| `length` | The number of characters at the start of each value that the tools compare. |

## [kinds.\<kind\>]

A kind gives the steps that make an artifact.
`.toolchain/toolchain.toml` gives the kinds `program`, `library`, `archive` and `file`.
`.toolchain/project.toml` can add more kinds.

| Key | Value |
| --- | --- |
| `output` | The name of the file of an artifact. |
| `steps` | The steps that make an artifact. |
| `unlinked` | Optional. The steps when one of the files of `link` is missing. |

A step does one of these actions:

| Key | Action |
| --- | --- |
| `run` | Runs a command, in the folder of the project. |
| `section`, `text` | The name and the text of the custom section that the step writes into a WebAssembly artifact. |
| `contains` | Makes sure that the bytes of the artifact contain this text. |
| `copy` | Copies the first file to the second file. |
| `rename` | Moves the first file to the second file. |

The steps can use these variables of the artifact:

| Variable | Value |
| --- | --- |
| `{name}` | The name of the artifact. |
| `{output}` | The file of the artifact. |
| `{work}` | The start of the name of a temporary file. |
| `{sources}` | The files that match the patterns of `sources`. |
| `{defines}` | Each item of `defines`, after `-D`. |
| `{include}` | Each folder of `include`, after `-I`. |
| `{flags}` | The items of `flags`. |
| `{link}` | The files of `link`. |
| `{manifest}` | The items of `manifest`. |

## [targets.\<target\>]

`.toolchain/toolchain.toml` gives six targets, and `.toolchain/docs/toolchain.toml.md` describes their keys.
`.toolchain/project.toml` can add more keys to a target. Each key of a target is a variable.

## [[artifact]]

An artifact is a file that the build makes, for example a program, a library or a copied file.

| Key | Value |
| --- | --- |
| `name` | The name of the artifact. It can contain variables, for example `{system}`. |
| `kind` | The kind of the artifact. |
| `sources` | The patterns of the sources, from the folder of the project. |
| `include` | Optional. The folders of the headers. |
| `defines` | Optional. The macros, without `-D`. |
| `flags` | Optional. More flags of the compiler, for example `-fvisibility=hidden` or `-lm`. They come after the sources and the files of `link`. They can contain variables. |
| `link` | Optional. The files that the artifact links. |
| `beside` | Optional. The files to copy into the folder of the artifact. |
| `manifest` | Optional. A list of texts for the steps. |
| `folder` | Optional. The folder of the artifact, in the output folder of the target. |
| `target` | Optional. The target of the artifact, if it is not the target of the build. |
| `variants` | Optional. The variants. Each variant has a `name`, and can have the other keys and `unstamped`. |
| `test` | Optional. `true` for an artifact that only the tests use. Only the command `test` makes it, and `package.txt` does not list it. Thus `pack` does not put it into the zip. |
| `part` | Optional. The name of a part of the project. `--part <name>` acts on the part `<name>`, and repeats. A build makes the base and each named part. The base is the untagged artifacts, and a build always makes it. |
| `path` | Optional. `true` for a test library that a case loads by a `*_PATH` macro. `.toolchain/docs/build.py.md`, *Modules*, tells it. |

A variant is a copy of the artifact with a different name and some different keys.
A variant with `unstamped = true` does not do the `section` steps.
An artifact with a `target` gets the keys of that target and the variable `{target}`, for example `{exe}` and `{triple}`.
`.toolchain/docs/build.py.md`, *Artifacts*, tells where its output goes, and when the `unlinked` steps run.

Thus a build for a different target can compile the sources, and not link them.
If the kind has no `unlinked` steps, the tools stop.
The command `test` stops if an artifact is not linked.

## [module] and [[modules]]

`[module]` holds the keys shared by the built test modules.
`[[modules]]` lists each one. A row adds a `name`, what differs from the shared keys, and `path = true` to expose a `*_PATH` macro.
`.toolchain/docs/build.py.md`, *Modules*, gives the rest.

## [compose]

`[compose]` tells what the deliverable of each part holds, beside the binaries that the part builds.
`.toolchain/docs/build.py.md`, *The deliverable*, tells how `build` composes it.

`[compose.<part>]` has these keys, for the part `<part>`:

| Key | Value |
| --- | --- |
| `headers` | Optional. The patterns of the public headers of the part, from the folder of the project. |
| `assets` | Optional. The files of the part, from the scratch folder, for example a merged `acc.wasm`. |

A part with no table, and a part with empty keys, composes only its binaries.

## [test]

`docs/contributing/tests.md`, *The settings* gives the keys of `[test]`.

## [check]

| Key | Value |
| --- | --- |
| `unicode` | Optional. The paths of the files that can hold text that is not ASCII. A path can contain `*`. The default is an empty list. |
| `guides` | Optional. The names of the guides that the project adds to `docs/contributing`, beside the 6 guides of the template, for example `timings.md`. `.toolchain/docs/layout.py.md`, *The paths* allows them. |
| `retired` | Optional. A table of the retired words of the project. A key is a retired word, and its value is the word to write, or an empty text. `.toolchain/docs/text.py.md`, *Retired words* tells the check. |

For example:

```toml
[check]
unicode = ["docs/extern/names.md"]

[check.retired]
utilise = "use"
whilst = ""
```

## [[style.exception]]

An exception lets one file break one rule of the style.

| Key | Value |
| --- | --- |
| `rule` | The rule, for example `F4`. |
| `path` | The path of the file. It can contain `*`. It can end with `:` and the name of a function. |
| `why` | The reason for the exception. |

For example:

```toml
[[style.exception]]
rule = "F4"
path = "src/parse.c:parse"
why = "The table of the grammar is one long function."
```

The check of the style reports an exception that excuses nothing.

## [[branches.exception]]

An exception lets a project keep a defensive branch that no case can reach, for `test --branches`.

| Key | Value |
| --- | --- |
| `path` | The path of the file. It can contain `*`. |
| `line` | The line of the branch. |
| `col` | The column of the branch. |
| `side` | `true`, `false`, `function` or `case`. |
| `why` | The reason the branch is unreachable. |
| `system` | Optional. The system the exemption applies on, for a branch that only one platform has. |

For example:

```toml
[[branches.exception]]
path = "src/backends/wasm/wasm_own.c"
line = 23
col = 9
side = "true"
why = "A wasm32 memory holds at most 65536 pages, which is the reserve, so a grow never exceeds it."
```

`test --branches` reports an exception that states no reason, or that matches no unreached outcome.
An exemption with a `system` is applied and checked only on that system.

## [leaks]

`[leaks]` names the functions that `test --leaks` counts, in addition to `malloc`, `calloc`, `realloc` and `free`.
A project writes it in `leaks.toml` of `.toolchain/config`.

| Key | Value |
| --- | --- |
| `memory` | The functions that take memory, which the code gives back with `free`. |
| `pairs` | A table: each key is a function that takes a resource, and its value is the function that gives it back. |
| `kinds` | A table of kinds. Each kind has `takes` and `gives`: tables from the name of a function to a place, or to a list of places. |
| `with` | The files that give more `memory`, `pairs` and `kinds`, each in a table `[leaks]`. The variables of the build are replaced in each path. |

For example:

```toml
[leaks]
memory = ["os_path_whole"]

[leaks.pairs]
os_reserve = "os_release"
os_lib_open = "os_lib_close"
```

A kind names its functions at their places, for example a module that a call writes through its fourth argument:

```toml
[leaks.kinds.wasmtime_module]
takes = { wasmtime_module_new = "*4" }
gives = { wasmtime_module_delete = "1" }
```

A project that uses a package can name the file of the package: `with = ["{bridge_os}/leaks.toml"]`.
`.toolchain/docs/leaks.py.md` tells what a function of a pair must give and get. `.toolchain/docs/leaks.py.md`, *The places*, tells the places of a kind.

## [[leaks.exception]]

An exception lets a case keep a resource by design, for `test --leaks`.

| Key | Value |
| --- | --- |
| `case` | The name of the case. It can contain `*`. |
| `kind` | The kind of the resource: `memory`, the function that takes the resource of a pair, or a kind of `kinds`. |
| `why` | The reason the case keeps the resource. |
| `system` | The system that the exemption applies on. An exemption with no `system` applies on each system. |

For example:

```toml
[[leaks.exception]]
case = "a library with no unload stays open"
kind = "os_lib_open"
why = "A backend library that cannot unload stays open, and the next world uses it."
```

`test --leaks` reports an exemption that states no reason, or that excuses no kept resource.
