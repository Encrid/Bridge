# Documentation style

This document tells how to write the documents of the project.
`python project.py check` checks each rule that a program can check. A person checks the other rules.

## The documents

| Document | Content |
| --- | --- |
| `readme.md` | What the project is, how to install it, and how to use it. Its section Contributing lists the documents of `docs/contributing`. |
| `docs/contributing/<name>.md` | A rule of a contribution, for the contributors. |
| `docs/extern/<path>.md` | The document of `include/<path>`, for the users of the project. |
| `docs/intern/<path>.md` | The document of `src/<path>`, for the developers of the project. |
| `docs/tools/<path>.md` | The document of `tools/<path>`, for the developers of the project. |
| `.toolchain/docs/<name>.md` | The document of a part of the toolchain. |

Each file of `include`, of `src` and of `tools` has its document.
No other file is in `docs/extern`, `docs/intern` or `docs/tools`.
The first line of the document of a file is `#`, a space, and the path of the file, for example `# src/parse.c`.

## The readme

The readme follows Standard Readme: https://github.com/RichardLitt/standard-readme.

1. The first line is `#`, a space, and the value of `[project] name` of `.toolchain/project.toml`.
2. The next line with text is a description of the project, with at most 119 characters.
3. The sections Install, Usage, Contributing and License are necessary.

The sections come in this order: Table of Contents, Security, Background, Install, Usage, the extra sections, API, Maintainers, Thanks, Contributing, License.

## The document of a file

Start with the first line, then tell in one or two sentences what the file gives.
Then write the sections that the file needs.

A document of `docs/extern` describes a public header for its users:

- Write a section for each function, type, constant and macro of the header.
- Give the parameters, the result, the errors and the messages.
- Write each public name in a code span, for example `parse_line`. The check needs each public name in a code span.

A document of `docs/intern` describes a source for the developers:

- Write a section for each part that a developer must know: the data, the steps and the limits.
- Write the steps in pseudo code if the text alone is not clear.
- End with the section Functions: a table with each function and its result.

## Simplified Technical English

Write each document in Simplified Technical English (STE).
The reference is ASD-STE100 Simplified Technical English, Issue 9, of 2025-01-15.
You can get a copy free of charge: https://www.asd-ste100.org/STE_downloads.html.

The check counts these rules. `[prose]` of `.toolchain/toolchain.toml` gives the limits.

- A sentence of a procedure has at most 20 words. A procedure is a numbered list.
- Each other sentence has at most 25 words.
- A paragraph has at most 6 sentences.

A person checks the other rules of STE, for example these rules:

- Use the active voice.
- Write one instruction in a sentence.
- Use at most 3 nouns in a noun cluster.
- Use the words of the dictionary of STE with their one meaning, and the technical names of the project.

## Blocks of code

A block of code has a tag: `c`, `toml`, `sh`, `text`, `pseudo` or `mermaid`.
A block with the tag `pseudo` holds pseudo code.

The pseudo code is a subset of the pseudo code of Cambridge International.
The reference is the Pseudocode Guide for Teachers of Cambridge International AS & A Level Computer Science 9618, for the exams of 2026:
https://www.cambridgeinternational.org/Images/697401-2026-pseudocode-guide-for-teachers.pdf.
The subset is different from the guide in these points:

- The assignment is `<-`. The guide uses an arrow, which is not a character of ASCII.
- The names are the names of the code, for example `rows_of` or `MAX_ROWS`.
- The subset adds the types with a size: `U8`, `U16`, `U32`, `U64`, `I8`, `I16`, `I32`, `I64`, `F32` and `F64`.
- A statement has one line. A clause of `CASE` has one statement.
- `NEXT` names the variable of its `FOR`. `CALL` always has parentheses.
- The subset has no comment, no enumerated type, no pointer, no set, no file, no class, no input and no output.

Each level of a block adds 3 spaces. `ELSE` and the end of a block have the level of the start of the block.
The clauses of a `CASE` have one level more than the `CASE`. The keywords are in upper case.

```pseudo
FUNCTION largest(a : I32, b : I32) RETURNS I32
   IF a > b THEN
      RETURN a
   ENDIF
   RETURN b
ENDFUNCTION
```

## Citations

A path in a code span names a file of the project. The check refuses a path that names no file.
To cite a section of a different document, write its path in a code span, a comma, and its heading between two asterisks.
To cite a section of the same document, write its heading between two asterisks.

```text
`docs/contributing/coding_style.md`, *The legal notice*
*The readme*
```

## The messages of the code

`readme.md` or a document of `docs/extern` quotes each message of the code.
`docs/contributing/tests.md`, *The messages of the code*, names the functions that write them.
In a quote, write `<name>` in place of a conversion such as `%s`.

## Characters

Each character is a character of ASCII, except in the files that `[check] unicode` of `.toolchain/project.toml` names.
No document has a comment.
