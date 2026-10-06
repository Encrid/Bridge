# The log

What was weighed here and refused is `log-refused.md`, *What was refused: The log*.

The log is Bridge-ASM700's, not a module's, and it passes the test in `no-builtins.md`, *No builtins*:
anything can fail, so every deployment needs it, and a failure nobody can report is worse than a
missing feature.

A module says things by appending to **one ring in its private memory**, with a single writer by
construction: a module's systems are serialised - `threads.md`, *Threads* - so no line needs an
atomic. No call either, so a line costs stores rather than the 500 ns a call out of a module cost
in the original Bridge. Bridge-ASM700 drains every ring at each barrier.

    u32 when      barrier count, and offset within the phase
    u8  -         padding: zero, and not read
    u8  level     a severity: lower is worse
    u16 system    which system was running
    u32 text      offset into this module's scratch arena, always
    u32 len
    u32 arg[2]    two inline numbers: a value, and what it is set beside

**This record never leaves Bridge-ASM700.** What the host sees is resolved - `host-api.md`, *Nothing
here is an ABI layout* - so the shape above is the module seam's alone and its stride is free to
change.

**Two numbers, and no more.** One lets a line say "something is N" with no text formatted in the
module; the second lets it say what N is set beside - what was wanted, which one, out of how many -
which is what most lines in the tests do.

Fixed stride, so the ring is an array: head and tail are counters, record `i` is `base + i*24`, and
the number dropped is a subtraction. Variable-length records would need scanning, which is why
idTech3's journal is a sequential file and not a ring.

**The text is in the scratch arena, and that is not a default but the whole rule.** An offset means
nothing without a memory, and we have three - so one of them is fixed by the SDK rather than
carried in the datum. That is the component model's own answer to the same problem: its canonical
ABI takes a `memory` **option**, an index decided where the function is declared, so no value has to
say which address space it came from. idTech3 and flecs never faced it, because `VM_ArgPtr` resolves
against a single `dataBase` (`vm.c:607`) and `ecs_os_api.log_` takes a real pointer.

**So a literal is copied in, and costs a `memcpy` of its length.** Writing it instead as an offset
into its own memory would be free, and would need one more seam entry to read a memory
Bridge-ASM700 does not own - paying surface for zero copies on a path whose own claim is only that a
line costs stores. Godot's alternative is refused too: it copies at the boundary and hands back a
handle - `string_new_with_utf8_chars_and_len2` - which is a call with a payload, the crossing this
ring replaced.

**That is also why the scratch arena exists**: it is the only memory a module can write that
**Bridge-ASM700 can read**. A wasm module's own memory is never adopted; the shared memory is columns.

Two rules follow. **Drain precedes the scratch-arena reset**, because the message lives in that
arena and the barrier does both. And **Bridge-ASM700 bounds-checks `text + len`** against a determinate
block, which is what makes the check writable at all. One compare per record is the price.

**A level is a severity of syslog, RFC 5424, by its number**: 0 is the worst and 7 the most
detail. Four are named, in `bridge.h` for a host and in `bridge-abi.h` for a module:

    3  err      4  warn      6  info      7  debug

The four that are named have two sets of names, `BRIDGE_LOG_ERR` and its like in `bridge-abi.h` for
a module that writes a line, and `BRIDGE_ERR` and its like in `bridge.h` for a host that reads one:
the same numbers, in two headers that are each whole without the other, held equal by an assertion
where a module's level becomes a host's. A module may write any of the eight. A module's `log_level`
is any of them but the first: zero is unset for it, as for every key, and refused; so is a number
past 7. Nothing is lost that anyone states: a line of severity 0 passes every threshold from 1 up,
and the least severe of the named ones is 3. `tests/stage2.c`, case `unsetlevel`. Filtering is at
the writer, which begins `if (level > threshold) return` against a threshold in the private-memory
header, so a line that is switched off costs a load and a branch.

**Drop and count, never block.** A log may lose lines, and that is exactly what separates it from a
signal: a signal is an entity, so the entity ceiling governs it and running out is a failure where it
happens rather than a silent drop - `data.md`, *Observers*, and `budgets.md`, *Budgets*.

Where drained records go - a file, a console, another process - is the host's business, and
nothing about it crosses to a module.

**Nothing but log lines go through here.** Profile counters are not records: `profile.md`, *The
profile*.
