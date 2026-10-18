# src/backends/native/native_inside.h

This private header is shared by the files of the native backend.
It numbers the eight ABI entries, declares the backend structs, and holds `native_enter`.
`docs/intern/lib/backend/backend.h.md`, *The entries of a backend*, lists what Bridge asks a backend for.

## The entries and the structs

`enum native_entry` numbers the eight entries, and `NATIVE_ENTRY_NAMES` gives their names in that order.
The structs hold the backend, an adopted block, an opened module, a kept function, and an instance.
An instance also records the thread a call runs on, and whether an entry has refused during that call.

## The invariant

`native_enter` reaches one ABI entry on behalf of a module.
It finds the instance from the table pointer, because the table is a member of the instance.
It answers zero when no call is running, when an entry has already refused, or when the entry is unset.
It answers zero when the caller runs on another thread, so a foreign callback reaches nothing.
Once an entry refuses, every later entry answers zero until the call returns, and Bridge then fails the invocation.

## Functions

| Function | Result |
| --- | --- |
| `native_enter` | Reaches an ABI entry, after the checks above. Gives the entry's result, or zero. |
| `native_fill_api` | Declared here, given by `docs/intern/backends/native/native_load.c.md`. |
| `native_spawn` and the phase entries | Declared here, given by `docs/intern/backends/native/native_phase.c.md`. |
