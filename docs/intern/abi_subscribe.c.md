# src/abi_subscribe.c

This file registers a system or an observer: it reads what the module stated, hands the terms to storage, and lists the system.

## The steps

1. Check what the system states, and copy its name: `docs/intern/abi_subscriber.c.md`.
2. Have all the memory that it needs: `docs/intern/abi_subscriber_room.c.md`.
3. Read its changes.
4. Read its terms, one by one: `docs/intern/abi_term.c.md`.
5. If a change or a term is refused, free what was had, and give 1.
6. Hand the terms to storage, and list the system among those of the deck and of its module.
7. Give its slot.

## The changes

The changes of a system are what it attaches or detaches: the slots of those components, each a `u32`.
They are a list of their own, and not a kind of term.
A term is one component that a query asks for, and a system attaches a component to entities that do not carry it.
A system that attaches or detaches nothing states no changes.
An attach or a detach of what the running system did not state is refused: `docs/intern/abi_entry.h.md`, *What a call may change*.

A change is a write, so it needs the grant that a write needs.
A change that names a slot that the module never declared is refused.
If the call in progress was refused before, the registration stops at its first change.
`destroy` is stated nowhere, and `init`, `shutdown` and `reload` state nothing: a lifecycle call runs alone.

## The terms

Each term is copied out of the scratch arena, since the module can put it at any offset.
Storage is asked for the type of the term and its operator.
The conflict graph reads which component the term touches and whether it writes it.
A term that must not match touches nothing.

An observer goes to storage with a listener: its events, `abi_hear`, and the observer.
A system with terms goes to storage as a system.
A system with no term matches nothing, and storage holds nothing of it: it runs once in each pass of its phase.

## The slot

The slot is the place of the system among the systems of its module, or of the observer among the observers.
Systems and observers have slots of their own.
The system is also put among the systems of the deck, in the order of registration.

## Functions

| Function | Result |
| --- | --- |
| `read_changes` | Reads what a system or an observer attaches or detaches. 1 with a refusal. |
| `read_term` | Reads term `i`: what storage is asked, and what the conflict graph reads of it. 1 with a refusal. |
| `hand_over` | Hands the terms of a system or an observer to storage, and fills in the system. |
| `list_system` | Puts a system among the systems of the deck, and among the systems or the observers of its module. Gives its slot there. |
| `abi_subscribe` | Registers a system or an observer, and gives its slot in `r`. 1 with a refusal. |
