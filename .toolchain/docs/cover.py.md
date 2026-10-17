# .toolchain/tools/cover.py

`cover.py` checks that the documents of `docs/extern` cover each public name of the project.

```sh
python .toolchain/tools/cover.py [--root <folder>]
```

## The public names

The public names come from the headers of `include`:

| Name | Example | The name |
| --- | --- | --- |
| A function: a prototype, or a function `static inline`. | `int sample_add(int a, int b);` | `sample_add` |
| A macro with arguments. | `#define SAMPLE_MAX(a, b) ((a) > (b) ? (a) : (b))` | `SAMPLE_MAX` |
| A constant: an item of an `enum`, or a `constexpr`. | `constexpr int SAMPLE_SIZE = 4;` | `SAMPLE_SIZE` |
| A layout: a `struct` or a `union`. | `typedef struct { int x; } sample_point_t;` | `sample_point_t` |
| A field of a layout. | `typedef struct { int x; } sample_point_t;` | `x` |

The name of a layout is the name of its `typedef`. If the layout has no `typedef`, its name is the name of its tag.
A field that is a pointer to a function gives the name of the pointer.
`cover.py` removes the attributes of C23 before it reads a header, for example `[[nodiscard]]`. Thus a prototype can start with an attribute.
A function is a line that starts with a letter or `_`, with no `typedef`, and with a name before its first parenthesis.

## The check

Each public name of a header is a word of a code span of the document of the header.
`shared.document` gives the document of the header: `.toolchain/docs/shared.py.md`, *The documents*.
A name counts only when no letter, digit or `_` touches it.
If a document is not there, `cover.py` reads it as an empty document.

## Output

Its faults follow the headers.
The output follows `.toolchain/docs/shared.py.md`, *The report of a check*.
A fault is `<header>: <name> is not in <document>`.

## Functions

| Function | Result |
| --- | --- |
| `named(name, text)` | Tells if the text has the name as a word. |
| `names(text)` | Gives the public names of the text of a header. |
| `spans(root, doc)` | Gives the code spans of a document, one on each line. |
| `main()` | Reads the options, checks each header, and writes the faults. |

## Limits

`cover.py` reads the code with patterns, not with a parser of C.

- It does not read a layout in a layout. It stops the outer layout at the first `}`.
- It does not read the name of a bit field.
- It does not read a `typedef` that is not a layout, for example a pointer to a function.
