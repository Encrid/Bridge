# .toolchain/tools/scope.py

`scope.py` gives the parts that a pull request changes, and each part that uses them. The step `Gate` of `ci.yml` runs each gate only for these parts: `.toolchain/docs/workflows.py.md`, *The jobs of ci.yml*.

```sh
python .toolchain/tools/scope.py --changed <file> --into <branch>
```

`--changed` names a file with one changed path on each line, as `git diff --name-only` writes them. `--into` names the branch that the pull request goes into.
The tool writes one line with `--part <part>` for each part, in the order of the names. For the whole project, it writes nothing.

## The part of a path

A file of a part is in the folder of the part, or is its readme or its config file.

1. A path under `src/`, `include/`, `docs/extern/`, `docs/intern/`, `tests/cases/` or `tests/` loses that prefix. The first prefix that matches wins.
2. `docs/usage/<folder>.md` gives the folder of the readme.
3. `part_of` of `shared.py` then gives the part whose folder holds the rest of the path, the longest folder first.
4. A file of `.toolchain/config` belongs to the part whose table `[parts.<part>]` it states. A file with no such table, or with more than one, belongs to no part.

## The scope

1. A part that uses a changed part joins. Then a part that uses a joined part joins, until no part joins.
2. The tool writes nothing for a project with no part, for a path of no part, or for an empty list.
3. It writes nothing with `--into main` too, since each test command runs for the whole project when `dev` goes into `main`.

The messages are `name the list of the changed paths with --changed` and `cannot read the list <path>`.

## Functions

| Function | Result |
| --- | --- |
| `main()` | Reads the options and the list, and writes the line. |
| `scoped(settings, root, paths, into)` | Gives the sorted parts of the scope, or no part for the whole project. |
| `owner(settings, root, path)` | Gives the part of one path, or `None`. |
| `stated(root, path)` | Gives the part that a config file states alone, or `None`. |
| `users(settings, named)` | Gives the changed parts, and the parts whose `uses` lead to them at any depth. |
