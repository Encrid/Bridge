# .toolchain/tests/cases/xref.toml

The group `xref` checks `.toolchain/tools/xref.py`.
Each case writes a small project into `{case}/p` with `files`, and runs `xref.py --root {case}/p`.

The text of `{case}/p/readme.md` starts with `# p` and an empty line. Thus its third line holds the fault.
In this document, each path of the small project starts with `{case}/p`.

| Case | Claim | False variant |
| --- | --- | --- |
| a path that is not in the project | The readme names a file of `src` that is not there. `xref.py` writes `readme.md:3: src/gone.c is not a file of the project`. | `{case}/p/src/gone.c` is there. |
| a path into include | The readme names `include/gone.h`. `xref.py` writes `readme.md:3: include/gone.h is not a file of the project`. | `{case}/p/include/gone.h` is there. |
| a citation of a document that is not there | The readme cites the heading `Usage` of `notes.md`. `xref.py` writes `readme.md:3: notes.md is not a file of the project`. | `{case}/p/notes.md` is there, with the heading `Usage`. |
| a citation of a heading that is not there | The readme cites the heading `Usage` of `{case}/p/docs/a.md`, which has the heading `Use`. `xref.py` writes `readme.md:3: docs/a.md has no heading Usage`. | `{case}/p/docs/a.md` has the heading `Usage`. |
| a citation on two lines | The same citation continues on the fourth line. `xref.py` writes `readme.md:3: docs/a.md has no heading Usage`. | `{case}/p/docs/a.md` has the heading `Usage`. |
| a reference to a heading that is not there | The readme refers to its heading `Usage`. `xref.py` writes `readme.md:3: this document has no heading Usage`. | The readme has the heading `Usage`. |
| a path in a fenced block | The readme names a file of `src` that is not there, in a fenced block. `xref.py` ends with 0. | The path is out of the block. |
| a path out of the folders of the layout | The readme names `build/gone.c`. `xref.py` ends with 0. | The path starts with `tools` in place of `build`. |
| a folder | The readme names `src/gone`, a folder that is not there. `xref.py` ends with 0. | The path has the extension `.c`. |
| a document of tests | `{case}/p/tests/a.md` names a file of `src` that is not there. `xref.py` ends with 0. | `{case}/p/docs/a.md` has the same text. |
