# .toolchain/tools/shared.py

`shared.py` holds what more than one tool uses.
The tools import it. It does not run alone.

## Constants

| Name | Value |
| --- | --- |
| `HERE` | The folder of `project.py`, which holds `.toolchain`, zig and the caches of zig. It is the default root. |
| `NAME` | The pattern of a variable: a name in braces, with an optional separator after a colon. |
| `KEYS` | The keys that a step, a case, a false variant, a helper, a step of a script, the claims and a source can have. It also gives the keys of a kind of `[leaks.kinds]`. |
| `SIDES` | The folder of `docs` that holds the documents of a folder: `extern` for `include`, and `intern` for `src`. |

## Functions

| Function | Result |
| --- | --- |
| `die(why)` | Stops the tool with the message `<tool>: <why>`, where `<tool>` is the file name of the tool. |
| `known(table, what, where)` | Gives `table`. It stops if `table` has a key that `KEYS[what]` does not have. |
| `merged(into, more, above)` | Adds the keys of `more` to `into`, as the section *The configuration* tells. |
| `config(root)` | Gives the configuration of the project in `root`. |
| `located(text, values)` | Gives the path of `text` in the root, after it replaces the variables. |
| `expand(text, values)` | Gives `text` with the value of each variable in place of the variable. |
| `expanded(items, values)` | Gives the items of a list with the value of each variable. |
| `files(root, patterns)` | Gives the files of `root` that match the patterns. Each file comes one time, in the order of the patterns. |
| `status(code)` | Gives an exit status as a decimal number and as a hexadecimal number. |
| `run(argv, root)` | Runs a command in `root`, and writes its output. It stops the tool if the command fails. |
| `this_target(targets)` | Gives the target whose `runs_on` is this machine, or nothing. |
| `sha256(data)` | Gives the SHA-256 digest of the bytes, in hexadecimal. |
| `variables(config, args, target, zig)` | Gives the variables of a build, as the section *Variables* tells. |
| `arguments(*valued, **flags)` | Reads the options of the command line. Each item of `valued` adds an option that has a value. An item can be a pair of the option and the type of its value, for example `('--copies', int)`. Each item of `flags` adds an option that has no value. |
| `header(mark)` | Gives a pattern for the lines of `.toolchain/header.txt`, each one after `mark`, as the section *The legal notice* tells. |
| `project_name(config)` | Gives `[project] name`, as the section *The name of the project* tells. |
| `document(path)` | Gives the document of a file, as the section *The documents* tells. |
| `repository(root)` | Tells if the root is the top folder of a git repository. |
| `ours(root, tests)` | Gives the files of the project, as the section *The files of the project* tells. |
| `report(faults, count, items)` | Writes the faults of a check, and ends the check, as the section *The report of a check* tells. |

## The configuration

`config` reads `.toolchain/toolchain.toml` of `HERE`, then `.toolchain/project.toml` of the root, then each `.toolchain/config/*.toml` of the root, in the order of the names.
`merged` adds each key of a later file to the configuration:

1. If the key names a table in the two files, `merged` merges the two tables in the same way.
2. Else, if `.toolchain/toolchain.toml` has the key, the tool stops.
3. Else, the key and its value go into the configuration.

The message names the full key, for example `project.toml states variables.language, and toolchain.toml states it too`.
Thus a list of tables, for example `[[artifact]]`, comes from one file only.

## Variables

`variables` gives the values in this order. A later value replaces an earlier value with the same name.

1. The variables that the tools give: `root`, `build`, `home`, `dist`, `out`, `target`, `python` and `zig`.
2. The keys of the target.
3. The keys of `[variables]`. The tool stops if a key has a name of step 1 or of step 2.
4. With `--debug`, the keys of `[debug]`.
5. The values of `--set`. The tool stops if `[variables]` does not have the name.

`build` is `--out`, or `[build] folder` in the root.
`dist` is `<build>/<target>`, or `<build>/<target>-debug` with `--debug`. It holds the deliverable that the build composes.
`out` is `<dist>/scratch`. It holds each built artifact, and the scratch of the tests.
The messages are `[variables] names a built-in variable or a key of a target: <names>` and `--set <name>=<value>: [variables] has no <name>`.

## Expansion

`expand` replaces each variable of a text with its value. If the value contains variables, `expand` replaces them too.
A list value in a text needs a separator: `{<name>:<separator>}`. `expand` joins the items with the separator.
`expanded` replaces a list variable that is a full item of a list with all its items.
`expand` stops the tool if a variable has no value, or if a list value in a text has no separator.

## The options

`arguments` reads the options `--root`, `--target`, `--part`, `--debug`, `--out` and `--set`.
A tool adds its own options:

- `arguments('--commits')` adds the option `--commits <value>`. Its value is in `args.commits`, or `None` if the option is not given.
- `arguments(pack='--pack')` adds the option `--pack`, with no value. `args.pack` tells if the option is given.

The default of `--root` is `HERE`. `arguments` makes the root an absolute path.
`--part` can be given more than one time. `args.part` is the list of its values, or `None`.

## The legal notice

`header` reads `.toolchain/header.txt` of `HERE`, and gives a pattern for each line of it:

1. It writes `mark`, a space and the line. For an empty line, it writes `mark` only.
2. `<year>` matches a year as `.toolchain/docs/header.txt.md` gives it.
3. `<holder>` matches any text. A line with `<holder>` matches one time or more.

`text.py` gives the mark `#`. `style.py` gives the mark `**`, and adds `/*` before the notice and `*/` after it.

## The name of the project

`project_name` gives the value of `name` in `[project]` of `.toolchain/project.toml`.
If the value is empty or not there, it stops the tool with the message that `.toolchain/docs/project.toml.md`, *[project]*, quotes.

## The documents

`document` gives the path of the document of a file, from the first folder of the path:

- The document of `include/<path>` is `docs/extern/<path>.md`.
- The document of `src/<path>` is `docs/intern/<path>.md`.
- The document of `tools/<path>` is `docs/tools/<path>.md`.
- A file `.gitkeep`, and a file of a different folder, have no document. Then `document` gives `None`.

The check of the layout and the check of the cover use `document`.

## The files of the project

If `repository` tells that the root is the top folder of a git repository, `ours` asks git for its files.
These are the files that git tracks, and the files that git does not ignore.
Else, `ours` walks the folders of the root, as `.gitignore` tells: it skips `build` at the root, and each `__pycache__`.
`ours` gives only the files that exist, as paths from the root.
It leaves out the files of `tests` and of `.toolchain/tests`. Thus no check reads the tests of the project or of the toolchain.

With `tests=True`, `ours` keeps the files of `tests`, and leaves out only `.toolchain/tests`. The check of the retired words uses it.

`repository` asks `git -C <root> rev-parse --show-toplevel` for the top folder of the repository of the root.
Thus a project in a folder of a different repository, for example a case in `build`, is not a repository.
The check of the commits also uses `repository`.

## The report of a check

Each check tool ends with `report`. `faults` is the list of the faults, one line for each fault.
`count` is the number of the things that the check read, and `items` is their name. Its default is `files`.

1. `report` writes each fault on its own line.
2. If there is a fault, it writes `faults: <number>`, and the check ends with the status 1.
3. Else, it writes `no fault in <count> <items>`, and the check ends with the status 0.

A check gives its faults in the order of the paths and of the lines, unless its document gives another order.

For example, the check of the commits gives `items='commits'`, and writes `no fault in 12 commits`.
