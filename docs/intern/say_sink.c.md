# src/say_sink.c

This file is the one place that calls the sink of the host inside a world.

## The flag of the sink

`say_send` sets `in_sink` of the world, calls the sink, and clears `in_sink` when the sink returns.
Each record of Bridge and each line of a log goes through it: `src/say.c` and `src/block_log.c`.
While the flag is set, each call of a function that changes the world is refused: `docs/intern/bridge_check.c.md`, *A call from the sink*.
`bridge_create` calls the sink by itself when it cannot make a world. There is no world then, so the host cannot call one.

## Functions

| Function | Result |
| --- | --- |
| `say_send` | Hands the sink one record, with `in_sink` of the world set during the call. |
