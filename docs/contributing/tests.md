# Tests

`python project.py test` builds the project, then runs the cases of `tests/cases`.
A case runs a command, and makes claims about its exit status, its output and its files.
`python project.py test --break` runs the false variant of each case. Each case must then fail.
`tests` can also hold other files, for example the data that the cases read. No check reads `tests`.

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

- A case checks one behavior. Its name tells the behavior.
- Each behavior that a document describes has a case.
- The false variant changes the command or its input. It cannot change the claims.
- A case writes its files only into its folder, `{case}`.

## Groups

A group is a file of cases in `tests/cases`, or in a folder of `tests/cases`.
The name of a group is the path of the file in `tests/cases`, without `.toml`.
For example, the name of the group `tests/cases/<folder>/<name>.toml` is `<folder>/<name>`.
If `tests/cases` holds no case, `test` writes `no case: tests/cases holds no case`.

## The keys of a case

| Key | Value |
| --- | --- |
| `name` | The name of the case. |
| `run` | The command, as a list. |
| `env` | Changes to the environment of the command. The key is the name of a variable. The value is a text, or `false` to remove the variable. |
| `stdin` | The text that `test` writes to the command, after the script of the case. |
| `script` | The steps that `test` does while the command runs. |
| `helper` | A second command that runs at the same time. |
| `files` | The text files that `test` writes before the command starts. The key is the path, and the value is the text. |
| `folders` | The folders that `test` makes before the command starts. The key is the path. The value is a list of artifacts or files to copy into the folder. |
| `claims` | The claims about the command. A case must make one claim or more. |
| `false` | The false variant of the case. Each case must have one. |
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

## Scripts

Each step of a `script` waits for a line of the output that contains the text `wait`.
Then the step does these actions, in this order:

1. `put` copies files. The key is the path, and the value is an artifact or a file.
2. `remove` removes the files of the list.
3. `type` writes this line to the command.

If the output ends before the text comes, `test` stops the steps.

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

For each line, write a case that runs the branch, or remove the branch.
A condition in an argument or in the body of a macro cannot get a probe. The run writes it, and stops.
Thus write such a condition outside the macro.

The probes change the program, thus the run does not judge the claims. It writes the verdicts only for information.
`.toolchain/docs/branches.py.md` tells how the probes work.

## The leaks

Each case must give back each resource that the code takes, also on the paths of the errors.
`python project.py test --leaks` builds the project with a count of the resources in each file `.c` of `src`.
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

## Run the tests

| Command | Result |
| --- | --- |
| `python project.py test` | Runs each case. |
| `python project.py test --break` | Runs the false variant of each case. Each case must fail. |
| `python project.py test --branches` | Runs each case one time on a build with probes. Each branch must run. |
| `python project.py test --leaks` | Runs each case one time on a build with counts. Each case must give back what the code takes. |
| `python project.py test -k <part>` | Runs only the cases whose name contains `<part>`. |
| `python project.py test --copies <number>` | Runs this number of copies of each case at the same time. Each copy must pass. |
| `python project.py test --rounds <number>` | Runs all the cases this number of times, one round after the other. |

The copies of a case compete for the processors and the memory. Thus they find a fault that shows only under load.
A case that fails in some copies writes the number of these copies, for example `differs in 2 of 6 copies`.

## The settings

`[test]` of `.toolchain/project.toml` holds the settings of the tests:

| Key | Value |
| --- | --- |
| `timeout` | The maximum time of a case, in seconds. A case fails if its command does not end in time, also with `--break`. |
| `copies` | The number of copies of each case that run at the same time. `--copies` replaces it. |
| `rule` | The rules of the groups. Each `[[test.rule]]` has `lacks` and `exempt`. |
| `writers` | The functions that write the messages of the code, for example `fprintf`. |

A rule adds a claim to each case: the output does not contain the text `lacks`.
The rule does not apply to the groups of `exempt`.

## The messages of the code

If `[test] writers` names functions, `test` also checks the messages of the code after each case passed.
A message is the format of a call of a writer, in a file `.c` or `.h` of `src` or `include`.
A case must write each message, and a document must quote it.
`docs/contributing/documentation_style.md`, *The messages of the code* tells how a document quotes a message.
