# src/lib/config/config_room.c

This file finds where the text of the settings of a module goes, before any of it is written.

## The room

Only a text for a `bridge_text_t` field takes room in the persistent arena.
Each such text takes its length rounded up to 8 bytes.
A text for a char array, and any other setting, takes no room there.

## Where the text goes

1. Add up the room of every setting. If it is 0, nothing more is needed.
2. If the module had a block of text at its last load, and the text fits there, use that block.
3. If not, take the room from the start of what is left of the persistent share of the module.

If the share has not the room, or the module has no share, the load is refused.
The refusal gives the bytes of text, and the bytes left in the share.

## Functions

| Function | Result |
| --- | --- |
| `get_text_size` | Gives how many bytes of the arena one setting takes: its text rounded up to 8 for a `bridge_text_t` field, and 0 for any other. |
| `count_text` | Gives how many bytes of the arena all the settings take. |
| `reuse_kept` | Takes the block that the text of the module had at its last load, if `total` bytes still fit there. |
| `config_place_text` | Finds where the text of the settings goes, before any is written: the block it had, or the persistent share of the module. Gives 1 if there is no room. |
