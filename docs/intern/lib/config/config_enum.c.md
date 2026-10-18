# src/lib/config/config_enum.c

This file writes a setting into an enum field or a bitmask field.

## An enum

An enum takes an integer that it can hold, or the text name of one of its constants.
The name of a constant is the one thing that a deployment can be expected to know.
An enum is the integer that its author stated: as many bytes, signed or not.
A signed enum of n bytes holds from -2^(8n-1) to 2^(8n-1) - 1, and an unsigned one from 0 to 2^(8n) - 1.
An integer that the enum cannot hold is refused, and so is a name that is not one of its constants.
A setting of any other kind is refused as a misfit.

## A bitmask

A bitmask takes an integer from 0 to the most that its bits hold.
An integer outside that range is refused, and a setting that is not an integer is refused as a misfit.

The field gets the low bytes of the value, as many as the type has.

## Functions

| Function | Result |
| --- | --- |
| `find_constant` | Gives the value of a constant of an enum, by its name. False if there is no name, no enum, or no constant of that name. |
| `fits_enum` | Gives whether an enum holds an integer. |
| `config_seed_enum` | Writes an integer, or the value of the constant that a text names, into an enum. Gives 1 if it refuses. |
| `config_seed_bitmask` | Writes an integer into a bitmask that has bits enough for it. Gives 1 if it refuses. |
