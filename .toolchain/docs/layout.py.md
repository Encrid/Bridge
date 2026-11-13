# .toolchain/tools/layout.py

`layout.py` checks the tree of the project: the paths, the needed files, the documents and the names.
It reads the files that `ours` gives, so no rule of `layout.py` applies to a file that `ours` leaves out.

## Functions

| Function | Result |
| --- | --- |
| `own(root)` | Gives the paths of the guides that `[check] guides` of the configuration adds, and the path of the readme of each part. |
| `faults(root, paths)` | Gives the faults of the tree. |
| `main()` | Runs the check. |

## The paths

The layout allows only the paths of this table.
`<name>` is one name of a file or of a folder.
`**` is one name or more, with a `/` between two names.

| Path | Rule |
| --- | --- |
| `.github/workflows/ci.yml`, `.github/workflows/release.yml`, `.github/workflows/cla.yml`, `.github/workflows/warm.yml` | Needed. `.toolchain/docs/workflows.py.md` tells what they run. |
| `.gitattributes`, `.gitignore`, `cla.md`, `license` | Needed. |
| `project.py`, `.toolchain/project.toml`, `readme.md` | Needed. |
| `architecture.md` | The map of the project. |
| `toolchain.yml` in `.github/workflows` | The workflow of the template, which runs the cases of the toolchain. A project does not need it, so this document names no path of it. |
| `docs/contributing/<name>.md`, for each name of `GUIDES`: `agreement`, `coding_style`, `coding_style_cxx`, `commits`, `decisions`, `documentation_style`, `needs`, `pull_request` and `tests` | Needed. |
| `docs/contributing/<name>`, for each name of `[check] guides` | A guide of the project. |
| `docs/usage/<folder>.md`, for the `folder` of each part of `[parts]` | Needed. The readme of the part. |
| `docs/extern/**.md`, `docs/intern/**.md`, `docs/tools/**.md` | The documents of the files. The section *The documents* gives their rules. |
| `docs/<folder>/**`, for a `<folder>` that is not `contributing`, `extern`, `intern` or `tools` | Any file. A project organizes its other documents as it wants. |
| `include/**.h`, `include/**.hpp` | The public headers: a `.h` for C, or for C and C++, and a `.hpp` for C++ only. |
| `src/**` | The sources, the private headers and the other inputs of a build. |
| `tools/**` | The tools of the project. |
| `third_party/**` | Any file. |
| `.toolchain/tools/<name>.py` | Needed: one file or more. The tools of the toolchain. |
| `.toolchain/toolchain.toml` and `.toolchain/header.txt` | The values and the legal notice of the toolchain. |
| `.toolchain/config/<path>.toml` | A file of the configuration of the project, also in a folder of `.toolchain/config`. |
| `.toolchain/docs/<name>.md` | A document of the toolchain. |
| `.toolchain/presets/<name>/<name>.toml` | A file of a preset. `.toolchain/docs/presets.md` tells the presets. |
| `.toolchain/runtime/<name>.c`, `.toolchain/runtime/<name>.h` | A source or a header that the toolchain adds to the shared libraries of a target: `.toolchain/docs/toolchain.toml.md`, *[targets.\<target\>]*. |
| `<folder>/.gitkeep`, in `docs/extern`, `docs/intern`, `include` or `src` | An empty file that keeps an empty folder in git. |

`layout.py` does not read `tests`, and does not read `.toolchain/tests` of the repository `mc23t`. Thus they can hold any file.
`layout.py` makes no check on a file under `third_party`. So `third_party` holds any file, with any name.

The messages are:

- `<path>: the layout does not allow this file`
- `<path>: the layout needs this file`
- `<folder>/: the layout needs this folder`

## The documents

`shared.document` gives the document of a file: `.toolchain/docs/shared.py.md`, *The documents*.
Each document keeps the path of its file below the first folder. Thus the documents of a part are in the folders of the part.
For example, the document of `include/<folder>/<name>.h` is `docs/extern/<folder>/<name>.h.md`, and the document of `src/<folder>/<name>.c` is `docs/intern/<folder>/<name>.c.md`.
`.toolchain/docs/project.toml.md`, *[parts]*, tells the parts of a project.

The first line of a document is `#`, a space, and the path of its file.
A document of `docs/extern`, of `docs/intern` or of `docs/tools` is a fault if its file is not there.
The readme of a part is in `docs/usage`, outside these 3 folders. Thus it needs no file.

The messages are:

- `<path>: no document <document>`
- `<document>: no file has this document`
- `<document>: the first line is not # <path>`

## The names

Each name of a path is in snake_case.
Such a name has lower-case letters, digits and `_`, then each extension: a `.`, then lower-case letters and digits.
`.github`, `.gitattributes`, `.gitignore` and `.gitkeep` are the only exceptions.

A file that a build or an editor makes is a fault. Its name has one of these forms:

- It ends with `.o`, `.obj`, `.a`, `.so`, `.dll`, `.dylib`, `.exe`, `.gch`, `.d` or `.pyc`.
- It ends with `~`, or it starts and ends with `#`.
- It is `a.out`.

The messages are:

- `<path>: '<name>' is not in snake_case`
- `<path>: a compiled or temporary file`

## Output

A fault of `layout.py` has no line, so its faults follow the paths only.
The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.
