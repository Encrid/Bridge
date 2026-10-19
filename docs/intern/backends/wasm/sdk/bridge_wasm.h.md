# src/backends/wasm/sdk/bridge_wasm.h

A wasm module compiles this header, with `bridge_abi.h`, to reach Bridge and read its own data.
It pulls in the accessors, the scratch arena, and the readers for spans, entities, kept storage, toggles, observers and the log.
It adds the macros a module registers with and reads its rows through.

## The macros

`BRIDGE_COMPONENT` and `BRIDGE_DECLARE` register a component by its type and descriptor; `BRIDGE_DECLARE_TAG` registers a tag that carries no bytes.
`BRIDGE_ADD_SYSTEM` and `BRIDGE_OBSERVER` register a system and an observer, each with its terms, its changed slots, and the systems it runs before.
`BRIDGE_ATTACH` lends a value, then attaches it to a slot.
`BRIDGE_ROW` finds a row's address; `BRIDGE_GET` and `BRIDGE_SET` load and store a field, and pick the width from the field's type.
`docs/intern/backends/wasm/sdk/bridge_wasm_acc.h.md` describes the accessors these macros build on.
