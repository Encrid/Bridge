# src/say_sink.c

This file is the one place that calls the sink of the host inside a world.

## The call of the sink

`say_send` calls the sink. Each record of Bridge and each line of a log goes through it: `src/say.c` and `src/block_log.c`.
The call of the world that runs owns the world, so a call that the sink makes is refused: `docs/intern/bridge_check.c.md`, *A call from the sink*.
`bridge_create` calls the sink by itself when it cannot make a world. There is no world then, so the host cannot call one.

## Functions

| Function | Result |
| --- | --- |
| `say_send` | Hands the sink one record. |
