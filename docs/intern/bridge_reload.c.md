# src/bridge_reload.c

This file reloads a module: a new instance takes the place of the old one, over the storage that stays.
`docs/intern/module.h.md`, *Between phases only*, tells when a reload is legal.

## The steps of a reload

1. Refuse what a load refuses from the config, and what a reload alone is refused for: `src/reload_check.c`.
2. Grow the share of the persistent arena to what the reload states: `src/reload_share.c`.
3. Make the record of the new instance, which takes over what was set aside for the module.
4. Take the systems of the old instance out of storage.
5. If the backend cannot hold two instances, shut the old instance down and destroy it.
6. Make the new image, instance and private memory, and call `reload` of the new instance.
7. Build the schedule again, with the same refusal as a load.
8. Move the rows of each type that the reload made in the place of another: `src/reload_rows.c`.
9. Shut the old instance down, if step 5 did not, and put the new record in its place.
10. Let the deck run again, if the deck waits for this module, and run a barrier.

Step 3 is in `src/reload_record.c`. A failure in steps 6 to 8 goes to `src/reload_fail.c`.
A reload is the `reload` of the new instance, and then the `shutdown` of the old one.
Where the backend holds both instances, the old instance is untouched until step 9.
So a reload that fails before step 9 leaves the module as it ran.
Like a load, `bridge_reload` gives 1 if the deck has stopped, since a module that nothing will ever run did not reload.

## Two instances at once

A process cannot hold two copies of one library.
So on a backend that does not state `BACKEND_SIDE_BY_SIDE`, the old instance goes first: its `shutdown`, then its instance and its image.
Then the new image opens, and its `reload` runs.
A failure after this has no old instance to keep.

## What a reload keeps and changes

A reload has one mode, and it keeps the components: storage stays, which is the point of a reload.
An unload and then a load is the way that loses the components, and its verbs say so.
`reload` is not `init` again: `init` starts from nothing, and `reload` starts over what exists.
The `reload` of the new instance declares and registers as `init` did, and does nothing else.
A component whose layout changed gets its values across: `docs/intern/abi_relay.c.md`, *What each change does*.

What a reload may change is what belongs to the instance.
The new config brings the work limit, the memory limit, the timeouts, the log level and the three sizes of the private memory.
It brings `spawns` and the grants too, and `persistent`, which only grows.
A component that the new instance no longer declares goes as at an unload: `docs/intern/module_types.c.md`, *The policies at unload*.
The settings are not seeded again, since after `init` the settings component is the state.
If the new instance no longer declares the settings component, the record forgets it: its value went by its policy.

The systems of the old instance leave storage before the new instance registers its own, in the places that those had.
So a reload needs no room for the systems of a module twice: `src/abi_alloc.c`.
The new record takes the place of the old one in the list of modules, so the order of the loads stays.
The ids held ready for the old instance are the ids of the new record from the swap on.

## Functions

| Function | Result |
| --- | --- |
| `retire_old` | Takes the systems of the old instance out of storage. Without `BACKEND_SIDE_BY_SIDE`, it also shuts the old instance down and destroys it. |
| `forget_config` | Forgets the settings component if the new instance no longer declares it. |
| `swap` | Puts the new record in the place of the old one, releases what the new instance no longer declares, and discards the old record. |
| `start_new` | Starts the new instance with `reload`, builds the schedule again, and moves the rows. 1, with why in the reload, if one fails. |
| `bridge_reload` | Reloads a module. 0 if the new instance runs and the deck runs, and 1 if not. |
