# src/lib/abi/abi_subscriber.c

This file checks what a system or an observer states, before anything is made of it.

## The checks

1. Refuse one more than `[budget] systems`. A system parked for a reload does not count.
2. Refuse one that asks for more terms than `[budget] terms`.
3. Refuse a name, a term or a change that does not lie in the scratch arena.
4. Refuse a name that is empty, or of 128 bytes or more.
5. Refuse more changes than the module declared types.
6. Refuse a reserved name: `init`, `shutdown` or `reload`.
7. Refuse a name that has `@` or `:`.
8. Refuse a name that the module registered already, as a system or as an observer.
9. Refuse an observer with no term.
10. Refuse a list of `before` outside the scratch arena, or with more slots than the module has earlier systems.

Systems and observers are exports of the module, and an export has one name.
So a system and an observer of one module do not share a name.
A name with no `@` and no `:` gives each system one full name, which a pair of `[order]` can name: `docs/intern/lib/schedule/schedule_order.c.md`.
An observer with no term has no change to subscribe to.
`docs/intern/lib/abi/abi_system.c.md` refuses a registration outside the first call of the module, and an observer with no events or unknown ones.
`docs/intern/lib/abi/abi_subscribe.c.md` refuses a change whose slot the module did not declare.

`get_slots` of `src/lib/abi/abi_slots.h` finds the changes in the scratch arena. A list of no change needs no place there.
`abi_check_before` of `src/lib/abi/abi_before.c` checks the list of `before` last: `docs/intern/lib/abi/abi_before.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `check_budget` | Refuses a system or an observer that is one more than `[budget] systems`, or that asks for more terms than `[budget] terms`. |
| `read_stated` | Finds the name, the terms and the changes in the scratch arena, and copies the name out of it. 1 with a refusal. |
| `is_taken` | Whether the module has a system or an observer of a name. |
| `check_name` | Refuses a reserved name, a name with `@` or `:`, a name that the module registered already, and an observer with no term. |
| `abi_check_subscriber` | Refuses a system or an observer that states what it may not. 1 with a refusal. |
