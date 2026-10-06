# What was refused: Lifetime

The record of `lifetime.md`: `readme.md`, *Three layers, three seams*.

## What unload leaves behind

Of `lifetime.md`, *What unload leaves behind*.

Refused: leaving a second table without saying so, which is what this did: a deployment saw more
tables than archetypes and no line told it why. No condition brings it back.

Refused: destroying what cannot be placed, which is what this first did. `strip` says the entity
stays, and a reload that stops declaring a component is a strip too, on a call that means "keep
running". No condition brings it back: `strip` says the entity stays.

Refused: never joining, and leaving every stripped table where it stands, which is one path and no
row moved: it doubles the tables and spans of every set that already had one, for as long as their
entities live. The condition that brings it back is the join costing more than the second table
does.

Refused: moving every block of the column region so that a join always fits. The condition that
brings that back is second tables seen to pile up in a deployment.

Refused: one rule for every component, which is right for an event and wrong for a handle, or the
reverse. The condition that brings it back is modules seen to state one policy for everything they
declare.

Refused: destroying what a module spawned, which needs an owner per entity. The condition that
brings it back is an owner for each entity existing for some other reason.

Refused: a fourth value that makes the unload itself fail while entities still carry the component -
a new failure path nothing has asked for. The condition that brings it back is data that must not be
lost by an unload somebody else requested.

## When a component is moved

Of `lifetime.md`, *When a component is moved*. What becomes of a row that is off, and what else
was weighed, is in the record of `stage11.md`, *The questions*.

Refused: moving a component where it stands, a table at a time, so that it never needs room for
both. It is less memory for the length of a reload; and a move that fails half way has moved
half, where this one has moved nothing. The condition that brings it back is a deployment that
cannot give a reload the room, measured.

## When a component layout changed

Of `lifetime.md`, *When a component layout changed*.

Refused: one call that reloads several modules as one and moves the rows once all of them present
the new struct. It keeps every declarer's own state, at the price of a new call at the host seam
that swaps several modules together and puts them all back if one fails. The condition that brings
it back is a deployment that must change a shared struct and keep what each declarer holds. This
paragraph called the refusal "forced rather than chosen", and it was neither weighed nor given a
condition.

Refused: moving the rows at once and saying what went, which is what was first decided here. It
fails nobody who removed a field, and for a rename it loses every value of the field on a call that
means "keep running". The condition that brings it back is a rename that can be told from a removal.

Refused: refusing every such change, so that a struct which loses a field is had only by unloading,
which loses every row. The condition that brings it back is moving rows proving unsound.

This record refused "a flag to waive the refusal, which is a flag whose job is disabling a check".
What a reload states is not that: the check runs either way and names what would go, and the
statement says the host has read it. So that refusal is withdrawn, with the one beside it that
refused every reload losing a byte, which the two above replace.

Refused: a way for a module to state "this field was that one", which is one more thing in the ABI
for a case two reloads or a new name already cover. The condition is renames being common enough
that losing their values is a daily cost.
