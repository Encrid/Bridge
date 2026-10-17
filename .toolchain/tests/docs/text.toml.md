# .toolchain/tests/cases/text.toml

The group `text` checks `.toolchain/tools/text.py`.
Each case runs `text.py` on `{case}`. `files` writes the files of the case, and `folders` copies the other files.
The false variant of a case changes the file that has the fault.

| File | Content |
| --- | --- |
| `.toolchain/project.toml` in `.toolchain/tests/data/text` | An empty table `[project]`. Each case copies it, or the next file. |
| `.toolchain/project.toml` in `.toolchain/tests/data/text/unicode` | The same values, and a `[check] unicode` that names `docs/*.md`. |

The project `clean` has five files with no fault:

- `.toolchain/project.toml`,
- the `project.py` of this project, which starts with the legal notice,
- `a.toml`, and `ci.yml` in `.github/workflows`, which each have a `#` in a string,
- the document `a.md` in `docs`.

A Python file of a case has no legal notice if the fault of the case is a different fault. Thus the fault is on line 1.

| Case | Claim | False variant |
| --- | --- | --- |
| files with no fault | `text.py` checks `clean`, writes `no fault in 5 files`, and ends with 0. | `a.toml` has a comment. |
| a character that is not ASCII | A line of `a.md` has the letter U+00E9: `docs/a.md:3: a character that is not ASCII`. | The letter `e`. |
| a file that [check] unicode names | `[check] unicode` names `docs/*.md`, thus the letter U+00E9 in `a.md` is not a fault. | `.toolchain/project.toml` has no `[check]`. |
| a Python file without the legal notice | `project.py:1: the file does not start with the legal notice`. | `text.py` checks `{case}/fixed`, which has the `project.py` of this project. |
| a notice with two holders | A second line of copyright, with a different holder, is not a fault. | The second line has no `Copyright (C)`. |
| a notice of a different holder | The line of copyright names `Someone`. It is not a fault. | The notice has no line of the licence. |
| a comment in a Python file | `project.py:1: a comment`. | No comment. |
| a string statement | A docstring: `project.py:1: a string statement`. | No docstring. |
| a comment in a TOML file | `a.toml:1: a comment`. | The `#` is in the string. |
| a comment in a YAML file | `.github/workflows/ci.yml:1: a comment`. | The `#` is in a string. |
| a comment in a document | `a.md` has an HTML comment: `docs/a.md:3: a comment`. | No HTML comment. |
| a retired word | `[check.retired]` gives `utilise` the new word `use`, and `a.md` holds `utilise`: `docs/a.md:3: 'utilise' is retired: write 'use'`. | `a.md` holds `use`. |
| a retired word with no new word | `[check.retired]` gives `whilst` an empty text, and `a.md` holds `Whilst`: `docs/a.md:3: 'Whilst' is retired`. | The table gives the new word `while`. |
| a word that holds a retired word | `a.md` holds `utilised`, which is not the whole word `utilise`. `text.py` writes `no fault in 2 files`. | `a.md` holds `utilise`. |
| a retired word in the tests | `tests/a.md` holds `utilise`: `tests/a.md:1: 'utilise' is retired: write 'use'`. | `tests/a.md` holds `use`. |
| a retired word in the toolchain | `.toolchain/docs/a.md` holds `utilise`, and so does the table in `.toolchain/project.toml`. Neither is a fault: `no fault in 2 files`. | `docs/a.md` holds `utilise` too. |
