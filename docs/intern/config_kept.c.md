# src/config_kept.c

This file remembers where the settings text of an unloaded module is, for its next load.
Why: `docs/intern/config.h.md`, *The text of the settings*.

## The list of places

The world keeps a list of places, one for each unloaded module whose share stays.
An entry is the stem of the module, and the offset and the length of the block of its text.
The list grows by one when such a module unloads, and loses the entry when the share goes back to the arena.
Each entry is a share that is still held, so `[budget] persistent` bounds the list, and no count does.

A module with no settings text leaves no entry.
An entry that exists for the stem is written again.
When an entry goes, the last entry of the list takes its place.

## No memory

An unload does not fail for want of memory.
If there is no memory for one more entry, the unload goes on, and a record says what is lost.
What is lost is that the next load of the module does not write over the old block, and takes fresh room.

## Functions

| Function | Result |
| --- | --- |
| `config_find_kept` | Gives what is remembered of the settings text of the module `stem`, or `nullptr`. |
| `add_kept` | Makes room to remember the text of one more module. Gives `nullptr`, with a record, if there is no memory for it. |
| `config_keep` | Remembers where the settings text of an unloaded module is. |
| `config_forget` | Forgets where the settings text of a module was. |
