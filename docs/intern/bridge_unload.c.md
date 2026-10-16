# src/bridge_unload.c

This file unloads a module.
`docs/intern/module.h.md`, *Between phases only*, tells when an unload is legal.

## The steps of an unload

1. Refuse the call if the deck has stopped for another module, or a module is pinned to another thread.
2. Refuse an identity that no loaded module has, by the name that was asked for.
3. If this module stopped the deck, let the deck run again without it.
4. Take its systems and observers out of the schedule, before its last call.
5. Call its `shutdown` under its timeout, apply what it queued, and drain its log ring.
6. Release each type that it declared. A component that no other module declares goes by its policy.
7. Destroy the ids held ready for it, since nobody will ever spawn them.
8. Give its share of the persistent arena back, unless a row that stays holds a string in it.
9. Take it out of the list of modules, and discard its record, its instance and its image.
10. Build the schedule again, and run a barrier.

Step 5 applies what the module asked and was not given yet, what `shutdown` asked included.
Step 6 follows `docs/intern/module_types.c.md`, *The policies at unload*.
Step 8 follows `docs/intern/module_share.c.md`, *When a share goes back*.
In step 9, to discard the image of a module of the native backend closes its library.
After the barrier, what left is out of the spans of each module, and the observers hear of it.

## Nothing to chase

No module holds a reference to the module that leaves: a module has a query, and calls no other module.
A query that now matches nothing walks no span.
So an unload has no reference to look for and clear.

## Functions

| Function | Result |
| --- | --- |
| `find_loaded` | Gives the loaded module whose identity is `identity`, or null. A null identity matches no module. |
| `take_out` | Does the rest of an unload, after the last code of the module ran: its types, its ids, its share and its record. |
| `bridge_unload` | Unloads a module. 0 if it is gone, and 1, with a record, if refused. |
