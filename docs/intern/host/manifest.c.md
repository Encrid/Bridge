# src/host/manifest.c

This file reads the words of a manifest once the section is found.

## The words

A manifest is a run of words, and a zero byte ends each word.
The first word is the identity; it must carry a `:` and an `@` and no `=`.
`read_word` stores the identity, or splits a later word at `=` and hands the pair to `manifest_take`.
`parse` walks the words, rejects a word that is too long, and demands an identity with an `abi`.
`manifest_read` first finds the section for the format, then parses it.

## Functions

| Function | Result |
| --- | --- |
| `manifest_read` | Fills a `manifest_t`, or gives the reason that it refused. |
| `manifest_get_value` | Locates one key among the words and points at its raw value. |
