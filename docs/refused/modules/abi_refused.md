# What was refused: The ABI

The record of `abi.md`: `README.md`, *Three layers, three seams*.

## The ABI

Refused: a pointer type the backend translates, which would have the backend reading a module's own
memory and pairing each pointer with its length. The condition that brings it back is a value too
large for a scratch arena.

Refused: a real pointer on the native backend alone, which is one copy fewer there and two meanings
of an entry.

Refused: `attach` answering a place to write, with the queue in the module's private memory, which
is one copy fewer everywhere and the queue, its checks and its budget made again. The condition that
brings either back is the second copy of a large value measured as a cost that matters.

Refused: the observer's whole query as spans, which is what a system gets and cannot say which
entity changed - an event would be found by scanning every event. The condition that brings it back
is an observer that wants the shape and not the change, and such a thing is a system.

Refused: a fourth operator on a term, meaning "not matched, and changed by this system", which takes
no new argument. It gives "term" two meanings, and each such term would hold four unused bytes in
every span. The condition that brings it back is entries that can take no more arguments.

Refused: a flag at `declare`, for the whole module. Every system of the module would then conflict
over the component, and pairs be refused that change nothing. The condition that brings it back is
modules of one system each.
