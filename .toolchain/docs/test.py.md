# .toolchain/tools/test.py

`test.py` builds the project with `build.py`, then runs the cases of `[test] cases`, which is `tests/cases` by default.

## Functions

| Function | Result |
| --- | --- |
| `environment(changes, values)` | Gives the environment of a command, with the changes of `env`. |
| `free_port()` | Gives a free port of this machine. |
| `started(argv, root, timeout, env)` | Starts a command, with a timer that stops it after `timeout` seconds. |
| `waited(running, text)` | Reads the output until a line contains the text. It gives false if the output ends first. |
| `typed(running, text)` | Writes a text to the command. |
| `ended(running, text)` | Writes the text, closes the input, and waits for the end of the command. It gives the status and the output. |
| `ordered(wanted, lines)` | Tells if the lines have each wanted line, in order. |
| `holding(path, text)` | Tells if a file is there and contains the text. |
| `same(path, said)` | Tells if a file is there and holds the same text as the output. |
| `holds(claims, lacks, code, said, values)` | Tells if each claim is true. |
| `source_of(item, outputs, values)` | Gives the file of an artifact, or a file of the project. |
| `placed(use, outputs, values)` | Writes the `files` of a case, and fills its `folders`. |
| `scripted(running, script, outputs, values)` | Does the steps of a script. |
| `made_false(case)` | Gives the false variant of a case. |
| `tried(config, values, outputs, where, case, broken)` | Runs one case. |
| `judged(config, group, broken, run)` | Gives the verdict of one run of a case. |
| `verdict(config, values, outputs, group, index, case, broken, copies)` | Runs the copies of one case at the same time, and gives their verdict. |
| `test(config, values, outputs, groups, broken, said, copies)` | Runs all the groups, and writes each verdict. With `said`, it writes `said.txt`. It gives the names of the cases that failed. |
| `chosen(groups, part)` | Gives only the cases whose name contains `part`. It stops if no case has it: `no case has <part> in its name`. |
| `groups_of(values, folder)` | Reads and checks the groups of a folder: `tests/cases`, or `.toolchain/tests/cases`. |
| `measured(args)` | Runs the command `test --branches`, as *The branches* tells. |
| `main()` | Runs the command `test`. |

## main

With `--branches`, `main` runs `measured`, and nothing else. Else `main` does these steps:

1. It builds with `build`, which stops if the target does not run on this machine. With `--toolchain`, it only calls `prepared` of `build.py`, and builds nothing. `--part` builds the base and the named parts, and repeats.
2. It stops if an artifact is not linked: `the cases need what is linked: <names>`.
3. It reads the groups of `[test] cases` (the default is `tests/cases`), or of `.toolchain/tests/cases` with `--toolchain`.
4. If there is no case, it writes `no case: <folder> holds no case`, and ends.
5. With `-k`, it keeps only the cases whose name contains the part.
6. With `--part`, it keeps only the groups of the named parts. It stops if no group is in them: `no case group is in the parts <names>`.
7. It runs the groups, one time for each round of `--rounds`. Before each round of 2 rounds or more, it writes `round <n> of <rounds>`.
8. With no `--break`, `--toolchain`, `-k` or `--part`, it checks the messages of the code with `checked` of `said.py`.

The number of copies is `--copies`, or `[test] copies`. If neither gives it, `main` stops: `.toolchain/project.toml: write [test] copies`.

`main` stops at step 7 if a case failed. Thus step 8 runs only if each case passed.
`.toolchain/docs/said.py.md` gives the checks of step 7.

The message of step 1 is `<target> does not run here: build it, and test it where it runs`.

## Groups

`groups_of` reads each file `**.toml` of its folder, in the order of the paths.
The name of a group is the path of its file in `tests/cases`, without `.toml`.
A group is in a part if the name of the group is the part, or begins with the part and a slash.
A file with no case gives no group.
`groups_of` stops if a case:

- has a key that a case does not have, or its false variant or its helper has such a key,
- claims nothing: `<path>: the case <name> claims nothing`,
- has no false variant: `<path>: the case <name> has no false variant`.

## A case

`tried` runs one case:

1. It empties the folder of the case, `<out>/cases/<group>/<number>`, or `<out>/cases/<group>/<number>/<copy>` for a copy.
2. It adds the variables `{case}` and `{port}`. `{port}` is a new free port.
3. With `--break`, it uses the false variant of the case.
4. It writes the files, and fills the folders of the case.
5. If the case has a helper, it starts the helper, and waits for its text `wait`.
6. It starts the command, and does the steps of the script.
7. It writes `stdin` to the command, and waits for the end of the command.
8. If the case has a helper, it writes `stdin` of the helper, and waits for the end of the helper.

`tried` runs each command in the root, with a timer of `[test] timeout` seconds.
The timer stops a command that does not end in time.
`placed` and the steps `put` copy each file with its permissions. Thus a copy of a program can run on Linux and on macOS.

`made_false` makes the false variant: the keys of `false` replace the keys of the case.
For `files` and for `folders`, each path of `false` replaces only that path.

## Verdicts

`verdict` runs the copies of a case at the same time, in threads: `[test] copies` or `--copies` copies, or one copy if the case has `alone = true`.
`judged` gives the verdict of each copy. It adds the `lacks` of each rule to the claims, but not the rules that exempt the group.
The case passes only if each copy passes. If copies fail, the verdict of the first copy that failed ends with `in <failed> of <copies> copies`.

| Run | The claims | Verdict | The case |
| --- | --- | --- | --- |
| `test` | true | `as written` | passes |
| `test` | false | `differs` | fails |
| `test --break` | false | `fails, as it must` | passes |
| `test --break` | true | `still as written` | fails |
| both | a command does not end in time | `did not end in <timeout> seconds` | fails |

`test` writes `<case>: <verdict>` for each case. With `--break`, it writes `<case>, made false: <verdict>`.
For a case that fails, it writes the status and the output of each command.
After each group, it writes the number of its cases and the number of the cases that passed.

In a run with no `--break`, no `--toolchain` and no `-k`, `test` then writes `<out>/said.txt`.
The file holds the output of each command and of each helper, in the order of the cases.
`said.py` reads it. A run with `--break` writes no `said.txt`, because a false variant does not run as the project runs.
If a case failed, `test` stops with the names of the cases that failed, after it writes `said.txt`.

## Claims

`holds` tells if each claim of a case is true. It replaces the variables in each text of the claims.
The output of a command is what it writes to its output and to its errors, without the carriage returns.
A claim `line` compares full lines. A claim `order` ignores the spaces at the start and at the end of a line.

## The branches

`measured` does these steps:

1. It stops with one of these options: `--break`, `--toolchain`, `--copies` or `--rounds`.
2. It does the steps 1 to 10 of a build with `ready` of `build.py`.
3. It sets the output folder of the target to `<build>/branches/<target>`.
4. It writes the sources with their probes, with `probed` of `branches.py`.
5. It builds the artifacts from these sources, with `made` of `build.py`.
6. It stops if an artifact is not linked, as step 2 of *main* tells.
7. It reads the groups of `tests/cases`. With `-k`, it keeps only the cases whose name contains the part.
8. It sets the variable `C23_BRANCHES` of the environment to `<build>/branches/<target>/hits.txt`.
9. It runs each case one time, and writes its verdict. A case that fails does not stop it.
10. It writes the report, with `reported` of `branches.py`.

The message of step 1 is `--branches runs each case one time: it takes no --break, --toolchain, --copies or --rounds`.
`ready` stops at step 2 if the target does not run on this machine.
`.toolchain/docs/branches.py.md` tells what the probes measure, and what the report contains.
