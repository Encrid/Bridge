# .toolchain/tests/cases/commands.toml

The group `commands` checks `project.py`.
Its cases run `project.py` on the project in `.toolchain/tests/data/take`, with the output in `{case}/out`.

| Case | Claim | False variant |
| --- | --- | --- |
| the command fetch | `fetch` writes the path of zig. | `build`. |
| the command build | `build` writes `package.txt`. | `fetch`. |
| the command pack | `pack` writes the path of the zip. | `build`. |
| the command test | `test` writes `no case: tests/cases holds no case`. | `build`. |
| a command that project.py does not know | `project.py nothing` stops: `project.py: name a command: fetch build pack test check clean distclean cla`. | `test`. |
| the exit status of a tool | `build --target nowhere` ends with the status of `build.py`: `build.py: no target nowhere`. | No `--target`. |

The commands `check`, `clean`, `distclean` and `cla` have their cases in the groups of their tools.
