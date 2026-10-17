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

## [build]

| Key | Value |
| --- | --- |
| `folder` | The default output folder, in the root. |
| `cache` | The folder of the caches of zig, from the folder of `project.py`. |

## [variables] and [debug]

| Key | Value |
| --- | --- |
| `language` | The flags of the language: C23, the pedantic errors, and each warning as an error. |
| `optimize` | The flags of the optimization. `[debug] optimize` gives the flags of a debug build, with the sanitizer of undefined behaviour. |
| `float` | The flags of the rules of floating point. |

## [targets.\<target\>]

There are six targets: three operating systems on two types of processor.

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

## [kinds.\<kind\>]

| Kind | Output | Steps |
| --- | --- | --- |
| `program` | `{name}{exe}` | `zig cc` compiles the sources, and links them with the files of `link`. |
| `library` | `{name}{library}` | `zig cc -shared` compiles the sources into a shared library. |
| `archive` | `{name}.a` | `zig build-lib -static` compiles the sources into a static library. It does not report a flag that it does not use. |
| `file` | `{name}` | It copies the one source to the output. |

## [branches]

`dump` is the command that gives the syntax tree of one source as JSON, for `test --branches`.
It has the variables of the kinds, and `{source}`, the path of the source.
It does not report a flag that it does not use, since it compiles nothing.
Its `-o` names a file in the folder `obj`. Without it, zig would leave an empty `a.out` in the root of the project.
`.toolchain/docs/branches.py.md` tells how the tree is used.

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

These keys are the limits of the documents. The check of the documents, `.toolchain/tools/prose.py`, reads them.

| Key | Value |
| --- | --- |
| `sentence` | The maximum number of words in a sentence. |
| `step` | The maximum number of words in a sentence of an item of a numbered list. |
| `paragraph` | The maximum number of sentences in a paragraph, in an item of a list and in a cell of a table. |
| `description` | The maximum number of characters in the description of the readme. Standard Readme asks for fewer than 120. |
