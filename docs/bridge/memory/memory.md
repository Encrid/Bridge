# Memory

Which block each thing lives in, and who may read it. What it costs when one runs out is
`budgets.md`; where a string goes is `strings.md`; where flecs allocates is `flecs.md`. What the
data *is* is `data.md`.

## Where the seven regions live

Seven things must be addressable with no call, and they do not have the same readers:

    region              memory   who reads it
    component columns      1     every module that declared the component
    entity index           1     every module that follows a reference
    persistent arena       1     every module holding a string in a component
    query spans            2     only the element they belong to
    column_base            2     only the element they belong to
    scratch arena          2     only the element they belong to
    log ring               2     one per element, and BridgeC draining it

Memory 2 is BridgeC's alone, so it is laid out from offset 0 with nothing to negotiate. A fixed
header says where this element's spans, `column_base`, arena and log ring begin, the log level it is
filtering at, and where the entity index begins in memory 1 - the one thing it reads there that it
cannot derive. Every other memory-1 offset is relative to memory 1 itself, including the `(offset,
length)` a component holds into the persistent arena, so nothing else needs a base.

The span region's size is BridgeC's too, and BridgeC resizes it at a barrier, where nothing holds an
offset into it. That is Quake's progs layout - fixed offsets at the bottom, everything else found
through them. So no element has to declare a span capacity it could not compute: span count is
archetype count, and archetype count is a property of the deployment.

What stays exposed is memory 1: the columns, because shared columns are the design, and the index,
because a module must read it with plain loads and wasm has no read-only import.
