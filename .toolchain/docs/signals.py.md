# .toolchain/tools/signals.py

`signals.py` starts the command of a case, and sends the signals of its script.
`test.py` imports it. It does not run alone.

## Functions

| Function | Result |
| --- | --- |
| `allowed(use, path)` | Stops if a case of the file `path` has a script or a key `ignore` that *The checks* refuses. |
| `started(argv, root, env, use)` | Starts a command in `root`, with pipes for its input and for its output. Its errors go into its output. |
| `sent(proc, name)` | Sends the signal `name` to the command. |
| `drained(screen)` | Reads the screen of a console until the screen closes, and drops the text. |
| `Console(argv, root, env, grouped)` | Starts a command on a console of its own, on Windows. With `grouped`, the command gets a process group of its own. |

## The start

On Linux and on macOS, `started` starts the command with `subprocess.Popen`.
With `ignore`, the command starts through `sh -c 'trap "" INT; exec "$0" "$@"'`. Thus the command starts with the interrupt ignored.

On Windows, `started` starts a `Console` if the script sends a signal or the case has `ignore`. Else it uses `subprocess.Popen`.
A `Console` is a pseudo console of Windows. The input and the output of the command are pipes, thus the output holds no text of the screen.
The command inherits only these 2 pipes. Thus a command of another copy gets no pipe of this command.

The command gets a process group of its own if the case has `ignore`, or if the script sends a termination. Such a group ignores ctrl-c.
A process passes to its commands a flag that ignores ctrl-c. Thus `Console` sets this flag of `test` to false before it starts the command.

## The signals

| Signal | Linux and macOS | Windows |
| --- | --- | --- |
| `interrupt` | `SIGINT` | ctrl-c: `Console` writes the character 3 to the console. |
| `termination` | `SIGTERM` | ctrl-break: a Python program with no console attaches to the console of the command, and calls `GenerateConsoleCtrlEvent`. |
| `hang-up` | `SIGHUP` | `Console` closes the console. |

`Console` closes the console one time, also when the command ends.
If a call of Windows fails, `test` stops: `<call> failed: error <number>`.

## The checks

`allowed` checks a case, and its false variant, before `test` runs a case. It stops with `<path>: the case <name>` and one of these texts:

| Fault | Text |
| --- | --- |
| `ignore` is not `["interrupt"]`. | `ignores <value>, and only interrupt can be ignored` |
| A step waits after the output closed. | `waits for a line after it closes the output` |
| `close` is not `"output"`. | `closes <value>, and only the output can close` |
| A signal is not known. | `sends <value>, and the signals are interrupt, termination and hang-up` |
| A step sends a signal after a hang-up. | `sends a signal after a hang-up, and on Windows a hang-up ends the console` |
| A pause is not a number above 0. | `pauses <value>, and a pause is a number of seconds above 0` |
| The script sends an interrupt and a termination. | `sends an interrupt and a termination, and on Windows a termination turns off the interrupt` |

A step with a key that a step of a script does not have also stops `test`: `<path>: a script has no key <key>`.
The checks are the same on each system. Thus a case runs the same way on each runner.
