# .toolchain/tools/text.py

`text.py` checks the characters, the legal notices and the comments of the project.
It also checks the words and the calls that the project refuses.
It reads the files that `ours` gives.

## Functions

| Function | Result |
| --- | --- |
| `notice()` | Gives the pattern of the legal notice of a Python file, from `shared.header`. |
| `python(text, notice)` | Gives the faults of a Python file: its notice, its comments and its string statements. |
| `faults(path, data, notice, unicode)` | Gives the faults of one file. `unicode` is true if the file can hold characters that are not ASCII. |
| `retired(text, words)` | Gives a fault for each retired word of `words` in `text`. |
| `refused(args, settings)` | Gives each name of the files of `[check] refuse`, with the file as the configuration writes it. |
| `called(text, names)` | Gives a fault for each call of a refused name in `text`. |
| `main()` | Runs the check. |

## The characters

Each character of a file is a character of ASCII.
`[check] unicode` in `.toolchain/project.toml` can name some files. These files can also hold other characters.
A path of `[check] unicode` can contain `*`, as a pattern of `glob` in Python.
The default of `[check] unicode` is an empty list.

The message is `<path>:<line>: a character that is not ASCII`.
`text.py` writes it one time for each line that has such a character.

## The Python files

A file `.py` starts with the legal notice of `.toolchain/header.txt`, each line after `# `:

```text
# Copyright (C) <year> <holder>
#
# Under the GNU Affero General Public License, version 3 only: see license.
```

`.toolchain/docs/header.txt.md` gives the values of `<year>` and `<holder>`.
More lines of the copyright can follow the first line, one line for each holder.

A Python file has no other comment, and no string statement.
A string statement is a statement that is only a string, for example a docstring.
`text.py` finds the comments with the module `tokenize`, and the string statements with the module `ast`.
Thus a `#` in a string is not a comment.

The messages are:

- `<path>:1: the file does not start with the legal notice`
- `<path>:<line>: a comment`
- `<path>:<line>: a string statement`

## The TOML and YAML files

A file `.toml` or `.yml` has no comment.
A comment starts with a `#` that is not in a string.
A string starts and ends with `"`, `'`, `"""` or `'''`.
In a string between two `"`, a `\` escapes the next character.
The message is `<path>:<line>: a comment`.

## The Markdown files

A file `.md` has no HTML comment.
An HTML comment starts with the four characters `<`, `!`, `-` and `-`.
The message is `<path>:<line>: a comment`.

## Retired words

`[check.retired]` of `.toolchain/project.toml` holds the retired words of the project.
`text.py` refuses each retired word as a whole word, in upper or lower case.
It reads the files of `ours`, also the files of `tests`, but no file of `.toolchain`.
Thus the table itself is no fault, and a project never gets a fault that only the toolchain can fix.

The messages are:

- `<path>:<line>: '<word>' is retired: write '<new>'`
- `<path>:<line>: '<word>' is retired`, when the table gives no new word

## Refused calls

`[check] refuse` of the configuration names the files that list the calls that the project refuses.
Each line of such a file is the name of a function. A line with no text names nothing.
For example, a project names the list of the functions that a library which it links does not implement.

`text.py` reads each file `.c` and `.h` of `src` and of `include`.
A call is the name, then `(`, with only spaces between them. A letter, a digit or `_` just before the name makes a different name.

A path of `[check] refuse` can hold variables.
`text.py` makes the variables of the build of this machine with `prepared` of `build.py`. Thus it needs the pinned zig.
Then it adds the variable of each source with `sources` of `fetch.py`, which stops if a source is not fetched.
The variable of a dependency or of a tool is not there.

The messages are:

- `<path>:<line>: '<name>' is refused by <file>`. `<file>` is the path as the configuration writes it.
- `[check] refuse: <path> is not there`, when a file of `[check] refuse` is not there. `<path>` is the path with the values of its variables. Then `text.py` stops.

When a source of a path is not fetched, `text.py` stops with the message of `sources`. Run `fetch`, then `check`.

## Output

The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.
