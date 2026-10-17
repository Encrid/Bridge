# .toolchain/tests/cases/check.toml

The group `check` checks `.toolchain/tools/check.py`.

The case `every check` writes a small project into `{case}/p` with `files`, and `folders` copies `license`, `project.py` and `.toolchain/tools/check.py` into it.
The project has each file that the layout needs, a readme with the sections of Standard Readme, and the name `p`.
Thus each check passes on it.

| Case | Claim | False variant |
| --- | --- | --- |
| every check | `check.py` writes the name of each tool in the order of `TOOLS`, then `every check passes`, and ends with 0. | `check.py` checks `{case}`, which holds only the folder `p`. |
| a range of commits | `check.py` gives `--commits nothing..nowhere` to `commits.py`, which stops: `commits.py: git log nothing..nowhere`. | No `--commits`. |
| a tool that is not there | `folders` copies `check.py` and `shared.py` into `{case}/.toolchain/tools`, and `toolchain.toml` into `{case}/.toolchain`. That copy finds no other tool: `check.py: failed: layout text prose pseudo xref dup cover licence commits style c23`. | The `check.py` of the project checks `{case}`. Some checks pass there. |
