# What was refused: The schedule

The record of `schedule.md`: `README.md`, *Three layers, three seams*.

## A module's phase names are mapped by the deployment

Of `schedule.md`, *A module's phase names are mapped by the deployment*.

Refused: nothing, which is what this had, so that a module fitted only a deployment that spoke its
words. The condition that brings it back is none.

Refused: names shipped by the library, for the reason this section opens with. Whether deployments
come to share a list of names is still stage 17's question; the table is what makes the answer not
matter to whether a module can be used. The condition that brings it back is `PLAN.md`, stage 17,
finding deployments that share one list.

## Order within a phase is never implicit

Of `schedule.md`, *Order within a phase is never implicit*.

Refused: leaving out an entity the module spawned since the last barrier, which no other module can
reach. It costs four bytes an entity to know who spawned what, and makes the rule two. The condition
that brings it back is deployments ordering emitters that have no reason to be ordered, often enough
to be named a burden.

Refused: the order the modules were loaded in, which is what a barrier used. It contradicted the
paragraph above, and made what one module left on an entity depend on which the host loaded first.
No condition brings it back.

Refused: finding the collision at the barrier, by noting who changed each component of each entity.
It costs every barrier a note for every change, and is found only when the data collides. The
condition that brings it back is stating proving too coarse.

Refused: another fixed order, by a module's name, which is as arbitrary as the first. No condition
brings it back.

Refused: namespace and system alone, which reads like a component's name and forbids two modules of
one namespace from sharing a system name. The condition that brings it back is namespaces proving to
be one module each.

Refused: refusing those pairs as well, which would have a deployment ordering the inside of a module
it did not write. The condition that brings it back is a module whose registration order turns out
to be an accident.

## What a pass depends on

Of `schedule.md`, *What a pass depends on*.

Refused: two calls at the host seam, to be handed what native modules wrote and to supply it in
their place, which is what this said for a day. A host has no such call and needs none. The
condition that brings one in is a recorder built as a module finding something a module cannot
reach.
