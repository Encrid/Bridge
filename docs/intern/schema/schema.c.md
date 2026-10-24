# src/schema/schema.c

This file gives `schema_parse` and `schema_free`: a type read whole from its text, or refused whole.
What a descriptor may say is `docs/intern/schema/schema.h.md`, *The type set*.

## The steps

1. `schema_parse` empties `out`, and starts a reader at the first character of the text.
2. It sets the last value to -1, below zero, so that the first counted constant is 0.
3. It reads the first token.
4. `parse_body` reads the body that the kind of the source names.
5. `check_end` refuses a text that goes on after its closing brace.
6. After a refusal, `schema_free` frees what the reading made.

For a kind other than `BRIDGE_STRUCT`, `parse_body` reads constants: those of a bitmask for `BRIDGE_BITMASK`, and those of an enum for any other kind.
For a struct, the text must begin with a brace.

## Functions

| Function | Result |
| --- | --- |
| `parse_body` | Reads constants with `schema_parse_constants`, or a struct with `schema_parse_struct` after its opening brace. Gives 1 if refused. |
| `check_end` | Reads the next token, and refuses it if the text has one after its closing brace. Gives 1 if refused. |
| `schema_parse` | Lays out a type from its text. Gives 0 and fills `out`, or gives 1 and writes `why` of the source, with nothing left allocated. |
| `schema_free` | Frees the name of each field and of each constant, and both arrays. Leaves the type empty. |
