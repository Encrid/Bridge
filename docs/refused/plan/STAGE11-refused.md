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

Refused: a toggled row switched by a queued change, applied at the barrier and told to
observers. It is two entries at the module seam, in each backend and each SDK, for a change
that costs 18.9 ns where a write costs a store. The condition that brings it back is a module
that must act on a switch at the barrier it happens, and cannot read the bit each time it
runs. Both ways at once is refused with it, and no condition brings that back.

Refused: a second cell for a toggled term's bits, in every span and in a second array beside
`column_base`. Measured no quicker, and two published shapes change for it. The condition that
brings it back is a toggled component reached by id across thousands of tables, measured as
losing by the cell ahead of the column.

Refused: the bits as a term of their own, stated by the author. A reader that forgets the term
is handed no bits and told nothing; and a reach by id needs one of the other two beside it. No
condition brings it back.

Refused: a direct-indexed component that every entity has, zero from its spawn until a system
writes it, with `attach` and `detach` of it refused. It is less to build; and it is a second
kind of component, one no module can be told of and whose given value cannot be told from none.
The condition that brings it back is a component every entity has by definition, and for that a
reader may already leave the bit untested.

Refused: a singleton that exists from `declare`, zeroed, and is only ever written in place; and
two entries of its own to set and to remove one. The first is a second kind of component, for
nothing the chosen way does not allow; the second is two entries in each backend and each SDK
for what `attach` and `detach` already say. No condition brings either back.

Refused: a settings component kept in a table on an entity of Bridge-ASM700's own, as it was,
alone or beside the singleton. A second row of it can exist, and with both there are two paths
for one thing. No condition brings it back.

Refused: a sparse set's index taken from `[budget] columns` when its component is declared,
with storage's own copy allocated then. No key; and storage would allocate when a module loads,
which `storage.md`, *What bounds it*, says it does not, and one budget would pay for rows, for
indexes and for cells. The condition that brings it back is the two keys being stated wrongly
more often than they help.

Refused: reserving for every declared type being a sparse set. Two hundred megabytes at
sixty-four types and four hundred thousand entities, decided by a number nobody stated for
that. No condition brings it back.

Refused: no tag, the smallest component being one byte. Nothing to build; and measured at 200
KB of columns for each marker 204,800 entities carry. The condition that brings it back is the
case of no size costing more to keep right in five storage layouts than the bytes it saves,
shown by defects found in it.

Refused: a row that is off losing the component when it leaves a toggle; and no move out of a
toggle while a row is off. The first drops a value the module's `reload` could have dropped
itself; the second can be met only by code in the build that is being replaced. No condition
brings either back.

Refused: one key that says yes or no to whatever a reload would lose. It lets through the
rename the refusal exists to catch. No condition brings it back.

