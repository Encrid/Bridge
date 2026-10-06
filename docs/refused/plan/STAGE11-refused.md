# What was refused: Stage 11

What was weighed in `STAGE11.md` and refused, each with the condition that would bring it back.

## The questions

Of `STAGE11.md`, *The questions*.

Refused: a component's storage layout stated in the manifest. It is the same statement moved
away from the code that depends on it, with nothing to hold the two together. No condition
brings it back.

Refused: stated by the deployment, alone or over the author. The deployment does know how many
entities carry a component; and the module's reading code is compiled for one storage layout, so
another at run time means code for all five in every module, or a read through a call. The
condition that brings it back is reads that go through a call, which `query.md`, *The interface
is a query*, refuses.

Refused: storage working out, at each barrier, the rows that match a query over two storage
layouts, and publishing them as a list. The answer is then exactly the entities that match; and
it is work at every barrier for every change of such a component, in every query that names it,
and a list a query sized for the most there may be. The condition that brings it back is a
deployment where the test at each row of such a query is a measured share of a tick, on a
component that changes seldom.

Refused: an archetype that counts the components kept in sparse sets, with a list of entities
for each, as Bevy has. It brings back a list for every combination of components, which is what
the sparse set was passed to end. No condition brings it back.

Refused: a sparse set's index holding the value's place in the array, with a marker for none
and the array's address published beside it. The same time and the same memory, for one more
thing published and a second way to say none. No condition brings it back.

Refused: the index in pages, made where an entity that carries the component is. Measured at
one and a half times the lookup; ids are handed out from the low end, so the pages touched are
most of them; and a pool of pages is a budget nobody states today. The condition that brings it
back is a deployment where eight bytes for every entity, for every such component, is a measured
share of its memory.

