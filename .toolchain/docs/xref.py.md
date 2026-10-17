# .toolchain/tools/xref.py

`xref.py` checks that the documents of the project cite only what is there.
A document is a file `.md` of the project that `ours` gives.

```sh
python .toolchain/tools/xref.py [--root <folder>]
```

## The checks

`xref.py` does these checks on each document:

1. Each path names a file of the project.
2. Each citation names a document of the project, and a heading of that document.
3. Each reference names a heading of its own document.

A path is a code span that holds only folders and the name of a file, with `/` between them, for example `.toolchain/tools/shared.py`.
The name of the file has an extension.
The first folder is a folder of the layout: `.github`, `.toolchain`, `docs`, `include`, `src`, `tests`, `third_party` or `tools`.
Thus `xref.py` does not check a folder, a path in `build`, a name with no folder, or a pattern with `*` or `<`.
`xref.py` looks for the file of a path or of a citation on the disk. Thus a path can name a file of `tests`.

A citation is a code span, a comma, a space, and a heading between two asterisks.
A reference is a heading between two asterisks, with no citation before it.
For example, the first line is a citation, and the second line is a reference:

```text
`docs/contributing/commits.md`, *Commits*
*The checks*
```

A heading is a line that starts with 1 to 6 `#` and a space. The text of the heading is the rest of the line.
The text of a citation or of a reference is the full text of the heading.

## The method

`faults` reads one document in these steps:

1. It replaces each fenced block with spaces. Thus it does not read the blocks.
2. It replaces each line end with a space, if a character that is not a space follows it.
3. It checks each citation, then replaces each citation with spaces.
4. It checks each path.
5. It replaces each code span with spaces, then it checks each reference.

Step 2 joins the lines of a paragraph. Thus a citation can continue on the next line.
Each step keeps the length of the text. Thus the line of a fault is its line in the document.

## Output

The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.

| Fault | Line |
| --- | --- |
| A path or a citation names a file that is not there. | `<document>:<line>: <path> is not a file of the project` |
| A citation names a heading that is not there. | `<document>:<line>: <path> has no heading <heading>` |
| A reference names a heading that is not there. | `<document>:<line>: this document has no heading <heading>` |

## Functions

| Function | Result |
| --- | --- |
| `blank(match)` | Gives the text of a match with a space in place of each character. The line ends stay. |
| `headings(text)` | Gives the headings of a document, but not the lines of its fenced blocks. |
| `faults(root, docs, doc)` | Gives the faults of the document `doc`. `docs` gives the headings of each document. |
| `main()` | Reads the options, checks each document, and writes the faults. |
