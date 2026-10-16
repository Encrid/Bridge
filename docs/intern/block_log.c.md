# src/block_log.c

This file drains the log ring of a module to the sink of the host.

## The log ring

The log is the library's, and not a module's: each module can fail, so each deployment needs the log.
A module writes a line as a record in one ring in its private memory.
The systems of a module run one after another, so the ring has one writer, and no line needs an atomic.
A line is no call: it costs stores.
The record is a `bridge_log_record_t` of 32 bytes, and the ring is an array of them.
The head and the tail are counters, and the number of lines dropped is a subtraction.

The text of a record is always an offset into the scratch arena of the module.
An offset means nothing without a memory, and the scratch arena is the one memory that a module writes and the library can read.
So the drain comes before the reset of the scratch arena, which both happen at the barrier.

The header of the private memory holds the `log_level` of the module.
The module does not write a line whose level is above it, so a line that is off costs a load and a branch.
A ring that is full drops lines: it never blocks, and the drain tells how many it dropped.
Where the records go after the sink is the host's business.

The record of the ring never leaves the library.
The sink gets a `bridge_record_t` with resolved values: the identity of the module, the name of the system, and a pointer to the text.

## The levels

A level is a severity of syslog, RFC 5424, by its number: 0 is the worst and 7 is the most detail.
A module names four of them in `include/bridge_abi.h`, as `BRIDGE_LOG_ERR` and the others.
A host names the same four in `include/bridge.h`, as `BRIDGE_ERR` and the others.
Each header is whole without the other.
This file holds the two sets of names equal with static assertions, where the level of a module becomes the level of a host.

## The drain

1. Read `ring_head` in the header: the count of the records that the module ever wrote.
2. Hand the sink each record still in the ring, the oldest first. The ring holds `ring_len` records at most.
3. If the module wrote more than the ring holds, write a warning with how many lines were dropped.
4. Set the tail to the head, in the record of the module and in the header.
5. Set `scratch_used` to 0: the scratch arena is the module's again.

## A line handed to the sink

A module can write all of its private memory.
So the drain copies a record out of the ring before it reads it, and checks that the text lies in the scratch arena.
A text outside the scratch arena reaches the sink as "(a log line whose text lies outside the scratch arena)".
The system is the system of the slot in the record, or the observer of the slot if `observer` is not 0.
A slot with no system or observer gives null.
`barriers`, `place`, `arg` and `level` go to the sink as they are.

## Functions

| Function | Result |
| --- | --- |
| `get_writer` | Gives the name of the system or the observer that wrote a record, by the slot in the record. Null if the module has none there. |
| `is_in_scratch` | True if the text of a record lies in the scratch arena of the module. |
| `send_line` | Hands the sink the record at `at` of the ring of a module. |
| `block_drain` | Hands the log ring of a module to the sink, tells how many lines were dropped, and gives the module its scratch arena back. |
