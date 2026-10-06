# What was refused: Modules on the native backend

The record of `native.md`: `readme.md`, *Three layers, three seams*.

## Lifecycle

Of `native.md`, *Lifecycle*.

Refused: one export, `bridge_bind`, that is handed the table once and keeps it in a variable of the
module's, which is what this was. It left every module a global to declare and the backend a second
way to know who was calling. It had been preferred over an argument to `init` on the ground that
Bridge-ASM700 would then need a native case, which it would not: the call is the backend's. The
condition that brings it back is a module that cannot take an argument.

## What the backend provides

Of `native.md`, *What the backend provides*.

Refused: no argument, and a thread-local variable in the backend to say who is running, which is
what this was. It read like a wasm module's calls, and that was the reason given: "a second ABI for
one backend". It is not an ABI of Bridge-ASM700's at all, only this backend's SDK. The condition
that brings it back is native modules that must be source-compatible with wasm ones.
