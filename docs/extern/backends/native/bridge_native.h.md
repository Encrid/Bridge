# include/backends/native/bridge_native.h

A native module compiles this header, beside `bridge_abi.h`, to reach Bridge through the backend's table.
It pulls in the table, the kept-storage readers and the log, and adds the registration macros.

## The macros

`BRIDGE_NATIVE_COMPONENT` and `BRIDGE_NATIVE_DECLARE` register a component from a type and its descriptor.
`BRIDGE_NATIVE_DECLARE_TAG` registers a tag, a component that carries no bytes.
`BRIDGE_NATIVE_ADD_SYSTEM` and `BRIDGE_NATIVE_OBSERVER` register a system and an observer, with their terms and changes.
`BRIDGE_NATIVE_PUT_CHANGES` stages a subscriber's changed slots in the arena.

Each macro stages its statement in the arena, then reaches the matching entry.
`BRIDGE_NATIVE_ATTACH` lends a value and attaches it by slot.
`docs/extern/backends/native/bridge_native_api.h.md` describes the arena.

## Functions

| Function | Result |
| --- | --- |
| `bridge_native_get_query` | Gives a query's spans, as the last barrier left them. |
| `bridge_native_get_span` | Gives span `i` of a query, from the private base. |
