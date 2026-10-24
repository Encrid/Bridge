# src/storage/storage_settle.h

This header gives the release, the last step of `docs/intern/storage/storage_apply.c.md`, *The steps of a barrier*.
It empties the queue of each module, forgets the fold, and tells each module which of its changes storage did not apply.

## What a module hears

`say_unapplied` writes an error record for each kind of shortage: for want of a table, and for want of columns.
The record names the module, the budget and its value, and how many of its changes storage did not apply.
`say_barred` writes an error record for the destroys that storage barred.
It says how many entities the module asked to destroy that carry nothing that it may write.

`say_unhanded` writes an error record for the attaches and detaches of a held id, and says how many storage left undone.
Each record sets `fault_now` of the module to the words of its fault, so the barrier counts against its budget: `docs/intern/schedule/schedule.h.md`, *The failure budget*.
`empty_queues` sets `changed_now` of each module that queued something, for the same budget.

## Functions

| Function | Result |
| --- | --- |
| `say_unapplied` | Tells a module how many of its changes storage did not apply, for want of a table and for want of columns. Then sets the counts to 0. |
| `say_barred` | Tells a module how many entities it asked to destroy stay as they were. Then sets the count to 0. |
| `say_unhanded` | Tells a module how many attaches or detaches of a held id storage left undone. Then sets the count to 0. |
| `empty_queues` | Empties the queue of each module and its marks, and records whether the module queued something. |
| `settle_all` | Ends the work of a barrier on rows: empties the queues, forgets the fold, and tells each module what storage did not apply. |
