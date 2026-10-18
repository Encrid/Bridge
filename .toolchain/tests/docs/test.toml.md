# .toolchain/tests/cases/test.toml

The group `test` checks `.toolchain/tools/test.py`.
The first seven cases use the keys of a case. Their commands are short Python programs, thus they need no build.
The other cases run `test.py` on a project in `{case}`. Their `folders` copy the files of `.toolchain/tests/data/test` into it.

| File | Content |
| --- | --- |
| `.toolchain/project.toml` | A project with no artifact, and a timeout of 60 seconds. |
| `note.txt` | The line `written at the prompt`. |
| `passes.toml` | The case `passes`, which passes. |
| `fails.toml` | The case `fails`, which ends with the status 1. |
| `alone.toml` | The case `alone`, which has no false variant. |
| `silent.toml` | The case `silent`, which claims nothing. |
| `colour.toml` | The case `colour`, which has the key `colour`. |
| `late.toml` and `late/project.toml` | The case `late`, which waits 9 seconds, and a timeout of 2 seconds. |
| `said.toml` and `ruled/project.toml` | The case `said`, which writes `ended after`, and a rule that refuses this text, but not in the group `free/said`. |
| `still.toml` | The case `still`, whose false variant also passes. |
| `later/project.toml` | An artifact that links `linked.txt`, which is not there. Its kind has `unlinked` steps. |
| `nowhere/project.toml` | The project of `.toolchain/project.toml`, and the target `nowhere`, whose `runs_on` is empty. |

| Case | Claim | False variant |
| --- | --- | --- |
| a script | The script waits for `? name`, puts `note.txt` and types `you`. The lines come in order, and the file is there. | The script types `nobody`, and puts no file. |
| a helper | The command writes the line that the helper wrote into a file. | No helper. |
| a variable of the environment, given | `env` gives `C23_SAID` the value `port {port}`. | No `env`. |
| a variable of the environment, taken away | `env` removes `ZIG_LOCAL_CACHE_DIR`, thus the command writes `unset`. | No `env`. |
| claims on the status and on the output | `stdin` gives `two`. The command writes `one two`, and ends with 3. `status`, `status_not`, `has` and `lacks` hold. | `stdin` gives `three`. |
| a script that removes a file | The script removes `gone.txt` that `files` wrote, thus the command writes `False`. | The script removes no file. |
| a folder of files | `folders` copies `note.txt` into `{case}/in`. | The folder is empty. |
| a case with no false variant | `test` stops: `the case alone has no false variant`. | `passes.toml` in place of `alone.toml`. |
| a case that claims nothing | `test` stops: `the case silent claims nothing`. | `passes.toml`. |
| a case with a key that no case has | `test` stops: `a case has no key colour`. | `passes.toml`. |
| a case that differs | `test` writes `fails: differs`, the status and the output. Then it stops: `not as written: fails`. | `passes.toml`. |
| a case that does not end in time | `test` writes `late: did not end in 2 seconds`, and stops. | `passes.toml`. |
| a rule of the tests | The group `held/said` fails, and the group `free/said` passes. | `passes.toml` in `held`. |
| no case | `test` writes `no case: tests/cases holds no case`, and ends with 0. | `passes.toml`. |
| a false variant that still holds | `test --break` stops: `still as written with their claims made false: still`. | `passes.toml`. |
| a target that does not run here | `nowhere/project.toml` is the project. `test --target nowhere` stops: `nowhere does not run here`. | No `--target`. |
| a case that needs what is not linked | `test` writes `compiled, not linked: later:`. Then it stops: `the cases need what is linked: later`. | `linked.txt` is there. |
| the output of the cases | `test` runs the case `said`, and `{case}/out/{target}/said.txt` holds its output `ended after`. | `test --break`, which writes no `said.txt`. |
| a claim of a pattern | The output matches `first.*second: \d+` over two lines. | The two lines in the other order. |
| a claim of a pattern that is not found | The output has `hop A` one time, and the claim refuses `hop A.*hop A`. | The output has `hop A` two times. |
| a claim of the whole output | The output is the same as `.toolchain/tests/data/test/output.txt`. | The output lacks its second line. |
| copies of a case | `test --copies 3` runs 3 copies of `ran.toml`; the third copy writes `cases/ran/0/2/ran.txt`. | No `--copies`, thus one copy and no folder `2`. |
| a case that runs alone | With `--copies 3`, the case `lone` (`alone = true`) runs one time, in `cases/lone/0`. | `ran.toml` in place of `lone.toml`. |
| copies that differ | `test --copies 2` on `fails.toml` writes `fails: differs in 2 of 2 copies`. | No `--copies`. |
| rounds of the cases | `test --rounds 2` writes `round 2 of 2`. | No `--rounds`. |
| the cases of a part of a name | `test -k second` runs `second case` of `two.toml`, and not `first case`. | `-k first`. |
| a part of no name | `test -k nothing` stops: `no case has nothing in its name`. | `-k second`. |
| the groups of a part | `test --part alpha` runs the group `alpha/passes`, and not `beta/passes`. | `--part beta`. |
| a part of no group | `test --part nothing` stops: `no case group is in the part nothing`. | `--part alpha`. |
| a project with no copies | The `.toolchain/project.toml` of the project has no `[test] copies`: `test` stops. | The project gives `copies = 1`. |
