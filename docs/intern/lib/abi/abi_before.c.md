# src/lib/abi/abi_before.c

This file checks and reads `before` of a system or of an observer: the earlier systems of its module that run before it.

## The list

`abi_check_before` runs last among the checks of `docs/intern/lib/abi/abi_subscriber.c.md`, before Bridge makes anything.
It refuses a list that is outside the scratch arena.
It refuses a list with more slots than there are earlier systems of the module, or earlier observers.
Thus the cost of the slots of one registration has a limit, as the changes and the terms have.
A slot can come two times, and then the second time changes nothing.

## The slots

A slot of `before` names an earlier system of the module: `docs/extern/bridge_abi.h.md`, *add_system*.
An observer has a slot among the observers, so `before` of an observer names observers: `docs/intern/lib/abi/abi_subscribe.c.md`, *The slot*.
`read_before_slot` refuses a slot that is not below the slot of the new system: no system has it yet, or it is the new one.
Then it refuses a system of another phase, since the phases have no order: `docs/intern/lib/bridge/bridge_phases.c.md`.
All observers run at the barrier, so this test passes for each observer.

## The bits

For each slot, `schedule_join_before` gives the new system the bit of that system, and all of its bits.
Thus the bits of a system hold each system that its module states before it, also through a chain.
`docs/intern/lib/schedule/schedule_before.c.md` tells how the schedule reads them.

## Functions

| Function | Result |
| --- | --- |
| `abi_check_before` | Finds the list of `before`, and refuses it outside the scratch arena or with too many slots. 1 with a refusal. |
| `read_before_slot` | Refuses `slot`, item `i` of `before`, if it is not an earlier slot of the same phase. Else joins its bits. 1 with a refusal. |
| `abi_read_before` | Reads each slot of `before`. 1 with a refusal. |
