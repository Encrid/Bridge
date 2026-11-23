# .toolchain/toolchain.toml

`.toolchain/toolchain.toml` holds the values of the tools.
It is the same in each project, and a project does not change it.
`shared.config` adds `.toolchain/project.toml` to it, and refuses a key that the two files give.

## [zig]

| Key | Value |
| --- | --- |
| `version` | The pinned version of zig. |
| `folder` | The folder that `fetch` fills with zig, from the folder of `project.py`. |
| `url` | The address of the archive. `fetch` puts the values in `{version}`, `{host}` and `{format}`. |
| `archives.<target>` | For a machine of the target: the `host` and the `format` of the archive, and its `sha256` digest. |

## [fetch]

| Key | Value |
| --- | --- |
| `archives` | The folder of the archives that `fetch` downloads: `.toolchain/docs/fetch.py.md`, *The shared archives*. |
| `timeout` | The seconds that a download waits for its next bytes. |

## [build]

| Key | Value |
| --- | --- |
| `folder` | The default output folder, in the root. |
| `cache` | The folder of the caches of zig, from the folder of `project.py`. |
| `job_memory` | The memory of 1 job, in bytes: *The jobs*. The default is 1610612736, 1.5 GiB. |

## [variables] and [debug]

| Key | Value |
| --- | --- |
| `language` | The flags of the language: C23, the pedantic errors, and each warning as an error. |
| `optimize` | The flags of the optimization. `-s` tells the linker to drop the debug information, also of the objects that zig builds for the C library. Without it, a Linux program holds the folder of the project. `[debug] optimize` gives the flags of a debug build, with the sanitizer of undefined behaviour. |
| `float` | The flags of the rules of floating point: `-ffp-contract=off`, `-fno-fast-math` and `-fno-rounding-math`. *The rules of floating point* tells why. |
| `strip` | The flag of `zig build-lib` for the debug information. `-fstrip` drops it, as `-g0` does for the compiler. `[debug] strip` is `-fno-strip`, which keeps it. |
| `debug_info` | The flag of the compiler for the debug information, for the kinds that do not take `optimize`, such as the kinds of wasm. It is `-g0`, so an object of a release holds no folder of the machine. `[debug] debug_info` is `-g`. |
| `jobs` | The most jobs that a tool runs at the same time: *The jobs*. The default is 8. |

## The jobs

`jobs` limits the jobs that run at the same time: the steps of `zig build`, and the syntax trees of `rewrite.py`.
`build.py` gives it to `zig build` as `-j<jobs>`: `.toolchain/docs/build.py.md`, *The runner zig*. `each` of `rewrite.py` runs that many trees at the same time.

The tools use the least of 3 counts, and at least 1 job.
The 3 counts are `jobs`, the processors of the machine, and the free memory divided by `[build] job_memory`.
The free memory is `free_memory` of `.toolchain/docs/shared.py.md`, when the run starts. Thus a machine that runs other builds or test runs runs fewer jobs, and no setting changes.

The default of `job_memory` is the largest compile that a run measured, 1.36 GB, rounded up to 1.5 GiB.
With no measure of the memory, the tools use the least of `jobs` and of the count of processors.
If `jobs` is not a whole number above 0, the tools stop: `[variables] jobs is <jobs>, and it must be a whole number above 0`.

The default 8 is the limit that `rewrite.py` had before the setting, so a machine with 8 processors or fewer runs as before.
Without `-j`, `zig build` runs a step on each processor. On a machine with more than 8 processors, the default runs fewer steps.
The copies of a case are no jobs: `[test] copies` gives their count, since the copies must run at the same time.

## The rules of floating point

With `-ffp-contract=on`, the compiler can join a product and a sum into 1 fused multiply-add, with 1 rounding in place of 2.
A processor with such an instruction then gives different bits than a processor with no such instruction.
Thus `float` holds `-ffp-contract=off`, so that the same source gives the same results on each processor, as the need Deterministic of `docs/contributing/needs.md` asks.
A function that needs a fused multiply-add calls `fma` of C23, which gives the same result on each machine.

Each kind of `.toolchain/toolchain.toml` that compiles C passes `{flags}` after `{float}`. The kind `native_module` of the preset `native_module` does too, and so do the kinds `wasm_module` and `wasm_library` of the preset `wasm_module`, through `wasm_compile`.
Thus an artifact can override the default with `-ffp-contract=on` in its `flags`.
An artifact overrides the default only for a gain that a measure shows. The document of its sources states that measure as the reason.

## [targets.\<target\>]

There are six targets: three operating systems on two types of processor. Each project picks its targets with `[project] targets`: `.toolchain/docs/project.toml.md`, *[project]*.

| Key | Value |
| --- | --- |
| `runs_on` | The system and the processor that Python gives for a machine of the target. An empty list means: build only. |
| `system` | The family of the system: `posix` or `windows`. It is also the variable `{system}`. |
| `triple` | The target for zig, with the range of the versions of the system. |
| `cpu` | The model of the processor. |
| `exe` | The extension of a program. |
| `library` | The extension of a shared library. |
| `define` | The macros that the system needs. |
| `pic` | The flags for code that a shared library can hold. |
| `rpath` | The flags that tell a program to find a library in its own folder. |
| `soname` | The flags that give a shared library its name, without its folder. |
| `stack` | The flag of the linker for the size of the stack. |
| `errno` | The flag that tells if a function of the math library sets `errno`. |
| `runtime` | The sources that each shared library of the target adds to its own: `.toolchain/docs/windows_x86_64.h.md`. Only `x86_64-windows-gnu` has them. Each other target states an empty list. |

`.toolchain/docs/windows_x86_64.h.md`, *Why*, tells why only `x86_64-windows-gnu` needs a runtime.

## [kinds.\<kind\>]

| Kind | Output | Steps |
| --- | --- | --- |
| `program` | `{name}{exe}` | `each`: `zig cc -c` compiles one source. Then `zig cc` links the objects with the files of `link`. |
| `library` | `{name}{library}` | `each`: `zig cc -c` compiles one source, with `{pic}`. Then `zig cc -shared` links the objects, and compiles `runtime`, into a shared library. A program gets no `runtime`. |
| `archive` | `{name}.a` | `each`: `zig cc -c` compiles one source. Then `zig build-lib -static` puts the objects into a static library, with `{strip}`. Without `-fstrip`, zig asks the compiler for CodeView, and each object of a Windows archive holds its own path. |

Each step `each` passes `-MD -MF {deps}`, so zig knows the headers of each source.
It also passes `-Wno-unused-command-line-argument`, because `-c` uses no flag of the linker, for example `-s`.
| `file` | `{name}` | It copies the one source to the output. |

A kind can also have these keys:

| Key | Value |
| --- | --- |
| `flags` | The flags that the kind adds to `{flags}` of each artifact of the kind. |
| `dump` | The command that gives the syntax tree of one source of the kind. With no `dump`, the kind uses `[rewrite] dump`. |
| `probed` | The flags that the kind adds to `{flags}` only when it builds the tree of a run of `rewrite.py`: `.toolchain/docs/rewrite.py.md`. They can name `{tree}`, the folder of that tree. The syntax tree does not get them. |

## [rewrite]

`dump` is the command that gives the syntax tree of one source as JSON, for a changed copy of the code.
It has the variables of the kinds, and `{source}`, the path of the source.
Thus `{flags}` gives it the flags that the kind adds, for example the folders of the headers of a source.

It does not report a flag that it does not use, since it compiles nothing.
Its `-o` names a file in the folder `obj`. Without it, zig would leave an empty `a.out` in the root of the project.
`.toolchain/docs/rewrite.py.md` tells how the tree is used.

## [style]

These keys are the limits of the style of the code. The check of the style reads them.

| Key | Value |
| --- | --- |
| `columns` | The maximum number of columns in a line. |
| `lines` | The maximum number of lines in the body of a function. |
| `functions` | The maximum number of functions in a file. |
| `arguments` | The maximum number of arguments of a function. |
| `branches` | The maximum number of branches in one `if`. |
| `depth` | The maximum depth of nested branches. |

## [prose]

These keys are the tags and the limits of the documents. The check of the documents, `.toolchain/tools/prose.py`, reads them.

| Key | Value |
| --- | --- |
| `tags` | The tags that a block of code can have. `[check] tags` of the configuration adds more. |
| `sentence` | The maximum number of words in a sentence. |
| `step` | The maximum number of words in a sentence of an item of a numbered list. |
| `paragraph` | The maximum number of sentences in a paragraph, in an item of a list and in a cell of a table. |
| `description` | The maximum number of characters in the description of the readme. Standard Readme asks for fewer than 120. |

## [dup]

| Key | Value |
| --- | --- |
| `words` | The minimum number of words of a passage that `.toolchain/tools/dup.py` reports. |

## [said]

| Key | Value |
| --- | --- |
| `own` | The minimum number of own characters of a format that `.toolchain/tools/said.py` checks. |

## [commits]

| Key | Value |
| --- | --- |
| `types` | The types that the line of a commit can have, for `.toolchain/tools/commits.py`. |
| `words` | The maximum number of words in the description of a commit. |
