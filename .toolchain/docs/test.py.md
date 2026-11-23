# .toolchain/tools/test.py

`test.py` builds the project with `build.py`, then runs the cases of `[test] cases`, which is `tests/cases` by default.

## Functions

| Function | Result |
| --- | --- |
| `environment(changes, values)` | Gives the environment of a command, with the changes of `env`. |
| `common_env(config)` | Gives `[test] env`, or an empty table. A value that is no text and no `false` stops the run: `[test] env: write a table of names, each with a text or false`. |
| `bound(kind, port)` | Binds a socket of a kind, TCP or UDP, on `127.0.0.1` and a port. It gives the bound port, or 0 if the system refuses the port. Port 0 asks the system for a free port. |
| `free_port()` | Gives a port of this machine that is free for TCP and for UDP. UDP gives a free port, and TCP must accept the same number. Else it tries again. |
| `watched(timeout, stop)` | Starts a timer that calls `stop` after `timeout` seconds, and gives it. |
| `started(argv, root, timeout, env, use)` | Starts a command with `started` of `signals.py`, with a timer of `watched` that stops it after `timeout` seconds. `use` is the case, or nothing for a helper. |
| `waited(running, text)` | Reads the output until a line contains the text. It gives false if the output ends first. |
| `typed(running, text)` | Writes a text to the command. |
| `fed(running, text)` | Writes the text with `typed`, and closes the input. |
| `paced(running)` | Reads the output line by line. At each empty line, which ends the answer to a row, it starts the timer again. |
| `ended(running, text, rows)` | Starts `fed` on a thread of its own, reads the output, and waits for the end of the command. With `rows`, it reads the output with `paced`. It gives the status and the output. It reads no output that a step closed. |
| `ordered(wanted, lines)` | Tells if the lines have each wanted line, in order. |
| `holding(path, text)` | Tells if a file is there and contains the text. |
| `same(path, said)` | Tells if a file is there and holds the same text as the output. |
| `holds(claims, lacks, code, said, values)` | Tells if each claim is true. |
| `claimed(claims, lacks, code, answer, values)` | Tells if each claim of a row is true on its answer, as *A table* tells. |
| `source_of(item, outputs, values)` | Gives the file of an artifact, or a file of the project. |
| `placed(use, outputs, values)` | Fills the `folders` of a case with `put`, and then writes its `files`. It removes a file first, so that it never writes into a linked file. It gives the stamp of each file that it linked. |
| `put(source, target, values)` | Places a file: a link to a file of the build folder, or else a copy. It removes the target first, and tells if it made a link. |
| `stamp(path)` | Gives the size and the time of the last write of a file, or nothing if it is not there. |
| `sources_of(item, outputs, values)` | Gives the files of 1 item of `folders`: the artifact or the file, or each file of the project that a pattern with `*` matches. It stops the run if a pattern matches no file. |
| `cleared(folder)` | Removes a folder with all its files, also a file that is read-only on Windows, such as an object of git. It changes the mode of no other file, so a link keeps the mode of its build output. |
| `renamed(source, target)` | Renames a file onto its target. On Windows, it renames with the POSIX semantics, so the rename succeeds while a program holds the target open with a share of the delete. If the file system refuses it, it renames with `os.replace`. |
| `scripted(running, script, outputs, values)` | Does the steps of a script. |
| `expanded_hold(hold, values)` | Gives the hold with the variables replaced in `file`, `call` and `until`: *The hold*. |
| `held(hold, values)` | Gives the text of a hold in `C23_FAIL`, or the empty text with no hold: *The hold*. |
| `hold_checked(hold, name, path, calls, values)` | Stops if the hold of the case `name` of the file `path` has a fault: *The hold*. |
| `made_false(case)` | Gives the false variant of a case. |
| `typed_rows(use, broken, order, values)` | Gives the text that `test` writes to the command of a table. It is the input of each row of `order`, each on one line. With `broken`, it is the false input. |
| `tried(config, values, outputs, where, case, broken, counted, failing, part, order)` | Runs one case. With `order`, the case is a table, and the command gets the inputs of its rows in that order. With `counted`, it sets the variable `C23_LEAKS` of the case. `failing` gives the variables and the artifacts of the run `fail`, or `None`. `part` is the part of the case, or `False`. |
| `judged(config, group, broken, run)` | Gives the verdict of one run of a case. |
| `answered(config, group, broken, run, rows, order)` | Gives the verdict of each row of a table from one run, as *A table* tells. |
| `joined(founds, count)` | Gives one verdict for each row from the verdicts of the copies of a table, as *A table* tells. |
| `raced(config, values, outputs, group, where, case, broken, count, counted, failing, order)` | Runs `count` copies of a table at the same time, with the rows in `order`. It gives the verdict of each row, and the output of the first copy that failed. |
| `tabled(config, values, outputs, group, index, case, broken, copies, counted, failing)` | Runs the copies of a table in its order, and then the table in the reverse order. It gives the verdict of each row, and the output of the run that failed. |
| `verdict(config, values, outputs, group, index, case, broken, copies, counted, failing)` | Runs the copies of one case at the same time, and gives their verdict. |
| `uses_of(config, values, running)` | Gives the uses of each group of `running`, as *The uses* tells. It stops for each fault of *The uses*. |
| `test(config, values, outputs, groups, broken, said, copies, counted, failing, workers)` | Runs the cases of all the groups with `workers` threads, as *The workers* tells, and writes each verdict. With `said`, it writes `said.txt`. It gives the names of the cases that failed. |
| `slow_of(config)` | Gives `[test] slow`, or 10. A value below 0, or one that is no number, stops the run: `[test] slow is a count of seconds, 0 or more`. |
| `timed(slow, took, seconds)` | Writes the slowest cases, the line of each slow case, and the time of the run, as *The times* tells. |
| `lanes_of(jobs, uses)` | Gives the lanes of the cases. The cases of the groups with the same uses are 1 lane. Each case of a group with no use is 1 lane. |
| `one(config, values, outputs, group, index, case, broken, copies, counted, failing)` | Runs one case or table, and gives the name, the verdict and the outputs of each case or row. |
| `told(named, ends, broken)` | Writes the verdict of each case or row, and the output of a run that failed. |
| `workers_of(args, config)` | Gives `--workers`, or `[test] workers`, or the count of the processors. It stops if the count is not 1 or more: `[test] workers and --workers are a count of 1 or more`. |
| `narrowed(case, text)` | Gives a case whose name contains `text`, or a table with only its rows whose name contains it, or nothing. |
| `chosen(groups, text)` | Gives only the cases and the rows whose name contains `text`. It stops if no case has it: `no case has <text> in its name`. |
| `owner(config, group)` | Gives the part of a group from `part_of` of `shared.py`, with a space before and after it, or `False`. |
| `picked(config, groups, args)` | Gives only the groups of the parts of `--part` and the groups of `--group`, as *Groups* tells, then only the cases of `-k`, with `chosen`. |
| `rowed(case, path)` | Checks the rows of a table, and stops at a fault: *A table*. |
| `ruled(path, cases)` | Stops for a group that breaks a rule of `[test] tables`: `docs/contributing/tests.md`, *Its own process*. |
| `groups_of(config, values, folder)` | Reads and checks the groups of a folder: `tests/cases`, or `.toolchain/tests/cases` of the repository `mc23t`. |
| `changed(args, run, write, prepared)` | Does the steps of a changed run, as *A changed run* tells. It gives the config, the variables, what `write` gave, the artifacts and the groups. |
| `scoped(args, entries, key, names)` | Gives the exemptions whose `key` matches one of the names, with `*`. With no `--part` and no `--group`, it gives all the exemptions. |
| `measured(args)` | Runs the command `test --branches`, as *The branches* tells. |
| `tallied(args)` | Runs the command `test --leaks`, as *The leaks* tells. |
| `main()` | Runs the command `test`. |

## main

With `--leaks`, `main` runs `tallied`, and nothing else. With `--branches`, it runs `measured`, and nothing else.
Else `main` does these steps:

1. It builds with `build`, which stops if the target does not run on this machine, and writes `the build took <seconds> s`. With `--toolchain`, it only calls `prepared` of `build.py`, and builds nothing.
   With `--toolchain`, the configuration has each target of `.toolchain/toolchain.toml`, so a test of the toolchain runs on each target.
2. It stops if an artifact is not linked: `the cases need what is linked: <names>`.
   With `--build`, it then writes `built <count> artifacts`, and ends. Thus a project can prove that each artifact builds, the test ones too, with no case.
   `--build` stops with `--toolchain`, `--branches`, `--leaks` or `--break`: `--build builds each artifact and runs no case: it takes no --toolchain, --branches, --leaks or --break`.
3. It reads the groups of `[test] cases` (the default is `tests/cases`). With `--toolchain`, it reads `.toolchain/tests/cases` of the repository `mc23t`.
4. If there is no case, it writes `no case: <folder> holds no case`, and ends.
5. With `--part`, it keeps only the groups of the named parts. With `--group`, it keeps only the named groups. With `-k`, it keeps only the cases whose name contains the text.
6. If a case states `fail` or `sweep`, it makes the run `fail`: *A changed run*, with `failed` of `fail.py`.
7. It runs the groups, one time for each round of `--rounds`. Before each round of 2 rounds or more, it writes `round <n> of <rounds>`.
8. With no `--break`, `--toolchain`, `-k`, `--part` or `--group`, it checks the messages of the code with `checked` of `said.py`.

With `--break`, the false variant of a case tells if the case states `fail` or `sweep` at step 6.
The number of copies is `--copies`, or `[test] copies`. If neither gives it, `main` stops: `.toolchain/project.toml: write [test] copies`.
The number of workers is `--workers`, or `[test] workers`, or the count of the processors: *The workers*.

`main` stops at step 7 if a case failed. Thus step 8 runs only if each case passed.
`.toolchain/docs/said.py.md` gives the checks of step 8.

`.toolchain/docs/workflows.py.md`, *The workflows*, tells which workflow runs the cases of the toolchain on each target.

The message of step 1 is `<target> does not run here: build it, and test it where it runs`.

## Groups

`groups_of` reads each file `**.toml` of its folder, in the order of the paths.
The name of a group is the path of its file in `tests/cases`, without `.toml`.

A group is in a part if its name is the folder of the part, or begins with the folder and a slash.
The folder of a part is its key `folder`: `.toolchain/docs/project.toml.md`, *[parts]*.
`picked` stops if no group is in the named parts: `no case group is in the parts <parts>`.

`--group <group>` names one group, and the option can be given more than one time. `picked` stops if a named group is not there: `no case group <group>`.
A group can state the code that it tests: `.toolchain/docs/scope.py.md`, *The code of a group*.

A file with no case gives no group.
`groups_of` stops if a case:

- has a key that a case does not have, or its false variant or its helper has such a key,
- holds a step or a value of `ignore` that `allowed` of `signals.py` refuses, also in its false variant: `.toolchain/docs/signals.py.md`, *The checks*,
- claims nothing: `<path>: the case <name> claims nothing`,
- has no false variant: `<path>: the case <name> has no false variant`,
- states a `process` that is no reason: `<path>: the case <name> states the process <word>, and a process is status, signal, helper, command, state, script`,
- states the process `script` with no key `script` and no key `stdin`: `<path>: the case <name> states the process script, and it has no script and no stdin`,
- states a call in `fail`, also in its false variant, that `listed` of `fail.py` does not give: `<path>: the case <name> makes <call> fail, and [fail.calls] does not list it`,
- states such a call whose value is the empty text: `<path>: the case <name> makes <call> fail, and <call> gives no result, so no case can make it fail`,
- states a `hold` that `hold_checked` refuses, also in its false variant: *The hold*,
- states `systems` that is not a list of 1 or more systems of the targets: `<path>: the case <name> states systems <list>, and each must be the system of a target: <systems>`.

With `tables = true` in `[test]`, `groups_of` then gives the cases of each file to `ruled`.

Then `groups_of` keeps only the cases with no `systems`, or whose `systems` holds the `system` of the target. If it leaves a case out, it writes `<group>: <n> cases for another system: <names>`.
So a case that it leaves out runs in no run of `test`, also with `--branches` or `--leaks`.

## A case

`tried` runs one case:

1. It empties the folder of the case, `<out>/cases/<group>/<number>`, or `<out>/cases/<group>/<number>/<copy>` for a copy, with `cleared`.
2. It adds the variable `{case}`. If a text of the case holds `{port}`, it adds `{port}` too: a new free port.
3. With `--break`, it uses the false variant of the case.
4. It writes the files, and fills the folders of the case.
5. If the case has a helper, it starts the helper, and waits for its text `wait`.
6. It starts the command, and does the steps of the script.
7. It writes `stdin` to the command, and waits for the end of the command.
8. If the case has a helper, it writes `stdin` of the helper, and waits for the end of the helper.

A case that holds no `{port}` gets no port. In such a case, a value of `[variables]` that holds `{port}` stops `test`: `no variable {port} for <text>`.

With `counted`, the command and the helper get the variable `C23_LEAKS`: the path of the folder of the case, with `.leaks` added.
The environment of the command and of its helper is the one of `test`, with the changes of `[test] env`, then the changes of the case. So the `env` of a case replaces a variable of `[test] env`.
If the case states `fail` or `hold`, the command gets the variable `C23_FAIL`: `.toolchain/docs/fail.py.md`, *The layer*. Else `tried` removes `C23_FAIL` from the environment of the command.

A case that states `fail`, `sweep` or `hold` also gives the command `C23_FAIL_PART`, the part that `owner` gives for the group of the case. With no such part, `tried` removes `C23_FAIL_PART` too.
A case that states `fail_parts` gives the parts that it names in place of its own: `.toolchain/docs/fail.py.md`, *The layer*.
`groups_of` stops if `fail_parts` is empty or names no part of the project: `<file>: the case <name> states fail_parts <names>, and each must name a part of the project`.
With `failing`, a case that states `fail`, `sweep` or `hold` runs on the build of the run `fail`. Its folder, its `{out}` and its artifacts are those of that run.

A case that states `hold` makes 1 call of 1 file wait until another call has run a number of times: `.toolchain/docs/fail.py.md`, *The hold*.
`held` gives its part of `C23_FAIL`.

`tried` runs each command in the root, with a timer of `[test] timeout` seconds. For a table, the timer starts again at each answer: the time is for one row.
The timer stops a command that does not end in time.
`placed` and the steps `put` copy each file with its permissions. Thus a copy of a program can run on Linux and on macOS.

`placed` links each file of the build folder, for example an artifact or a fetched program, in place of a copy. A link is the same file, so it keeps the permissions too.
On Windows, a new copy of a program costs a scan of the antivirus at its first load. A link costs no copy and no new scan.
If the system refuses a link, for example across 2 disks, `placed` copies the file.

A link shares its bytes with the build output. A case that opens a placed file for a write changes the output for each later case. A replace by a rename is safe.
So `tried` takes the stamp of each linked file again when the case ends. If a stamp changed, the case fails: `wrote into the build output <paths>`.

The step `put` of a script copies its file to `<case>.put`, beside the folder of the case, and then renames the copy onto its target.
So a program that runs always finds the old file or the whole new file, and never a missing or a partial file.
A rename does not write into the old file, so a put never writes into a linked file.
On Windows, `renamed` uses the POSIX semantics, so a put also replaces a file that the program holds open, if its handle shares the delete.

`made_false` makes the false variant: the keys of `false` replace the keys of the case.
For `files` and for `folders`, each path of `false` replaces only that path.

## The hold

The key `hold` of a case is a table with exactly 5 keys:

| Key | Value |
| --- | --- |
| `file` | The C file of the project, from its root, whose call waits. |
| `call` | The function whose call waits. `[fail.calls]` lists it. |
| `at` | The number of the call in `file` that waits, 1 or more. |
| `until` | The function whose calls release the wait, in any file of the layer. `[fail.calls]` lists it. |
| `times` | The count of the calls of `until` that releases the wait, 1 or more. |

`file`, `call` and `until` can hold variables, for example a value of the target: `docs/contributing/tests.md`, *Variables*. So 1 case can hold a different file or call on each system. `hold_checked` checks the texts after it replaces the variables.

`hold_checked` stops `test` with the file and the name of the case if:

- the hold has another set of keys: `the case <name> states hold, and a hold has exactly the keys file, call, at, until and times`,
- `call` or `until` is not listed: `the case <name> holds at <key> <function>, and [fail.calls] does not list it`,
- `file` is not a C file of the project, or holds a space: `the case <name> holds in <file>, and the file of a hold is a C file of the project`,
- `at` or `times` is not a count of 1 or more: `the case <name> states the hold <key> <value>, and it is a count of 1 or more`.

## A table

A case with `rows` is a table. `docs/contributing/tests.md`, *Tables*, gives its rules.

`rowed` stops for a table that it cannot run:

| Fault | Message |
| --- | --- |
| The table has no `claims`. | `<file>: the case <name> claims nothing` |
| The table has a key `false`, `stdin` or `script`. | `<file>: the table <name> has rows, and a table has no key <key>` |
| `rows` is empty. | `<file>: the table <name> has no row` |
| A row has a key that no row has. | `<file>: a row has no key <key>` |
| A row has no `name`, `in`, `claims` or `false`. | `<file>: a row of the table <name> has no key <key>` |
| A row claims a thing of the whole run, such as `status`. | `<file>: a claim of a row has no key <key>` |
| A row has no claim. | `<file>: the row <name> claims nothing` |

`tabled` runs the command with `raced` and `tried`. `tried` writes the inputs with `typed_rows`.
`answered` cuts the output at each empty line. An answer is the text before an empty line; a text after the last empty line is no answer.

`claimed` judges the claims of a row on its answer with `holds`. The answer has no line end after its last line.
For the claim `output`, `claimed` adds that line end, and compares the answer with the text of the file. Thus the file of a row is the lines of its answer, each with its line end.
`answered` gives each row its verdict:

| The run | The verdict of a row | The row |
| --- | --- | --- |
| A row does not answer in time, or the command does not end in time after its last answer. | `did not end in <timeout> seconds` | fails |
| The command gave more answers than the table has rows. | `is one of <rows> rows, and the command gave <answers> answers` | fails |
| A claim of the case is false. | `is in a run that differs` | fails |
| The command gave no answer to the row, and it is the first such row. | `has no answer: the command ended at this row` | fails |
| The command gave no answer to the row. | `has no answer` | fails |
| The command answered the row. | The verdict of its claims on its answer, as *Verdicts* tells. | as for a case |

`raced` runs the copies of a table at the same time, in threads, as `verdict` does for a case.
The number of copies is `[test] copies` or `--copies`, or one copy if the table has `alone = true`.
The folder of a copy is `<number>/<copy>`. A row passes only if it passes in each copy.
If a row fails in copies, `joined` gives it the verdict of the first copy in which it failed.
That verdict ends with `in <failed> of <copies> copies`.

`tabled` runs a table of 2 rows or more a second time, with the rows in the reverse order. That run has one copy.
It does so only if each row passed, in a run with no `--break` that counts no leaks.
The second run has the folder `<number>/reverse`.
A row that fails then gets ` in the reverse order` after its verdict, and the table shows the output of that run.

`test` counts each row as a case. `narrowed` keeps the rows of `-k`.

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

In a run with no `--break`, `--toolchain`, `-k`, `--part` or `--group`, `test` then writes `<out>/said.txt`.
The file holds the output of each command and of each helper, in the order of the cases.
`said.py` reads it. A run with `--break` writes no `said.txt`, because a false variant does not run as the project runs.
If a case failed, `test` stops with the names of the cases that failed, after it writes `said.txt`.

## Claims

`holds` tells if each claim of a case is true. It replaces the variables in each text of the claims.
The output of a command is what it writes to its output and to its errors, without the carriage returns.
A claim `line` compares full lines. A claim `order` ignores the spaces at the start and at the end of a line.

## A changed run

`test --branches`, `test --leaks` and the run `fail` of `test` run cases on a changed copy of the code. `changed` does their shared steps:

1. It does the steps 1 to 10 of a build with `ready` of `build.py`, or it takes `prepared`.
2. It sets the output folder of the target to `<build>/<run>/<target>`.
3. It writes the changed copies of the sources, with the function `write` of the run. With `--part`, `write` gets only the named parts.
4. It builds the artifacts from these copies, with `made` of `build.py`.
5. It stops if an artifact is not linked, as step 2 of *main* tells.
6. It reads the groups of `tests/cases`, and keeps the groups and the cases that `picked` gives.

At step 3, `kept` of `build.py` gives `write` the artifacts of the named parts only. Thus the probes, the counts and the layers are only in their sources.
The scope of `fail` does not come from this step: each layer refuses calls only for the cases of its parts.
The parts of their `uses` are built from their own sources, at step 4.

`ready` stops at step 1 if the target does not run on this machine.
In `test --branches` and in `test --leaks`, each case runs one time. A case that fails does not stop the run, and its verdict is only for information.
The run `fail` gives `prepared`: the configuration and the variables of step 1 of *main*. Thus `test` prepares the build one time, and `main` judges the cases.

## The branches

`measured` does these steps:

1. It stops with one of these options: `--break`, `--toolchain`, `--copies` or `--rounds`.
2. It does the steps of *A changed run*, with the run `branches` and `probed` of `branches.py`.
3. It writes `<build>/branches/<target>/hits.bin` with a byte 0 for each outcome. The variable `C23_BRANCHES` names it.
4. It runs each case one time, and writes its verdict.
5. It writes the report, with `reported` of `branches.py`.

With `--part`, step 2 gives the named parts to `probed`, so only their files get probes: `.toolchain/docs/branches.py.md`, *The files of the named parts*.

With `--group`, step 2 gives `claimed` of `scope.py` to `probed`: `.toolchain/docs/branches.py.md`, *The files of the named groups*.
Step 5 then first writes `branches: measured the code of the groups <groups>`.

Step 5 then reads only the exemptions whose `path` matches a file with probes, with `scoped`. Thus an exemption of a different part is no fault.
The message of step 1 is `--branches runs each case one time: it takes no --break, --toolchain, --copies or --rounds`.
`.toolchain/docs/branches.py.md` tells what the probes measure, and what the report contains.

## The leaks

`tallied` does these steps:

1. It stops with one of these options: `--break`, `--toolchain`, `--copies`, `--rounds` or `--branches`.
2. It does the steps of *A changed run*, with the run `leaks` and `counted` of `leaks.py`.
3. It empties the folder `<build>/leaks/<target>/cases`.
4. It runs each case one time with `counted`, and writes its verdict. Each case gets its own file of lines.
5. It writes the report, with `reported` of `leaks.py`.

With `--part`, step 5 reads only the exemptions whose `case` matches a case of the run, with `scoped`. Thus an exemption of a different part is no fault.
The message of step 1 is `--leaks runs each case one time: it takes no --break, --toolchain, --copies, --rounds or --branches`.
`.toolchain/docs/leaks.py.md` tells what the run counts, and what the report contains.

## The workers

`test` runs the cases of all the groups in a pool of threads. The number of threads is the number of workers.

The cases of the groups with the same uses are 1 lane, which runs its cases one after the other. Each case of a group with no use is a lane of its own.
A worker runs 1 lane at a time. Thus the cases of the groups with no use run at the same time.
The cases of 1 lane hold only 1 worker, also when they wait for a use.

Each case takes the locks of the uses of its group: *The uses*. Thus 2 lanes that share a use take turns, for example `gpu` and `gpu display`.

`test` writes the verdicts in the order of the groups, and of the cases in each group. It writes the line of a group after its last case.
Thus the output of a run with many workers has the same lines in the same order as a run with 1 worker.
With `--workers 1`, the cases run one after the other.

Each case also waits for room, in a `Room` of `shared.py`.
It starts only when `free_memory` gives at least `[test] memory_floor`, or when no other case of the run runs. Thus the workers never take the last memory of the machine, also when another run holds it. A case still starts alone, so a run always ends.
With no measure of the memory, on a system that none of these 3 reads covers, a case does not wait.

A case can share a thing of the machine that no use names, for example the clipboard. It can then fail when another case runs at the same time. Such a case gets a use.

## The times

`test` measures the time of each case, from its start to its verdict, with the wait for a use and for room left out. A table is 1 case.
After the line of each group, it writes `<group>: its cases took <seconds> s`, the sum of the times of its cases. With many workers, this sum can be more than the time that the group took.
After the last group, it writes:

1. `the <count> slowest cases:`, then 1 line `  <seconds> s <group>: <name>` for each of the 10 slowest cases, the slowest first. A run with less than 10 cases lists each case.
2. `a slow case: <group>: <name> took <seconds> s, above [test] slow of <slow> s` for each case that took more than `[test] slow`. The default is 10 seconds.
3. `the run of the cases took <seconds> s`, from the start of the first case to the verdict of the last.

Thus each run of CI shows the cases that make it slow, and a new slow case shows at once.

## The uses

A use is a resource of the machine that 2 runs must not share at the same time, for example the GPU or the display.
`[test] uses` of `.toolchain/project.toml` maps the name of each use to patterns of paths of the code.
A group has a use when it tests a file that a pattern of the use matches.
A group of the whole with no `code` has each use.
The rule of a tested file is the rule of `tests` of `scope.py`: `.toolchain/docs/scope.py.md`, *The code of a group*.

Before each case, `test` takes the lock of each use of its group, in the order of the names. It gives them back after the case. `locks.py` holds the locks: `.toolchain/docs/locks.py.md`.
Thus the runs of 2 clones on 1 machine go on at the same time. Only the cases of the groups that share a use take turns.
While a group waits, `test` writes `<group>: waits for the use <name>, which another run holds`.

`test` stops for each of these faults of `[test] uses`:

- `[test] uses: write a table of the uses`: `uses` is not a table.
- `[test] uses: <name> is not a name of letters, digits, _ and -`: the name of a use is also the name of its file.
- `[test] uses: <name> is a list of patterns`: the value of a use is not a list of texts.
- `[test] uses: <name> has no pattern`: the list is empty.
- `[test] uses: <pattern> matches no file`: no file of the code, of `.toolchain/config`, `.toolchain/tools` or `.toolchain/runtime`, and no file of the project, such as `.toolchain/project.toml`, matches the pattern.
