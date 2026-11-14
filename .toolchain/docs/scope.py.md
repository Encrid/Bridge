# .toolchain/tools/scope.py

`scope.py` gives the case groups that test the files that a pull request changes. The step `Gate` of `ci.yml` runs each gate only for these groups: `.toolchain/docs/workflows.py.md`, *The jobs of ci.yml*.

```sh
python .toolchain/tools/scope.py --changed <file> --into <branch>
```

`--changed` names a file with one changed path on each line, as `git diff --name-only` writes them. `--into` names the branch that the pull request goes into.
The tool writes one line with `--group <group>` for each group, in the order of the names. If no group tests the change, it writes nothing.

## The code of a group

A test belongs to the code that it tests. A group states that code with the key `code`, at the top of its file:

```toml
code = ["src/store/**", "tests/store/**"]
```

Each item is a pattern for a path of the project, with `*` for any text.
A group tests a path that one of its patterns matches. It also tests its own file, and no other group tests that file.

A group with no `code` tests the code of its part. A group in no part tests the code that is in no part, which is all the code of a project with no part.
The code of a part is its sources, its headers, its tests and its config file. Its documents do not count.
The code in no part is each other file of `src`, `include`, `tests`, `tools` and `.toolchain/config`, and the file `.toolchain/project.toml`.
A config file belongs to the part whose table `[parts.<part>]` it states alone.

## The groups of the whole

A group tests the project as a whole if its file states `whole = true`. Such a group has no code by default, so it does not run for a changed source.

A pull request into `main` runs only the groups of the whole. Each piece ran its own groups when it changed.
A project with no such group runs no case there. `check` and the package proof of `pack` still run.

## The scope

For a pull request into a different branch than `main`, each changed path gives the groups that test it.
A file of the copy of the toolchain that no group tests gives the groups of the whole. The cases of the toolchain run in the repository `mc23t`.
A file of `src`, `include`, `tests` or `tools` that no group tests stops the tool. Thus no code is left with no test.
Each other path gives no group: a document and a workflow change no code.

A part that uses a changed part gives no group. Its groups test its own code.

The tool stops with these faults:

- `name the list of the changed paths with --changed`.
- `cannot read the list <path>`.
- `<group file>: code is a list of patterns`.
- `<group file>: whole is true or false`.
- `<path>: no case group tests this file`.

A project with no case group gives nothing.

## Functions

| Function | Result |
| --- | --- |
| `main()` | Reads the options and the list, and writes the line. |
| `scoped(settings, root, paths, into)` | Gives the sorted names of the groups of the scope. |
| `groups(settings, root)` | Gives each group of `[test] cases` that has a case: its name, its file, its `code`, its `whole` and its part. |
| `tests(settings, root, found, group, path)` | Tells if a group tests a path. `found` holds each group of the project. |
| `claimed(settings, root, names)` | Gives the function that tells if the named groups hold each group that tests a file: `.toolchain/docs/branches.py.md`, *The files of the named groups*. |
| `owner(settings, root, path)` | Gives the part of one path, or `None`. |
| `stated(root, path)` | Gives the part that a config file states alone, or `None`. |
