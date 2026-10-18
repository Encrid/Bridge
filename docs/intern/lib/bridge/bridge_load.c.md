# src/lib/bridge/bridge_load.c

This file loads a module.
`docs/intern/lib/module/module.h.md`, *Between phases only*, tells when a load is legal.

## The steps of a load

1. Refuse what `bridge_check_call` refuses: `docs/intern/lib/bridge/bridge_check.c.md`, *A call that runs the code of a module*.
2. Refuse what the config alone decides, find the backend that it names, and work out its demands.
3. Refuse a module loaded already in any version, a load past `[budget] modules`, or a share that does not fit.
4. Make the record of the module: all of it, or none.
5. Take the share of the persistent arena, if `persistent` is more than zero.
6. Set aside the ids of the module before `init` runs, because `init` can spawn.
7. Start the module, with `init` as its first call: `src/lib/module/module_start.c`.
8. Build the schedule again. It refuses a cycle, and a new system or observer that conflicts with another and that nothing orders.
9. Put the module at the end of the list of modules.
10. Let the world run again, if the world waits for this module.
11. Run a barrier. It applies what `init` queued, and publishes the spans.

Steps 2 and 3 are in `src/lib/module/module_check.c`.
Before the module has a record, a refusal says that the module is refused, and `on_error` does not apply.
After it has a record, a failure is the failure of the module, and its `on_error` tells what it costs.
`docs/intern/lib/module/module_say.c.md`, *Refused or not loaded*, tells the two.
The list of modules gives the order in which a barrier applies their changes: `docs/extern/bridge.h.md`, *bridge_load*.
A module that is loaded and that nothing will ever run is not a load that succeeded: `bridge_load` gives 1 if the world has stopped.

## The share of the arena

The share is the own part of the module of the persistent arena, for as long as it is loaded.
No other module allocates from it.
Its length is what `module_round_share` gives for `persistent`: `docs/intern/lib/module/module_share.c.md`, *Functions*.
The list of gaps of the arena gets room for one more range, so that to give the share back never asks for memory.
Step 3 refused a share past the largest gap and the tail, so `region_take` always finds room for it.
If there is no memory to keep track of the share, a record says so, and the record of the module is discarded.

## A load that fails

A failure after the record leaves storage as it was before the load:

1. Say that the module did not load. If its `on_error` is `fail`, stop the world.
2. Take its systems and observers out first, because a query must not outlive a component that it names.
3. Release the ids set aside for it.
4. Give the share back to the persistent arena.
5. Release each component that no other module declares, with no policy.
6. Discard the record, with the queue of what the module asked.
7. Build the schedule again.

No policy applies in step 5, because no entity can carry a change that was never applied.
The schedule without the systems of the module is one that was built before.
So the rebuild cannot be refused, and it asks for no memory.

## Functions

| Function | Result |
| --- | --- |
| `keep_share` | Records a share of `share` bytes at `at` as the one range of the module, with all of it left. |
| `take_share` | Takes the share of the module of the persistent arena. 1, with a record, and the module discarded, if there is no memory to keep track of it. |
| `fail_load` | Gives the lines that the module wrote, and says that the load failed. Stops the world if the module states `fail`, and leaves storage as it was. Gives 1. |
| `start_loaded` | Sets the ids aside, runs `init`, builds the schedule again, and then writes the settings. 1, with why in `err`, if one fails. |
| `bridge_load_now` | Loads a module. 0 if it loaded and the world runs, and 1 if not. |
