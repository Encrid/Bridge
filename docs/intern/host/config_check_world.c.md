# src/host/config_check_world.c

This file checks the world keys, the budget, the backends, and the content keys.

`config_check_world` checks `phases`, `threads`, `rounds` and `presets`, each at depth one.
`config_check_budget` checks the ten budget counts and limits at depth two.
`config_check_backend` checks that a backend value is a text.
`config_check_content` checks `paths`, `ignore`, a `load` flag, and an `arrive` word.
A world, budget or backend key in the local file draws the reason that such a key is fixed when the world is made.

## Functions

| Function | Result |
| --- | --- |
| `config_check_world` | Checks a world key, or gives the reason it refuses. |
| `config_check_budget` | Checks a budget key. |
| `config_check_backend` | Checks a backend key. |
| `config_check_content` | Checks a content key. |
