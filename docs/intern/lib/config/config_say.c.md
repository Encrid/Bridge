# src/lib/config/config_say.c

This file says why the settings of a module are refused.

## The reason

A refusal of the settings is written into the error of the load, as `printf` would write it.
For a setting whose kind the field does not take, the reason is in `docs/extern/bridge.h.md`, *The settings*.
The kind of a setting is one of these texts: a bool, an integer, a float, text.
A setting of no kind is refused before: `docs/intern/lib/config/config_field.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `config_refuse` | Writes why the settings are refused, as `printf` would. Gives 1. |
| `config_say_misfit` | Refuses a setting whose kind the field it names does not take: `field` tells what the field is. Gives 1. |
