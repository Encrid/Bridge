# The log

The log is BridgeC's, not an element's, and it passes the test in `no-builtins.md`, *No builtins*:
anything can fail, so every deployment needs it, and a failure nobody can report is worse than a
missing feature.

An element says things by appending to **one ring in its memory 2**, with a single writer by
construction: an element's systems are serialised - `threads.md`, *Threads* - so no line needs an
atomic. No call either, so a line costs stores rather than the 500ns a host call costs today.
BridgeC drains every ring at each barrier.

    u32 when      barrier count, and offset within the phase
    u8  -         reserved, to keep the stride at 24
    i8  level     negative is worse, flecs' convention
    u16 system    which system was running
    u32 text      offset into this element's scratch arena, always
    u32 len
    u32 arg[2]    two inline scalars, for "something is N"

**This record never leaves BridgeC.** What the host sees is resolved - `host-api.md`, *Nothing here
is a BridgeC layout* - so the shape above is the element seam's alone and its stride is free to
change.

Fixed stride, so the ring is an array: head and tail are counters, record `i` is `base + i*24`, and
the number dropped is a subtraction. Variable-length records would need scanning, which is why
idTech3's journal is a sequential file and not a ring.

**The text is in the scratch arena, and that is not a default but the whole rule.** An offset means
nothing without a memory, and we have three - so one of them is fixed by the binding rather than
carried in the datum. That is the component model's own answer to the same problem: its canonical
ABI takes a `memory` **option**, an index decided where the function is declared, so no value has to
say which address space it came from. idTech3 and flecs never faced it, because `VM_ArgPtr` resolves
against a single `dataBase` (`vm.c:607`) and `ecs_os_api.log_` takes a real pointer.

**So a literal is copied in, and costs a `memcpy` of its length.** Writing it instead as an offset
into memory 0 would be free, and would need a sixteenth seam entry to read a memory BridgeC does not
own - paying surface for zero copies on a path whose own claim is only that a line costs stores.
Godot's alternative is refused too: it copies at the boundary and hands back a handle -
`string_new_with_utf8_chars_and_len2` - which is a call with a payload, the crossing this ring
replaced.

**That is also why the scratch arena exists**: it is the only memory an element can write that
**BridgeC can read**. Memory 0 is the module's own and never adopted; memory 1 is columns.

Two rules follow. **Drain precedes the scratch-arena reset**, because the message lives in that
arena and the barrier does both. And **BridgeC bounds-checks `text + len`** against a determinate
block, which is what makes the check writable at all. One compare per record is the price.

Levels are flecs' signed convention, so flecs' own output needs no mapping - `ecs_os_api.log_` takes
`(level, file, line, msg)`, warn at -2 and err at -3. Filtering is at the writer: the generator
emits `if (level > threshold) return` against a threshold in the memory-2 header, so a line that is
switched off costs a load and a branch.

**Drop and count, never block.** A log may lose lines, and that is exactly what separates it from an
event: an event is an entity, so the entity ceiling governs it and running out is a failure where it
happens rather than a silent drop - `data.md`, *Observers*, and `budgets.md`, *Budgets*.

Where drained records go - a file, a console, a plugin - is the host's business and config's, and
nothing about it crosses to an element.

**Nothing but log lines go through here.** Profile counters are not records: `profile.md`, *The
profile*.
