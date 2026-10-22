# src/host/toml/toml_text.c

This file reads a quoted text and resolves its escapes.

## The escapes

A single-quoted text is literal.
A double-quoted text takes an escape that opens with a backslash.
`read_escape` maps the short escapes for a backspace, a tab, a line feed, a form feed, a carriage return, a quote and a backslash.
`read_code` reads a 4-digit or 8-digit hexadecimal code and writes it as UTF-8.

`read_code` refuses too few digits, a code above the highest point, and a surrogate code.
`put_utf8` encodes a code point as one to four bytes.
The reader also turns down a text over lines and a text with no end.

## Functions

| Function | Result |
| --- | --- |
| `toml_read_text` | Reads a quoted text into fresh memory, or refuses and gives null. |
