# src/host/state/state.c

This file states a whole module into a `bridge_module_config_t`.

`state_found` fills the fixed fields from the manifest, then takes each rule of `MODULE_KEYS`.
It next takes the phases and the settings of the module.
`get_flag` maps a boolean onto `BRIDGE_TRUE` or `BRIDGE_FALSE`.

## Functions

| Function | Result |
| --- | --- |
| `state_found` | Fills a module config. Gives true when every key is taken. |
