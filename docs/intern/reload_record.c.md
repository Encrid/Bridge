# src/reload_record.c

This file makes the record of the instance that a reload brings in. The new record takes over what was set aside for the module.

## What the new record takes over

The new record is made from the new config, as at a load: `src/module_record.c`.
Then it takes copies of what the old record holds for the module:

- The ids held ready for the old instance. They are the ids of the new instance from the swap on.
- The settings component that the config seeded, and where its text is.
- The share of the persistent arena, with what the reload added to it.

Until the swap these are copies, so a reload that fails leaves the old record as it was.
If there is no memory for the new record, a record says so, and the reload gives back what it added to the share.

## The ids

The array of ids of the new record holds all the ids held ready for the old one, whatever `spawns` the new config states.

## The share

The new share is the old share and what the reload added: `src/reload_share.c`.
If the last range grew where it is, the new record has the same ranges, and the last one is longer.
If not, the new record has one range more, and `alloc` moves to it when what is asked does not fit in the range before.
So the list of ranges of the new record has room for the ranges of the old share, and one more.
A module that had no share starts to allocate from the new range.

## Functions

| Function | Result |
| --- | --- |
| `alloc_record` | Makes the new record, with room for the ranges of the old share and one more, and for each id held ready. Null, with a record, if there is no memory. |
| `take_ids` | Copies the ids held ready for the old instance, the settings component, and where the text of the settings is. |
| `take_share` | Copies the share of the old instance, with what the reload added to it. |
| `reload_make_record` | Makes the new record and fills it. 1, with a record, and the added bytes of the share given back, if there is no memory. |
