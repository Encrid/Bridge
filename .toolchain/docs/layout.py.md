# .toolchain/tools/layout.py

`layout.py` checks the tree of the project: the paths, the needed files, the documents and the names.
It reads the files that `ours` gives, so no rule of `layout.py` applies to a file that `ours` leaves out.

## Functions

| Function | Result |
| --- | --- |
| `guides(root)` | Gives the paths of the guides that `[check] guides` of `.toolchain/project.toml` adds. |
| `faults(root, paths)` | Gives the faults of the tree. |
| `main()` | Runs the check. |

## The paths

The layout allows only the paths of this table.
`<name>` is one name of a file or of a folder.
`**` is one name or more, with a `/` between two names.

| Path | Rule |
| --- | --- |
| `.github/workflows/ci.yml`, `.github/workflows/release.yml`, `.github/workflows/cla.yml` | Needed. `.toolchain/docs/workflows.py.md` tells what they run. |
| `.gitattributes`, `.gitignore`, `cla.md`, `license` | Needed. |
| `project.py`, `.toolchain/project.toml`, `readme.md` | Needed. |
| `architecture.md` | The map of the project. |
| `docs/contributing/<name>.md`, for each name of `GUIDES`: `agreement`, `coding_style`, `commits`, `decisions`, `documentation_style`, `needs`, `pull_request` and `tests` | Needed. |
| `docs/contributing/<name>`, for each name of `[check] guides` | A guide of the project. |
| `docs/extern/**.md`, `docs/intern/**.md`, `docs/tools/**.md` | The documents of the files. The section *The documents* gives their rules. |
| `include/**.h` | The public headers. |
| `src/**` | The sources, the private headers and the other inputs of a build. |
| `tools/**` | The tools of the project. |
| `third_party/**` | Any file. |
| `.toolchain/tools/<name>.py` | Needed: one file or more. The tools of the toolchain. |
| `.toolchain/toolchain.toml` and `.toolchain/header.txt` | The values and the legal notice of the toolchain. |
| `.toolchain/config/<name>.toml` | A file of the configuration of the project. |
| `.toolchain/docs/<name>.md` | A document of the toolchain. |
| `.toolchain/presets/<name>/<name>.toml` | A file of a preset. `.toolchain/docs/presets.md` tells the presets. |
| `<folder>/.gitkeep`, in `docs/extern`, `docs/intern`, `include` or `src` | An empty file that keeps an empty folder in git. |

`layout.py` does not read `tests`, and does not read `.toolchain/tests` of the repository `my_c23_template`. Thus they can hold any file.
`layout.py` makes no check on a file under `third_party`. So `third_party` holds any file, with any name.

The messages are:

- `<path>: the layout does not allow this file`
- `<path>: the layout needs this file`
- `<folder>/: the layout needs this folder`

## The documents

`shared.document` gives the document of a file: `.toolchain/docs/shared.py.md`, *The documents*.

The first line of a document is `#`, a space, and the path of its file.
A document of `docs/extern`, of `docs/intern` or of `docs/tools` is a fault if its file is not there.

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
