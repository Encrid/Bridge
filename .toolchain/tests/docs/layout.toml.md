# .toolchain/tests/cases/layout.toml

The group `layout` checks `.toolchain/tools/layout.py`.
Each case writes a project into `{case}/project` with `files`, and runs `layout.py` on that project.
Each file of a project is empty, but a document holds its first line.
The project of each case has a `.toolchain/project.toml`, because `config` reads it.

The false variant of a case changes or adds a file of the project.
If the fault is a path, the false variant runs `layout.py` on `{case}/fixed`, which holds the project without that path.

The project of the case `a full project` has these files, and the document of each file that needs one:

| Folder | Files |
| --- | --- |
| The root, `.github` and `docs/contributing` | Each other file that the layout needs. |
| `.toolchain` | `project.toml`, `toolchain.toml`, `header.txt`, `tools/check.py`, `docs/check.py.md`, and the case file `tests/cases/Any Name.toml`. |
| `tools` | `release.py`, with its document `docs/tools/release.py.md`. |
| `src` | `sample.c` and `net/socket.c`. |
| `include` | `sample.h`. |
| `third_party/sample` | `fix.patch` and `license`. |
| `include`, `src`, `docs/extern` and `docs/intern` | `.gitkeep`. |
| `tests` | `Any Name.txt`. |

| Case | Claim | False variant |
| --- | --- | --- |
| a full project | `layout.py` writes `no fault in 34 files`, and ends with 0. `ours` leaves out the files of `tests` and of `.toolchain/tests`. | The project also has `notes.txt`. |
| a path that the layout does not allow | `notes.txt: the layout does not allow this file`. | No `notes.txt`. |
| a file that the layout needs | A project with no `license`: `license: the layout needs this file`. | The project has `license`. |
| the folder that the layout needs | A project with no file in `.toolchain/tools`: `.toolchain/tools/: the layout needs this folder`. No line names `src/`. | The project has a file in `.toolchain/tools`. |
| a source without its document | `src/a.c: no document docs/intern/a.c.md`. | The project has the document. |
| a header without its document | `include/a.h: no document docs/extern/a.h.md`. | The project has the document. |
| a document without its file | `docs/intern/gone.c.md: no file has this document`. | The project has the source `gone.c` in `src`. |
| a document in the wrong folder | A source has a document in `docs/extern`: `docs/extern/a.c.md: no file has this document`. | The source has no document in `docs/extern`. |
| an extra document in docs | `docs/contributing/notes.md: the layout does not allow this file`. | No `notes.md`. |
| the first line of a document | The document of a source starts with `# a.c`: `the first line is not # src/a.c`. | The first line is `# src/a.c`. |
| a name that is not in snake_case | `src/Notes.c: 'Notes.c' is not in snake_case`. | `notes.c` in place of `Notes.c`. |
| a compiled file | `src/sample.o: a compiled or temporary file`. | `sample.c` in place of `sample.o`. |
| an empty folder | `include`, `src`, `tools`, `docs/extern` and `docs/intern` hold only `.gitkeep`. No line names `.gitkeep`. | The project also has `notes/.gitkeep`, which the layout does not allow. |
| a file of tests | `Any Name.txt` in `tests` is not in snake_case. No line names it. | The file is in `src`, not in `tests`. |
| a tool without its document | `tools/a.py` has no document: `tools/a.py: no document docs/tools/a.py.md`. | `docs/tools/a.py.md` is there. |
| a document of a tool that is not there | `docs/tools/gone.py.md` has no file: `docs/tools/gone.py.md: no file has this document`. | `tools/gone.py` is there. |
| a guide that [check] guides names | `[check] guides` names `timings.md`, so the layout allows `docs/contributing/timings.md`. The output does not name it. | `.toolchain/project.toml` is empty. |
| third_party holds anything | A compiled file under `third_party` makes no fault. No line names `bridge-host.exe`. | The file is in `bridge`, not under `third_party`. |
