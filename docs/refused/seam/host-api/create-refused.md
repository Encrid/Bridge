# What was refused: Create

The record of `create.md`: `README.md`, *Three layers, three seams*.

## Destroy

Of `create.md`, *Destroy*.

Refused: skipping that one `shutdown` and saying so, which always finishes and leaves a device
unreleased by the only code that knew how.

Refused: calling it anyway, which breaks the one promise `pinned` makes at the moment a device is
being torn down. What this costs is a destroy that can fail, so a host that ignores the answer leaks
the bridge; the condition that brings the first refusal back is hosts that cannot get back to the
right thread at exit.

Refused: never closing any backend's library, which is one rule and makes a statement on behalf of
backends that could have made their own. The condition that brings it back is closing found to crash
for a backend that said its library may be closed.

Refused: the wasm backend pinning wasmtime itself, which is operating-system code on three platforms
for the same result. The condition that brings it back is that backend's library being loaded by
something other than Bridge-ASM700.