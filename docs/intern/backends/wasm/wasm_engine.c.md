# src/backends/wasm/wasm_engine.c

This file holds the watchdog thread that interrupts a call which runs past its timeout.
The thread sleeps until a timed call arms it, then wakes at the call's deadline.

## The watchdog

`read_armed` copies the armed call under the lock: its number, its deadline, its engine, and the quit flag.
With no armed call the thread waits on the wake event with no deadline.
With an armed call `wait_until` sleeps until the deadline, or until the wake event fires first.
When the deadline passes the thread increments the engine's epoch, which traps the running call.
It remembers the interrupted call, so it does not interrupt the same call twice.

## Functions

| Function | Result |
| --- | --- |
| `wasm_watch` | Runs the watchdog loop until the backend quits. |
