# src/lib/config/config_kept.c

This file keeps each share of an unloaded module that stays, and gives it back when it is free.
Why the settings text needs it: `docs/intern/lib/config/config.h.md`, *The text of the settings*.

## The list of shares

The world keeps a list of entries, one for each share of an unloaded module that stays.
An entry is the stem of the module, the place of its settings text, and the ranges of the share.
An unload adds an entry when the share of the module stays.
Each entry is a share that is still held, so `[budget] persistent` bounds the list, and no count does.

After each unload, `config_give_back_free` looks at each entry.
If `is_kept_held` gives false, the share goes back to the arena, and the entry goes: `docs/extern/lib/bridge.h.md`, *An unload*.
When an entry goes, the last entry of the list takes its place.

## The next load

`config_find_kept` gives the newest entry of a stem.
The next load of the module writes its settings text there, if it fits: `docs/intern/lib/config/config_room.c.md`.
Then the row of the settings holds a string in that share, and the share stays.

## No memory

An unload does not fail for want of memory.
If there is no memory for one more entry, the unload goes on, and a record says what is lost.
What is lost is the share: the arena does not get it back.

## Functions

| Function | Result |
| --- | --- |
| `config_find_kept` | Gives the newest entry of the module `stem`, or `nullptr`. |
| `add_kept` | Makes an entry for the share of an unloaded module, with a copy of its ranges. Gives `nullptr`, and says so, if there is no memory for it. |
| `config_keep` | Keeps the share of an unloaded module that stays, and the place of its settings text. |
| `is_kept_held` | Gives whether a string of a row that stays lies in the ranges of an entry. |
| `config_give_back_free` | Gives back each kept share that no row holds a string in, and forgets it. |
