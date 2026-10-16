# src/reload_fail.c

This file handles a reload that failed: it takes the new instance back out, and keeps the old instance or stops the module.

## The new instance taken out

A record says that the module was not reloaded, and why. Bridge says it once, and does not try the reload again.
Then the new instance goes back out:

1. Take its systems and observers out of storage.
2. Mark each type that it would have replaced as not replaced. Those rows stay as they are, under the type that they have.
3. Release what the new instance declared, with no policy.
4. Give back what the reload added to the share of the persistent arena.
5. Discard the new record. The ids and the entities are still those of the old record.

The schedule without the systems of the new instance was built before, so the rebuild cannot be refused.

## The old instance kept

If the backend holds both instances, the old instance is kept, and its systems go back into storage as they were.
Their places are free, because the systems of the new instance were dropped.

## The module stopped

If the backend cannot hold both instances, the old instance was shut down before, and there is none to keep.
The module stays, stopped, with everything that it declared: its record, its components and their rows.
A reload is a call that means "keep running", and such a call never discards data.
The module has no code, so it runs nothing, and `bridge_list_modules` shows it stopped.
If it states `fail` in `on_error`, the deck stops too: `docs/intern/bridge_stop.c.md`, *A stopped deck*.
A reload with an artifact that works brings the module back over the same rows, and an unload is what ends it.

## Functions

| Function | Result |
| --- | --- |
| `undo_new` | Takes the new instance back out: its systems, its types, and what the reload added to the share. |
| `keep_old` | Puts the systems of the old instance back in storage, as they were. |
| `stop_old` | Drops the systems of the module and marks it stopped, with what it declared kept. Says so, and stops the deck if the module states `fail`. |
| `reload_fail` | Says that the reload failed, takes the new instance out, then keeps the old instance or stops the module. Gives 1. |
