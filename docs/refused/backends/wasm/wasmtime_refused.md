# What was refused: The wasm backend

The record of `wasmtime.md`: `readme.md`, *Three layers, three seams*.

## Deadlines

Of `wasmtime.md`, *Deadlines*.

Refused: a thread incrementing every tick period, which is wasmtime's usual pattern. It needs a
period somebody must choose, wakes forever with nothing armed, and rounds every timeout to a tick.
The condition that brings it back is several deadlines running at once.
