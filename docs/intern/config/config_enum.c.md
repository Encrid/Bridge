# src/config/config_enum.c

This file writes a setting into an enum field.

## An enum

An enum takes an integer that is the value of one of its constants, or a text that is the name of one of them.
The name of a constant is the one thing that a deployment can be expected to know.
An enum is the integer that its author stated: as many bytes, signed or not.
A signed enum of n bytes holds from -2^(8n-1) to 2^(8n-1) - 1, and an unsigned one from 0 to 2^(8n) - 1.

`config_seed_enum` does these steps, and a refusal ends them:

1. Refuse an integer that the enum cannot hold.
2. Refuse an integer that is not the value of a constant.
3. Refuse a text that is not the name of a constant.
4. Refuse a setting of any other kind as a misfit.
5. Write the value into the field: `docs/intern/config/config_scalar.c.md`, *An integer*.

`docs/extern/bridge.h.md`, *The settings*, gives the reason of each refusal.
The schema reader refuses a constant that the enum cannot hold: `docs/intern/schema/schema.h.md`, *The type set*.
Thus step 1 never refuses the value of a constant.

## Functions

| Function | Result |
| --- | --- |
| `find_constant` | Gives the value of a constant of an enum, by its name. False if no constant has that name. |
| `has_constant` | Gives whether a constant of an enum has the value `value`. |
| `fits_enum` | Gives whether an enum holds an integer. |
| `config_seed_enum` | Writes an integer that is the value of a constant, or the value of the constant that a text names, into an enum. Gives 1 if it refuses. |
