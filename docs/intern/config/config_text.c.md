# src/config/config_text.c

This file writes a setting into a field that holds text, and refuses a setting for a field that no setting fills.

## Where a text goes

The type of the field says where a text goes, and the author chooses it when the author declares the field:

| Field | Where the text goes |
| --- | --- |
| `char name[N]` | The bytes, inline in the value, and a zero after them. Text that does not fit is refused. |
| `bridge_text_t` | The offset and the length of the bytes in the persistent arena. |
| An enum | The constant of that name: `docs/intern/config/config_enum.c.md`. |

Bounded text goes in the column, and text with no bound goes in the arena.
A text that lives for less than a phase goes in the scratch arena, and no setting goes there.

## A bridge_text_t

The bytes of the text go into the private copy of the texts, after the texts written before it.
`config_keep_text` copies them into the block of text of the module only when no setting was refused: `docs/intern/config/config_room.c.md`.
Each text takes the room that `docs/intern/config/config_room.c.md`, *The room*, gives.
No zero ends the text: its length tells where it ends.
The field gets the offset of the bytes and their length.
A setting that is not text is refused as a misfit.

## A char array

The bytes go inline, and the zero after them comes from the value, which starts as zeros.
A text that does not fit with its zero is refused, with its length and the length of the field.
An array that is not of `char`, and a setting that is not text, are refused as misfits.

## A struct

No setting fills a field that is a struct, so any setting for one is refused as a misfit.

## Functions

| Function | Result |
| --- | --- |
| `get_text_len` | Gives the length of a text setting, or 0 for a setting that is not text. |
| `put_text` | Copies the bytes of a text setting into the private copy of the texts. Gives where they will be in the block of text. |
| `config_seed_text` | Writes a text into a `bridge_text_t`: where its bytes are in the persistent arena, and how many. Gives 1 if it refuses. |
| `config_seed_struct` | Refuses a setting for a field that is a struct. Gives 1. |
| `config_seed_array` | Writes a text into a char array: the bytes, inline, and a zero after them. Gives 1 if it refuses. |
