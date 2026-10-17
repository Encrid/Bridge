# .toolchain/tools/check.py

`check.py` runs every check of the project, and tells which checks failed.
The command `check` of `project.py` runs it.

## main

`main` does these steps:

1. It reads the options `--root` and `--commits`.
2. It runs each tool of `TOOLS` in the order of `TOOLS`, with the Python that runs `check.py`.
3. Before a tool runs, `main` writes the name of the file of the tool.
4. It gives `--root <root>` to each tool. It gives `--commits <range>` only to `commits.py`.
5. A tool fails if it ends with a status that is not 0.
6. If a tool failed, `main` stops: `failed: <tools>`. Else it writes `every check passes`.

`<tools>` is the names of the tools that failed, in the order of `TOOLS`, with a space between two names.

`TOOLS` names the tools in this order: `layout`, `text`, `prose`, `pseudo`, `xref`, `dup`, `cover`, `licence`, `commits`, `style` and `c23`.
The document of a tool is `.toolchain/docs/<tool>.py.md`.

`main` runs the tools of the folder `.toolchain/tools` in `HERE`. Thus these tools check the project in the root.
A tool that is not there fails, because Python cannot run it.
A failure does not stop the other checks. Thus the output shows the faults of every check.
