# src/lib/say/say.c

This file hands the records of the library's own to the sink of the host.

## The sink

The library never writes a file.
It hands each record to the sink, and the host writes the record.
The sink is necessary: a library that can fail with no way to say why is worse than a library that refuses to start.
So `bridge_create` refuses a null sink, before anything is loaded.

Four properties follow from where the records are made:

- One call for each record. The call goes from native code to native code, so a batch saves no crossing.
- On the calling thread. The drain is at a barrier, in `bridge_run_phase`, after the parallel systems joined. So the sink needs no lock.
- The text is good for this call only. The scratch arena is reset after the drain, so a host that keeps a line copies it.
- The identity can be null. A record of the library's own, for example a refused load, can be about no module.

There is no call that gives the last error.
A failed call gives its status, `bridge_list_modules` tells what runs, and a record tells why.

## A record of the library's own

The text is written as `printf` writes it, into 512 bytes, so a longer text is cut.
The identity is the identity of the module, or null.
The system is the name of `s` of `say_t`, or null. `s` is a system or an observer.
`barriers` holds the count of barriers, and `place` is 0.
The level is the level that `say_t` states, and `say_error` states `BRIDGE_ERR`.

## Functions

| Function | Result |
| --- | --- |
| `send_record` | Hands the sink one record: the text, whose it is about, and the barrier at which it was said. |
| `say_error` | Hands the sink an error of the library's own, about no module, written as `printf` writes. |
| `say_about` | Hands the sink a record about a module or one of its systems, written as `printf` writes. |
| `copy_text` | Gives a copy of the first `n` bytes of a text, with a zero after them. Null if there was not the memory. |
| `bridge_lacks` | Gives 1, with the record of `docs/extern/lib/bridge.h.md`, *Out of memory*, if `got` is null. Gives 0 if not. |
