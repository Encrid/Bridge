# .toolchain/tools/runner.py

`runner.py` writes the commands that give a runner of GitHub what the cases of the project need, for example a display or a graphics driver.
The workflows `ci` and `release` write these commands to a file, and run the file with `bash`, before the step `Fetch`:

```sh
python .toolchain/tools/runner.py --target <target> > "$RUNNER_TEMP/runner.sh"
bash -e "$RUNNER_TEMP/runner.sh"
```

A pipe into `bash` is not safe: a command that reads its input, such as `brew install`, takes the commands after it, and they never run.

The tool knows only the mechanism. What a need gives is data of the project, so the toolchain of a project holds no field of another project.

## The needs

`[ci] needs` of the configuration lists the needs of the project: `.toolchain/docs/project.toml.md`, *[ci]*.
Each name must have a table `[need.<name>]` in the configuration of the project.

A need has 1 table for each system that it serves: `[need.<name>.<system>]`.
The system is `linux`, `windows` or `darwin`, the first word of `runs_on` of a target in lower case.
A need with no table for a system gives nothing on that system.

| Key | Value |
| --- | --- |
| `packages` | Linux: the packages of `apt`. |
| `brews` | macOS: the packages of Homebrew. |
| `values.<cpu>` | A table of texts for 1 processor, as `runs_on` names it, for example `AMD64` or `arm64`. The placeholders `{name}` of `commands`, of `url`, of `sha256` and of the `members` take their text from the row of the processor of the target. |
| `archive` | An array of tables. Each archive has `url` and `sha256`, and both take the placeholders. The key `into` names a folder under `$RUNNER_TEMP` and the shell variable that holds it; the default is the name of the need. The key `file` names the downloaded file; the last part of the url is the default. The key `members` lists the parts to unpack; the whole archive is the default. The folders of the path of each member drop from it, so the member lands in the folder. |
| `commands` | Lines of shell, after the installs and the archives. They write `$GITHUB_ENV` and `$GITHUB_PATH` themselves. |

A placeholder is `{name}` in lower case. A command writes a name of shell in braces only in upper case, so a placeholder and a name of shell never meet.

## The commands

`runner.py` writes the commands for the target of `--target`, or for the target of this machine.
`runs_on` of the target gives its system and its processor.
The commands do these steps:

1. On Linux, when a need has packages: `sudo apt-get update`, then 1 `sudo apt-get install -y` with the sorted packages of every need.
2. On macOS, when a need has brews: 1 `brew install` with the sorted brews of every need.
3. Each need, in the order of `[ci] needs`, gives its archives and its commands. Each archive makes its folder, downloads the file, checks the digest and unpacks.
   On Windows the unpack uses `tar.exe` of Windows with `cygpath`, because the `tar` of Git Bash reads no `.7z`. On Linux and macOS it uses `tar -xf`.
   2 archives with the same `into` share 1 variable and 1 `mkdir`.
4. The `commands` of the need, with the placeholders filled.

With no need, or with no need on the system, `runner.py` writes nothing. Then `bash` does nothing.
The commands of a need run after the installs and the archives of every need before them.

## Output

`runner.py` writes each command on its own line, with the line end of Linux on each system. `bash` reads a carriage return as a part of a command.
It stops with these messages:

- `[ci] needs: <need> is not a need: <declared names>`
- `no machine runs the target <target>`
- `an archive into <folder> has no <key>`
- `the archive into <folder> is not a name of a shell variable`
- `the members of the archive into <folder> drop different counts of folders`
- `no variable {<name>} for <text>`: a placeholder has no text for the processor of the target

## Functions

| Function | Result |
| --- | --- |
| `installs(needs, system, declared)` | Gives the commands of step 1 and step 2. |
| `folder_lines(folder, system)` | Gives the lines that make the folder of an archive under `$RUNNER_TEMP`. |
| `archive_lines(archive, values, system, folder)` | Gives the commands that download an archive, check its digest and unpack it. |
| `need_lines(name, system, cpu, declared)` | Gives the commands of the archives and the `commands` of 1 need. |
| `commands(needs, machine, settings)` | Gives the commands of the needs for a machine, as `runs_on` names the machine. |
| `main()` | Reads the options and the needs, and writes the commands. |
