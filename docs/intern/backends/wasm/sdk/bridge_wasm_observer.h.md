# src/backends/wasm/sdk/bridge_wasm_observer.h

This header reads an observer's triggers from the observer list in the private header.
A trigger records the entity that fired it and the events that matched.

## Functions

| Function | Result |
| --- | --- |
| `bridge_count_triggers` | Gives how many triggers an observer has. |
| `bridge_get_trigger` | Gives the address of trigger `i` of an observer. |
| `bridge_get_trigger_id` | Gives the entity id a trigger records. |
| `bridge_get_trigger_events` | Gives the events a trigger matched. |
