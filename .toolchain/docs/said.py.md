# .toolchain/tools/said.py

`said.py` checks the messages of the code.
A message is the format of a call of a writer, in a file `.c` or `.h` of `src` or `include`.
`[test] writers` of `.toolchain/project.toml` names the writers, for example `fprintf` or a function of the project.

```sh
python .toolchain/tools/said.py [--root <folder>] [--target <target>] [--debug] [--out <folder>]
```

At the end of a run that is not `--break`, `test.py` calls `checked` of `said.py`, if each case passed.

## The checks

1. A case wrote the message: the output of the cases has a line that matches the message.
2. A document quotes the message: a document of `docs/extern` or `readme.md` has a line that matches the message.

`said.py` reads the output of the cases in `<out>/said.txt`. `test.py` writes this file in each run that is not a run with `--break`.
`<out>` is the output folder of the target, as `.toolchain/docs/project.py.md`, *Output* tells.

## The format

The format is the first string of the arguments of the call. If more strings follow it directly, the format is all the strings together.
`said.py` changes the escapes `\"`, `\n` and `\\` to their characters, and removes the white space at the start and at the end.
A call with no string gives no message, for example the definition of the writer.

A conversion of the format, for example `%s` or `%d`, matches any text on the line.
Thus a document can write `<name>` in place of a conversion. `%%` matches the character `%`.
For example, this line of a document matches the format `"no file %s\n"`:

```text
`no file <path>`
```

The own characters of a format are the characters that are not in a conversion.
`said.py` does not check a format with less than 6 own characters, for example `"%s\n"`. Such a format has no words to find.

## Output

The count names messages.
The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.

| Fault | Line |
| --- | --- |
| No case wrote the message. | `<file>:<line>: no case wrote: <message>` |
| No document quotes the message. | `<file>:<line>: no document quotes: <message>` |

If `<out>/said.txt` is not there, `said.py` stops: `<out>/said.txt is not there: run test first`.

## Functions

| Function | Result |
| --- | --- |
| `called(text, at)` | Gives the arguments of a call: the text between the parenthesis at `at` and the parenthesis that closes it. |
| `messages(text, writers)` | Gives the line and the format of each call of a writer in the text. |
| `pattern(message)` | Gives the pattern that matches the message, and the number of its own characters. |
| `checked(root, out)` | Checks each message of the project in `root`, with the output of the cases in `out`, and writes the faults. |
| `main()` | Reads the options, finds the output folder of the target, and calls `checked`. |
