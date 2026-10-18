# src/lib/module/module_pointed.c

This file tells whether a row holds a string that lies in a share of the persistent arena.
The share is given by its ranges: those of a module, or those of a share that stayed.
An unload reads it to know whether the share can go back: `docs/intern/lib/module/module_share.c.md`, *When a share goes back*.

## How a string is found

A string that a component keeps is a `bridge_text_t` field: an offset into the persistent arena, and a length.
So every place that holds one can be found: in a field, in an array of them, or in a struct that a field holds.

The search reads every element of every field of a value, one at a time.
A field of a struct type is read as a value of that type, with the same steps.
A field of any other kind holds no string, and is passed.
A text holds a string in the share if its bytes overlap one of the ranges of the share.
A text of length zero holds nothing.

Only the rows of a struct type are read: an enum, a bitmask and a tag hold no text.
Every row of the type is read, and each row that holds such a string counts once.

## Functions

| Function | Result |
| --- | --- |
| `is_in_share` | Gives whether the text at `bytes` lies in one of the ranges of a share. |
| `get_element` | Gives where element `i` of a field is in a value. Gives `nullptr` past the last element, or for a kind that holds no string. |
| `points_in` | Gives whether a value holds a string in a share. |
| `count_row` | Counts a row that holds a string in the share. `storage_visit_values` calls it for each value. |
| `module_count_pointing` | Gives how many rows of a type hold a string in a share, given by its ranges. |
