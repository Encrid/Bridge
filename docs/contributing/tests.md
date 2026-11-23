# Tests

`python project.py test` builds the project, then runs the cases of `tests/cases`.
A case runs a command, and makes claims about its exit status, its output and its files.
`python project.py test --break` runs the false variant of each case. Each case must then fail.
`tests` can also hold other files, for example the data that the cases read. No check reads `tests`.

## Test a part

A test is for a part, and it can reach each outcome of the part. Do not write a test for each outcome.

1. A test program tests one part. It reads a table, and gives one answer for each row: *Tables*.
2. The rows come from what the code must do. Write one row for each class of inputs that the code treats in one way. Write one row for each edge between two classes. Do not take the rows from the lines of the code.
3. `test --branches` measures the table: *The branches*.
4. A case has its own process only if it needs one: *Its own process*.
5. A sweep tests the calls that fail for one reason, such as no memory: *A sweep*.

A row that runs a branch does not check it: only a claim checks it.
Thus each row claims what the code must give, also at an edge. For example, a limit of 10 gets a row with 10 and a row with 11.

## Write a case

1. Select a behavior that a document of the project describes.
2. Write the command that shows the behavior.
3. Write the claims: what the command must do.
4. Write the false variant: a change of the command or of its input that makes the claims false.
5. Run `python project.py test`. The case must pass.
6. Run `python project.py test --break`. The case must fail.

For example, the program `greet` writes `hello, <name>` for the name that it gets:

```toml
[[case]]
name = "greet writes the name that it gets"
run = ["{out}/greet{exe}", "Ada"]
claims = { status = 0, line = ["hello, Ada"] }
false = { run = ["{out}/greet{exe}", "Bob"] }
```

With `Bob`, the output has no line `hello, Ada`. Thus the claims of the false variant are false, and the case can fail.

- A case or a row checks one behavior. Its name tells the behavior.
- Each behavior that a document describes has a case or a row.
- The false variant changes the command or its input. It cannot change the claims.
- The false variant runs the code that does the behavior, with one input changed. A command that the program refuses before that code runs is not a false variant, for example an unknown name.
- A case writes its files only into its folder, `{case}`.

## Tables

A case can hold a table: the key `rows`. Each row is one behavior. `test` starts the command 1 time for the whole table.

For example, the program `greet` reads one name on each line, and answers each one:

```toml
[[case]]
name = "greet"
run = ["{out}/greet{exe}"]
claims = { status = 0 }
rows = [
    { name = "greet writes the name that it gets", in = "Ada", claims = { line = ["hello, Ada"] }, false = "Bob" },
    { name = "greet refuses an empty name", in = "", claims = { line = ["greet: no name"] }, false = "Ada" },
]
```

- A row has a name, its input `in`, its claims, and its false input `false`. A row needs each one.
- `test` writes the input of each row on one line of the standard input of the command, in the order of the table. Thus an input is one line.
- `test` reads the answers while it writes the inputs. Thus a table can be longer than a pipe holds.
- The program answers each line with one line or more, and then writes one empty line. Thus an answer holds no empty line. The program ends when its input ends.
- `test` judges the claims of a row on the answer to that row. A row claims `line`, `has`, `lacks`, `matches`, `matches_not`, `order` or `output`.
- For a row, `output` claims that the lines of the answer are the same as the text of the file.
- The claims of the case are for the whole run. A table needs them, as each case does.
- `test --break` starts the command 1 time with the false input of each row. Each row must then fail.
- `test` then runs the table in the reverse order. A row must answer the same in each order. A row that needs the state that a different row left fails: `differs in the reverse order`.
- If the program ends before it answers each row, the first row with no answer fails, and each row after it. `test` names that first row.
- The time of `[test] timeout` is for one row: it starts again when the program answers a row. If a row does not answer in time, each row of the table fails.
- `test` runs the copies of a table at the same time, as it does for a case: *Run the tests*. Each copy has a folder of its own, and a row must pass in each copy. The run in the reverse order has one copy.

`test` writes one line for each row, as for a case: `<row>: <verdict>`. It counts each row as a case.
A test program that reads its arguments becomes a program of a table with `rows.h`: `.toolchain/docs/rows.h.md`.
`-k` chooses the rows whose name contains its text. `test` then gives only their inputs to the command.

## Its own process

A case with no `rows` starts a process for one behavior. It states why, with the key `process`:

| `process` | The reason |
| --- | --- |
| `status` | The behavior is how the command ends: its status, an abort, or a kill. |
| `signal` | The case sends a signal to the command. |
| `helper` | The case runs a helper beside the command. |
| `command` | The behavior is in what the command gets when it starts: its arguments, its environment or its files. |
| `state` | The behavior needs a process that did nothing before. |
| `script` | The case talks to the command while it runs: it has the key `script` or the key `stdin`. A table has none of the 2 keys. |

`test` stops for a different word: `<file>: the case <name> states the process <word>, and a process is status, signal, helper, command, state, script`.
`test` also stops for `script` on a case with no `script` and no `stdin`: `<file>: the case <name> states the process script, and it has no script and no stdin`.

With `tables = true` in `[test]`, `test` also stops for a group that breaks one of these rules:

| Fault | Message |
| --- | --- |
| A case with no `rows` states no `process`. | `<file>: the case <name> has its own process and states no reason: write process, or make it a row of a table` |
| A table states `process`. | `<file>: the table <name> states process, and only a case with no rows has its own process` |
| 3 cases or more have one command, and each one makes one call of the same function fail. | `<file>: <count> cases refuse the calls of <function> one after the other, from the case <name>: write a sweep` |

A project states `tables = true` when its cases follow *Test a part*.

## Groups

A group is a file of cases in `tests/cases`, or in a folder of `tests/cases`.
The name of a group is the path of the file in `tests/cases`, without `.toml`.
For example, the name of the group `tests/cases/<folder>/<name>.toml` is `<folder>/<name>`.
If `tests/cases` holds no case, `test` writes `no case: tests/cases holds no case`.
In a project with parts, the groups of a part are in `tests/cases/<folder>`, with the folder of the part: `.toolchain/docs/project.toml.md`, *[parts]*.

A group states the code that it tests with the key `code`, a list of patterns at the top of its file. A group with no `code` tests the code of its part, or of its project if the project has no part.
A group that tests the project as a whole states `whole = true`, and runs when `dev` goes into `main`. In a project with no such group, each group runs then.
A change runs only the groups that test a changed file: `.toolchain/docs/scope.py.md`.

## The keys of a case

| Key | Value |
| --- | --- |
| `name` | The name of the case. |
| `run` | The command, as a list. |
| `env` | Changes to the environment of the command. The key is the name of a variable. The value is a text, or `false` to remove the variable. |
| `stdin` | The text that `test` writes to the command, after the script of the case. |
| `script` | The steps that `test` does while the command runs. |
| `ignore` | `["interrupt"]` if the command starts with the interrupt ignored: *Scripts*. |
| `helper` | A second command that runs at the same time. |
| `files` | The text files that `test` writes before the command starts. The key is the path, and the value is the text. |
| `folders` | The folders that `test` makes before the command starts. The key is the path. The value is a list of artifacts or files to place into the folder. A file of the build folder is linked, and each other file is copied. A file of the list with `*` is a pattern of the project. `test` places each file that the pattern matches, and stops if it matches none. |
| `sweep` | The functions whose calls the command refuses one after the other: *A sweep*. |
| `fail` | The calls that fail in the command. The key is a function, and the value is the number of its call that fails: *Failed calls*. |
| `fail_parts` | The parts whose calls `fail` refuses, in place of the part of the case. Use it when a program links a library of another part. |
| `hold` | A call that waits for the calls of another thread: *A hold*. |
| `claims` | The claims about the command. A case must make one claim or more. |
| `false` | The false variant of the case. Each case must have one. |
| `rows` | The rows of a table: *Tables*. A case with `rows` has no `false`, no `stdin` and no `script`. |
| `process` | The reason why a case with no `rows` has its own process: *Its own process*. |
| `systems` | The systems on which the case runs, for example `["windows"]`. Each is the `system` of a target. On another system, the case does not run, and `test` writes `<group>: <n> cases for another system: <names>`. A false variant cannot state it. |
| `alone` | `true` if the case must not run at the same time as a copy of itself. Such a case runs one time with `--copies`. |

Each case starts in an empty folder, `{case}`. The command runs in the folder of the project.
`test` replaces the variables in all the texts of a case, also in the texts of `files`.
`test` copies the files of `folders` without a change, and keeps their permissions. Thus a copy of a program can run.

## Claims

| Key | The claim is true if: |
| --- | --- |
| `status` | The command ends with this exit status. |
| `status_not` | The command does not end with this exit status. |
| `line` | Each text of the list is a full line of the output. |
| `has` | The output contains each text of the list. |
| `lacks` | The output contains no text of the list. |
| `matches` | Each regular expression of the list is found in the output. A `.` also matches a line end. |
| `matches_not` | No regular expression of the list is found in the output. |
| `order` | The output has each text of the list as a line, in this order. The spaces at the start and at the end of a line do not count. |
| `output` | The output is the same as the text of this file of the project. |
| `files` | After the command ends, each file contains its text. The key is the path, and the value is the text. |

The output is the text that the command writes to its output and to its errors.

## Variables

A text of a case can contain variables, for example `{out}`. `test` replaces each variable with its value.

| Variable | Value |
| --- | --- |
| `{case}` | The folder of the case. |
| `{port}` | A free port of this machine, for a case that holds `{port}` in a text. Each such case gets a new port. |
| `{root}` | The folder of the project. |
| `{out}` | The output folder of the target. It holds the artifacts. |
| `{target}` | The name of the target. |
| `{exe}` | The extension of a program on the target: `.exe` on Windows, else nothing. |
| `{library}` | The extension of a library on the target: `.dll`, `.so` or `.dylib`. |
| `{python}` | The Python program that runs the tools. |

Each key of `[variables]` of `.toolchain/project.toml` is a variable too.
So is each key of the table of the target, for example `{system}`, which is `windows` or `posix`.

## Scripts

Each step of a `script` does these actions, in this order:

1. `wait` waits for a line of the output that contains its text. A step with no `wait` does its actions at once.
2. `put` copies files. The key is the path, and the value is an artifact or a file.
3. `remove` removes the files of the list.
4. `type` writes this line to the command.
5. `close` closes the output of the command. Its only value is `"output"`. After it, a write of the command to its output fails.
6. `signal` sends a signal to the command: `interrupt`, `termination` or `hang-up`.
7. `pause` waits this number of seconds.

If the output ends before the text of `wait` comes, `test` stops the steps.

On Linux and on macOS, the signals are `SIGINT`, `SIGTERM` and `SIGHUP`.
On Windows, a command that gets a signal runs on a console of its own. The signals are ctrl-c, ctrl-break and the close of the console.
Thus `test` refuses 2 scripts on each system:

- A script with an interrupt and a termination. On Windows, ctrl-break needs a process group of its own, and such a group ignores ctrl-c.
- A script with a signal after a hang-up. On Windows, the hang-up closes the console.

With `ignore`, the command starts with the interrupt ignored. `.toolchain/docs/signals.py.md` tells how `test` starts the command and sends the signals.

## Helpers

A `helper` has a command `run`, a text `wait`, a text `stdin` and its own `claims`.
`test` starts the helper first, and waits for a line of its output that contains the text `wait`.
Then `test` starts the command of the case.
When the command of the case ends, `test` writes `stdin` to the helper, and waits for the end of the helper.
The claims of the helper must also be true.

An empty `helper` table means that the case has no helper. Thus a false variant can remove the helper.

## The break run

`python project.py test --break` runs the false variant of each case.
The keys of `false` replace the keys of the case.
Each path of `false.files` and of `false.folders` replaces only that path.
The claims of the case must then be false. A case that passes makes the break run fail.
Thus the break run shows that each case can fail.

`test` refuses a case that has no false variant, also without `--break`.

## The branches

Each branch of the code must run in a case.
`python project.py test --branches` builds the project with probes in each function of `src` and of `include`.
It runs each case one time. Then it writes a line for each branch that no case runs:

- a condition that no case makes true, or that no case makes false. A test with `&&` or `||` counts by its parts;
- a `case` or a `default` that no case reaches;
- a function that no case calls.

For each line, find the class of inputs or the edge that the table of the part lacks, and add its row.
If no input can run the branch, remove the branch. Do not write a case only to run a line.
A condition in an argument or in the body of a macro cannot get a probe. The run writes it, and stops.
Thus write such a condition outside the macro.

The probes change the program, thus the run does not judge the claims. It writes the verdicts only for information.
An outcome counts as soon as a case reaches it, also if the program then ends with `_Exit` or `abort`.
In a wasm module, an outcome counts when the host of the module destroys the instance: `.toolchain/docs/branches.py.md`, *A wasm module*.
`.toolchain/docs/branches.py.md` tells how the probes work.

## The leaks

Each case must give back each resource that the code takes, also on the paths of the errors.
`python project.py test --leaks` builds the project with a count of the resources in each file `.c` of `src`.
The count is also in each test source that a program, a library or an archive compiles.
So a test frees each block that it takes, and each block that the code gives to it.
It runs each case one time. Then it writes a line for each case that does not give back what the code took:

- `leaks: <case> keeps <number> <kind>`: at its end, the case holds a resource that the code took.
- `leaks: <case> gives back <number> <kind> that it did not take`: the code gives back a resource that code with no count took.
- `leaks: <case> calls realloc with 0 bytes`.

The kind `memory` counts `malloc`, `calloc`, `realloc` and `free`.
`leaks.toml` of `.toolchain/config` names more functions, pairs and kinds, for example the pair `os_reserve` and `os_release`.
`.toolchain/docs/project.toml.md`, *[leaks]*, gives its keys.

For each line, make the code give the resource back. If a case keeps a resource by design, write an exemption with its reason.
`.toolchain/docs/project.toml.md`, *[[leaks.exception]]*, tells how.
The counts change the program, thus the run does not judge the claims. It writes the verdicts only for information.
`.toolchain/docs/leaks.py.md` tells how the count works.

## Failed calls

A case can make a call of the code fail. Thus the case runs the path of the error, for example the branch of `if (!p)`.
The key `fail` names each function, and the number of its call that fails. The first call has the number 1.
In this case, the second call of `malloc` gives `nullptr`:

```toml
[[case]]
name = "greet writes an error when it has no memory"
run = ["{out}/greet{exe}", "Ada"]
fail = { malloc = 2 }
claims = { status = 1, line = ["greet: no memory"] }
false = { fail = {} }
```

`malloc`, `calloc` and `realloc` can always fail, and so can the two loaders `GetProcAddress` and `dlsym`, with `nullptr`.
`fail.toml` of `.toolchain/config` names more functions, each with the value that a failed call gives.
`.toolchain/docs/project.toml.md`, *[fail]*, gives its keys. Only the calls of the code of `src` count, and the files of one program share one count.

A case refuses the calls only in the programs of its part, in each run. A case in no part refuses them in each program.
The false variant states no `fail`, or a different number.

`test` runs each case that states `fail` on a copy of the code in which these calls can fail. It judges the claims of the case.
`test --branches` and `test --leaks` make the calls fail too. Thus a case reaches each `if (!p)`, and gives back what the code took on that path.

A call through a pointer of a loader can fail too.
A listed function that the file only names in a text fails by its own name with its own value.
For example, `"socket"` is a text of a call of `GetProcAddress`, and `fail = { socket = 1 }` fails that call.
`.toolchain/docs/fail.py.md` tells how a call fails.

## A sweep

A test program can refuse each call of the code in 1 run: the first call, then the second, until the code runs whole.
The case states `sweep`, with each function that the program refuses. Each one is a function that a case can make fail.

```toml
[[case]]
name = "each refused malloc of greet gives an error"
run = ["{out}/greet-unit{exe}"]
sweep = ["malloc"]
claims = { status = 0, line = ["2 refused calls, then greeted"] }
false = { run = ["{out}/greet-unit{exe}", "short"] }
```

The program includes `sweep.h` of `.toolchain/runtime`. Its artifact has `{toolchain}/runtime` in `include`.

```c
unsigned at = 0;

do {
    at++;
    sweep_refuse("malloc", at);
    greeted = greet("Ada");
} while (sweep_is_refused());
```

- `sweep_refuse` makes the call number `at` of the function fail, and starts each count again.
- `sweep_is_refused` tells if a call failed since then. When no call failed, the code made fewer calls than `at`, and the sweep is done.
- In each turn, the program checks what the code must do when the call fails, and writes a line that the case claims.
- `test` runs a case with `sweep` on the copy of the code in which calls can fail, as a case with `fail`. It refuses the calls only in the programs of the part of the case.
- `test --leaks` counts the whole run. Thus the code gives back what it took in each turn.

Use a sweep for the calls that fail for one reason, such as no memory. Use `fail` for one call whose failure is a behavior of its own.
`.toolchain/docs/sweep.h.md` tells how a sweep works.

## A hold

A case can give 2 threads the same order in each run. Then a race between them fails in each run, and not by chance.
The key `hold` makes call `at` of `call` in `file` wait until `until` has run `times` times, in any file:

```toml
[[case]]
name = "the worker writes after main"
run = ["{out}/held{exe}"]
hold = { file = "src/worker.c", call = "wait_turn", at = 1, until = "done", times = 1 }
claims = { status = 0, matches = ["main\\s+worker"] }
false = { hold = { file = "src/held.c", call = "wait_turn", at = 1, until = "done", times = 1 } }
```

`file`, `call` and `until` can hold variables, for example `src/{system}/read.c`, so 1 case holds the file of each system.
`[fail.calls]` lists `call` and `until`. A function with no result has the empty text as its value, so a case can hold it, and no case can make it fail.
Put the hold before the lock of the thread that waits. Else the other thread can wait for the lock, and the case ends only at its timeout.
`.toolchain/docs/fail.py.md`, *The hold*, tells how a call waits.

## Run the tests

| Command | Result |
| --- | --- |
| `python project.py test` | Runs each case. |
| `python project.py test --break` | Runs the false variant of each case. Each case must fail. |
| `python project.py test --branches` | Runs each case one time on a build with probes. Each branch must run. |
| `python project.py test --leaks` | Runs each case one time on a build with counts. Each case must give back what the code takes. |
| `python project.py test -k <text>` | Runs only the cases whose name contains `<text>`. |
| `python project.py test --group <group>` | Runs only the named group. You can use this option more than one time. |
| `python project.py test --part <part>` | Builds the part and the parts that it uses, then runs only the groups in `tests/cases/<folder>`, with the folder of the part. |
| `python project.py test --copies <number>` | Runs this number of copies of each case at the same time. Each copy must pass. |
| `python project.py test --build` | Builds each artifact, the test ones too, and runs no case. |
| `python project.py test --workers <number>` | Runs this number of cases at the same time. With 1, the cases run one after the other. |
| `python project.py test --rounds <number>` | Runs all the cases this number of times, one round after the other. |

The copies of a case compete for the processors and the memory. Thus they find a fault that shows only under load.
A case that fails in some copies writes the number of these copies, for example `differs in 2 of 6 copies`.

## The settings

`[test]` of `.toolchain/project.toml` holds the settings of the tests:

| Key | Value |
| --- | --- |
| `timeout` | The maximum time of a case, in seconds. A case fails if its command does not end in time, also with `--break`. For a table, it is the maximum time of each row: *Tables*. |
| `copies` | The number of copies of each case that run at the same time. `--copies` replaces it. |
| `memory_floor` | Optional. The bytes of memory that must stay free when a case starts, while another case of the run runs. The default is 2 GiB, and 0 turns the wait off: `.toolchain/docs/test.py.md`, *The workers*. |
| `workers` | Optional. The number of cases that run at the same time. The default is the count of the processors. `--workers` replaces it: `.toolchain/docs/test.py.md`, *The workers*. |
| `env` | Optional. Changes to the environment of each case, as the `env` of a case: a text, or `false` to remove the variable. The `env` of a case replaces them. A project states here what keeps a verdict the same on each machine. |
| `slow` | Optional. The seconds above which a case is slow, and `test` writes its line. The default is 10: `.toolchain/docs/test.py.md`, *The times*. |
| `rule` | The rules of the groups. Each `[[test.rule]]` has `lacks` and `exempt`. |
| `writers` | The functions that write the messages of the code, for example `fprintf`. |
| `tables` | Optional. `true` makes `test` refuse a case that breaks a rule of *Its own process*. The default is `false`. |
| `uses` | Optional. The resources of the machine that 2 runs must not share, such as the GPU. The key is the name of a use, and the value is a list of patterns of the code. A group that tests a matched file waits for the other runs that hold the use: `.toolchain/docs/test.py.md`, *The uses*. |

A rule adds a claim to each case: the output does not contain the text `lacks`.
The rule does not apply to the groups of `exempt`.

## The messages of the code

If `[test] writers` names functions, `test` also checks the messages of the code after each case passed.
A message is the format of a call of a writer, in a file `.c` or `.h` of `src` or `include`.
A case must write each message, and a document must quote it.
`docs/contributing/documentation_style.md`, *The messages of the code* tells how a document quotes a message.
