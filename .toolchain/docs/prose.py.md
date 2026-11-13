# .toolchain/tools/prose.py

`prose.py` checks the rules of STE that a tool can count, the tags of the blocks of code, and the readme.
The persons who write and review a document check the other rules. `docs/contributing/documentation_style.md` gives them.

## The documents

`prose.py` checks `readme.md`, each file `docs/**.md` and each file `.toolchain/docs/**.md`.
It does not check `license` and `cla.md`, because they are legal texts.

## Functions

| Function | Result |
| --- | --- |
| `documents(root)` | Gives the path and the lines of each document that `prose.py` and `pseudo.py` check. |
| `blocks(lines)` | Gives the first line, the last line and the tag of each block of code. `pseudo.py` uses it too. |
| `units(lines)` | Gives each paragraph, each item of a list and each cell of a table, with its first line. |
| `faults(lines, limits, tags)` | Gives the faults of a document. `tags` is the list of the tags of the blocks of code. |
| `readme(lines, name, most, needed)` | Gives the faults of a readme whose title is `name`, and which needs the sections of `needed`. |
| `main()` | Runs the check. |

## The limits

`[prose]` of `.toolchain/toolchain.toml` gives the limits.
`.toolchain/docs/toolchain.toml.md`, *[prose]* describes each key.

## Sentences

`units` cuts the text of a document into units:

- A paragraph is a group of lines between empty lines. A heading, a table, an item and a block of code also end a paragraph.
- An item of a list starts with `-`, `*`, `+`, or a number and a `.`. The next lines that start with a space are also in the item.
- Each cell of a table is a unit.

A heading and the lines of a block of code are not in a unit.
A sentence ends with a `.`, a `!` or a `?` before a space, or at the end of its unit.
A word is a group of characters between spaces, with a letter or a digit in it.
A span of code between two backticks is one word.

The messages are:

- `<path>:<line>: <number> words in a sentence, and <maximum> is the most`
- `<path>:<line>: <number> sentences in a paragraph, and <maximum> is the most`

`<line>` is the line where the sentence or the unit starts.
The maximum of a sentence in an item of a numbered list is `step`. The maximum of each other sentence is `sentence`.

## Blocks of code

A block of code starts and ends with a line that starts with three backticks.
The text after the first three backticks is the tag of the block.
The tag is one of `[prose] tags` of `.toolchain/toolchain.toml`, or one of `[check] tags` of the configuration.
The message is `<path>:<line>: the tag '<tag>' is not one of <tags>`. `<tags>` is the list of these tags, with a space between 2 tags.

## The readme

Standard Readme is a specification of the readme of a project: https://github.com/RichardLitt/standard-readme.
`readme` checks these rules of Standard Readme:

1. The first line is `# <name>`. `<name>` comes from `shared.project_name`.
2. The description is the first line with text after the title. It is not a heading.
3. The description has at most `[prose] description` characters.
4. The sections follow the order of Standard Readme. A section starts with `##`.
5. The readme has the sections Install, Usage, Contributing and License.

The readme of a part, `docs/usage/<folder>.md`, follows the same rules, with 2 changes.
Its title is `# <name>` with the `name` of the part. It needs only the sections Install and Usage.
Its messages start with its path in place of `readme.md`.

The order is Table of Contents, Security, Background, Install, Usage, the extra sections, API, Maintainers, Thanks, Contributing and License.
A section can also have the title Maintainer, Credits or Acknowledgements, as Standard Readme allows.
A section with a different title is an extra section.

The messages are:

- `readme.md:1: the title is not # <name>`
- `readme.md:1: no description after the title`
- `readme.md:<line>: <number> characters in the description, and <maximum> is the most`
- `readme.md:<line>: the section <title> must come before <title>`
- `readme.md:1: no section <title>`

## Output

The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.
