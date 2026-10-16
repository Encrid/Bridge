# src/module_share.c

This file gives the share of a module of the persistent arena back to the arena.
At an unload, it gives the share back only if no row that stays holds a string in it.

## When a share goes back

While a module is loaded, nothing frees what it allocated: there is no mark and no reset.
A string lives in a component that another module can read under a grant.
So a reset would make offsets stale that modules hold without having asked for the reset.
The discipline is allocation for a lifetime, with no record for each object, and the lifetime is that of the module.

At the unload, the share goes back to the arena, unless a row that stays holds a string in it.
Bridge can know what the module cannot.
After the components of the module are dropped, Bridge reads each `bridge_text_t` of each row that is left: `docs/intern/module_pointed.c.md`.
If none lies in the share, the share is free for the next load.
If one does, the whole share stays, and so does the string, which is still readable like any value beside it.

So no offset that a component holds ever becomes stale.
What a component keeps stays good for as long as the row of the component does.
An offset kept anywhere else is good only while the module that allocated it is loaded.
Examples are an offset in the memory of a module, or in a field that is two integers and not a `bridge_text_t`.

## A share that stays

A share that stays is said, as a warning.
There is one record for each component that holds a string in it: the module, the bytes that stay, the component, and how many rows.
A later load can be refused for want of that room, and the log then gives the reason.

An unload reads each text field of each row that stays, once, and never at a barrier.
A share with one string left in it stays whole: the record is there to show it.
In exchange, modules can be loaded and unloaded for as long as a deployment runs.

When the share stays, the world keeps its ranges, and the place of the settings text for the next load.
After each unload, each kept share that no row holds a string in goes back: `docs/intern/config_kept.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `module_round_share` | Gives the share that a `persistent` takes: the bytes rounded up to a multiple of 8, in 64 bits, or 0 for none. |
| `module_give_share` | Gives each range of a share back to the persistent arena, and lowers the count of ranges that the world holds. |
| `get_share_size` | Gives the length of the share of a module, in bytes: the sum of its ranges. |
| `is_share_held` | Gives whether a row that stays holds a string in the share of a module, and says each component that does. |
| `module_settle_share` | Gives the share of a module back at its unload, unless a row that stays holds a string in it. Then gives back each kept share that is free. |
