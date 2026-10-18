# src/backends/native/native_func.c

This file keeps the ABI entries a module defines, finds a module's exported functions, and calls them.
`docs/intern/backends/native/native_inside.h.md`, *The invariant*, tells when an entry may run.

## Defining and finding

`define_func` matches the name against `NATIVE_ENTRY_NAMES` and keeps its pointer in the instance.
An unknown name, or another namespace, is refused by name.
`instance_get_func` looks up the symbol `bridge_` and the name, because one process has a flat symbol namespace.
`call` records the calling thread, runs the function, and gives the status it returned.
Nothing catches a fault, so a native module that faults ends the process.

## Functions

| Function | Result |
| --- | --- |
| `define_func` | Keeps the pointer that answers an ABI entry. Refuses an entry the native SDK does not have. |
| `instance_get_func` | Finds an exported function once and remembers it. Refuses when the symbol is missing. |
| `call` | Runs an exported function and gives its status. A call made from inside a call returns to it. |
